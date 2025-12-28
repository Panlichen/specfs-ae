import json
import time
import os
from logging import Logger
from openai import Client
try:
    from google import genai
    from google.genai import types
except ImportError:
    pass
from utils import log_api_call, log_round_summary, log_workflow_start, log_workflow_end
from utils import extract_code_block

def call_llm_with_retry(client, model: str, messages: list, logger: Logger, 
                       stage_name: str, file_name: str, temperature: float = 0.0, 
                       response_format=None, initial_timeout: int = 480) -> object:
    retries = 0
    max_retries = 3
    timeout = initial_timeout
    
    client_type = os.getenv("CLIENT", "deepseek")

    while True:
        try:
            print(f"[{file_name}] ⏳ Calling LLM: {stage_name} (Timeout: {timeout}s)...")
            
            if client_type == "google":
                # Map model if needed, default to gemini-2.0-flash
                if model == "deepseek-reasoner":
                    model = "gemini-2.5-pro"
                elif model == "deepseek-chat":
                    model = "gemini-2.5-flash"

                # Convert messages to string prompt for Gemini
                prompt_content = ""
                for msg in messages:
                    prompt_content += f"[{msg['role'].upper()}]\n{msg['content']}\n\n"

                config = types.GenerateContentConfig(
                    temperature=temperature
                )
                if response_format and response_format.get('type') == 'json_object':
                     config.response_mime_type = "application/json"

                response = client.models.generate_content(
                    model=model,
                    contents=prompt_content,
                    config=config
                )
                
                # Mock OpenAI response structure
                class MockMessage:
                    def __init__(self, content):
                        self.content = content
                class MockChoice:
                    def __init__(self, message):
                        self.message = message
                class MockResponse:
                    def __init__(self, choices):
                        self.choices = choices
                
                response = MockResponse([MockChoice(MockMessage(response.text))])
            else:
                kwargs = {
                    "model": model,
                    "messages": messages,
                    "temperature": temperature,
                    "timeout": timeout
                }
                if response_format:
                    kwargs["response_format"] = response_format
                    
                response = client.chat.completions.create(**kwargs)

            print(f"[{file_name}] ✅ LLM Response received: {stage_name}")
            return response
        except Exception as e:
            retries += 1
            if retries > max_retries:
                logger.error(f"Max retries reached for {stage_name} in {file_name}: {e}")
                print(f"[{file_name}] ❌ Max retries reached for {stage_name}")
                raise e
            
            logger.warning(f"LLM call failed ({stage_name}) for {file_name}: {e}. Retrying...")
            print(f"[{file_name}] ⚠️ LLM call failed ({stage_name}): {e}. Retrying in 5s... (Attempt {retries}/{max_retries})")
            time.sleep(5)
            timeout += 60  # Dynamic adjustment: increase timeout by 60s

def evolve_code(client: Client, spec_content: str, orig_code: str, orig_spec: str, logger: Logger, file_name: str = "unknown") -> str:
    try:
        prompt = f"""I have generated a piece of code based on an initial specification. I will now provide you with the following: **[Original Specification]**, **[Generated Code]**, and **[Optimized Specification]**.
Your task is to generate new code according to the **Optimized Specification**, while also considering the structure and logic of the **Generated Code**.
**Important:**
* Your response must only contain a single C code block.
* Do not include any explanations, comments, or additional text outside of the code block.

[Original Specification]
{orig_spec}

[Generated Code]
{orig_code}

[Optimized Specification]
{spec_content}
"""
        messages = [{"role": "user", "content": prompt}]
        
        response = call_llm_with_retry(
            client, "deepseek-reasoner", messages, logger, "Evolve", file_name
        )

        api_output = response.choices[0].message.content
        log_api_call(logger, "deepseek-reasoner", messages, api_output, "Evolve")
        evolved_code = extract_code_block(api_output)
        return evolved_code

    except Exception as e:
        logger.error(f"Error in evolve_code: {e}")
        print(f"Error in evolve_code: {e}")
        return None

def generate_code(client: Client, spec_content: str, use_workflow: bool, logger: Logger, file_name: str = "unknown") -> str:
    log_workflow_start(logger, spec_content, use_workflow)
    
    if not use_workflow:
        try:
            logger.debug("=== Non-Workflow Mode ===")
            messages = [{"role": "user", "content": f"{spec_content}"}]
            
            response = call_llm_with_retry(
                client, "deepseek-reasoner", messages, logger, "Direct", file_name
            )
            
            result = response.choices[0].message.content
            log_api_call(logger, "deepseek-reasoner", messages, result, "Direct")
            log_workflow_end(logger, result, 0)

            return extract_code_block(result)
        except Exception as e:
            logger.error(f"Error calling DeepSeek API: {e}")
            print(f"Error calling DeepSeek API: {e}")
            return None

    # 工作流模式
    logger.debug("=== Workflow Mode Started ===")
    original_spec = spec_content
    refine_spec = ""
    generated_code = ""
    previous_code = ""
    max_rounds = 8

    for round_idx in range(max_rounds):
        current_round = round_idx + 1
        logger.debug(f"=== Starting Round {current_round} ===")
        print(f"[{file_name}] ▶️ Starting Round {current_round}")

        # Step 1: CodeGenerator
        codegen_prompt = (
            "You need to generate code according to provided specification and comments.\n\n"
            "The following will introduce the expected input (prompts) and the expected output.\n\n"
            "Your input (the prompt) should be composed of four parts (in most cases): [PROMPT], [RELY], [GUARANTEE], and [SPECIFICATION].\n"
            "And you may also see another two parts, [Previously generated code] and [Modification suggestions].\n"
            "In case you have these two parts and they are not empty, you should focus on modify the code according to these suggestions, instead of generating a new one.\n\n"
            "The descriptions different input parts are:\n"
            "* [PROMPT] represents the overall requirement for an LLM to generate the source code.\n"
            "* [RELY] clearly lists the predefined structures/functions from other modules that can be used for generating the source code. This is to avoid re - implementing functions, data structures, and variables that have already been implemented in other modules, ensuring correctness and modularity.\n"
            "* [GUARANTEE] provides the precise function signature that needs to be generated, along with specific requirements like the locking status, which the implemented source code (referred to as a single module) should meet. This is used to provide public functions, data structures, and variables for other modules to use, achieving correctness and modularity.\n"
            "* [SPECIFICATION] describes the functionality of the source code in this module (from the input). You should follow Hoare Logic and provide the pre - condition and post - condition for each function.\n\n"
            "For the output, only return a code block without any explanations or additional information.\n\n"
            "Notably:\n"
            "* you are generating a single module, instead of a whole project. So it is OK that you will directly use pre-defined functions and data structures defined in other modules (which are described in [Rely]), and do not generate thoese pre-defined modules and data structures already implemented in other modules, which will make the generation wrong!\n\n"
            f"{original_spec}\n"
        )
        if previous_code:
            codegen_prompt += f"\n[Previously generated code]\n{previous_code}\n"
        if refine_spec:
            codegen_prompt += f"\n[Modification suggestions]\n{refine_spec}\n"

        try:
            logger.debug(f"--- Round {current_round}: CodeGenerator ---")
            codegen_messages = [{"role": "system", "content": codegen_prompt}]
            
            codegen_resp = call_llm_with_retry(
                client, "deepseek-reasoner", codegen_messages, logger, 
                f"Round{current_round}_CodeGenerator", file_name
            )
            # 提取 code block
            generated_code = extract_code_block(codegen_resp.choices[0].message.content)
            
            log_api_call(logger, "deepseek-reasoner", codegen_messages,
                        codegen_resp.choices[0].message.content, f"Round{current_round}_CodeGenerator")
            logger.debug(f"Extracted Code Block:\n{generated_code}")

        except Exception as e:
            logger.error(f"Error in CodeGenerator Round {current_round}: {e}")
            print(f"Error in CodeGenerator: {e}")
            return None

        # Step 2: SpecEvaluator
        speceval_prompt = (
            "This is a code generation validation task.\n"
            "Please check if the generated code (in [Generated code]) meets the specification ([Spec]). If it does not, provide suggestions for modifications.\n\n"
            "If the [Generated code] can attain essentially the same outcomes as [Spec], you should put a boolean: true, in the `is_good` variable in output (in JSON); otherwise, put a boolean false.\n"
            "In case the [Generated code] does not fully meet the specification, you should put detailed instructions in variable `comments` to modify the code to fit the spec. Only check the code, do not check the comments.\n"
            "Your output should be in JSON format.\n"
            f"[Generated code]\n{generated_code}\n\n"
            f"[Spec]\n{original_spec}\n"
        )
        try:
            logger.debug(f"--- Round {current_round}: SpecEvaluator ---")
            speceval_messages = [{"role": "system", "content": speceval_prompt}]
            
            speceval_resp = call_llm_with_retry(
                client, "deepseek-reasoner", speceval_messages, logger, 
                f"Round{current_round}_SpecEvaluator", file_name, temperature=1.0, 
                response_format={'type': 'json_object'}
            )
            
            speceval_content = speceval_resp.choices[0].message.content
            log_api_call(logger, "deepseek-reasoner", speceval_messages,
                        speceval_content, f"Round{current_round}_SpecEvaluator")
            
            speceval_json = json.loads(speceval_content)
            is_good = speceval_json.get("is_good", False)
            comments = speceval_json.get("comments", "")
            
            log_round_summary(logger, current_round, generated_code, is_good, comments)
            
            if is_good:
                logger.debug(f"Code approved in Round {current_round}. Workflow completed.")
                print(f"[{file_name}] 🎉 Code approved in Round {current_round}")
                log_workflow_end(logger, generated_code, current_round)
                return generated_code
            else:
                refine_spec = comments
                previous_code = generated_code
                logger.debug(f"Round {current_round} completed. Moving to next round with refinements.")
                print(f"[{file_name}] 🔄 Round {current_round} completed. Refining...")
                
        except Exception as e:
            logger.error(f"Error in SpecEvaluator Round {current_round}: {e}")
            print(f"Error in SpecEvaluator: {e}")
            log_workflow_end(logger, generated_code, current_round)
            return generated_code

    logger.debug("Maximum rounds reached. Returning final generated code.")
    print(f"[{file_name}] 🏁 Maximum rounds reached.")
    log_workflow_end(logger, generated_code, max_rounds)
    return generated_code

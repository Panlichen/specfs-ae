import sys
import os
import argparse
from pathlib import Path

from utils import setup_logger, console
from tools.spec2code import spec2code
from tests.test_specfs import run_compile_and_test

PROJECT_ROOT = Path(__file__).parent.resolve()

def main():
    parser = argparse.ArgumentParser(description="SysSpec Code Generator")
    parser.add_argument('mode', choices=['gen', 'evolve'],
                       help='Generation mode: gen or evolve')

    args = parser.parse_args()
    mode = args.mode

    log_dir = PROJECT_ROOT / "log"
    # Enable console output for main logger
    logger = setup_logger(log_dir, "main", to_console=True)

    logger.info("== SysSpec Code Generation ==")

    client_type = os.getenv("CLIENT", "deepseek")
    if client_type == "google":
        if os.getenv("GOOGLE_API_KEY") is None:
            console.print("[bold red]GOOGLE_API_KEY environment variable is not set.[/]")
            sys.exit(1)
        console.print("[bold green]Using Google Gemini Client[/]")
    elif os.getenv("DEEPSEEK_API_KEY") is None:
        console.print("[bold red]DEEPSEEK_API_KEY environment variable is not set.[/]")
        sys.exit(1)
    else:
        console.print("[bold green]Using DeepSeek Client[/]")

    max_workers = 10
    if os.getenv("MAX_WORKERS") is not None:
        try:
            max_workers = int(os.getenv("MAX_WORKERS"))
            console.print(f"[bold green]Using MAX_WORKERS={max_workers}[/]")
        except ValueError:
            console.print("[bold red]Invalid MAX_WORKERS value. Using default of 10.[/]")

    try:
        logger.debug(f"Starting code generation in {mode} mode...")

        while True:
            spec2code(PROJECT_ROOT / "sysspec", logger, mode, max_workers=max_workers)

            # Phase 1: Generation Loop
            if run_compile_and_test(silent=True):
                break
            
            logger.info("Detected failure, retrying generation...")

        logger.debug("Finish SysSpec Code Generation Process!")

        # Phase 2: Verification
        console.print("\n[bold blue]Running final verification...[/]")
        run_compile_and_test(silent=False)

        console.print("\n[bold green]✅ All tests completed successfully![/]")

    except Exception as e:
        logger.error(f"Process failed: {e}", exc_info=True)
        sys.exit(1)


if __name__ == "__main__":
    main()

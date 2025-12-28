import sys
import subprocess
import time
import logging
import json
import re
import threading
import os
import shutil
from pathlib import Path
from typing import Optional

try:
    from rich.console import Console
    from rich.traceback import install
except ImportError:
    print("Please install dependency: pip install rich")
    sys.exit(1)

try:
    from dotenv import load_dotenv
except ImportError:
    print("Please install dependency: pip install python-dotenv")
    sys.exit(1)

install()
console = Console()

# Define paths relative to this utils file
MODULE_PATH = Path(__file__).parent.resolve()
DATA_DIR = MODULE_PATH / "data"
TOOL_DIR = MODULE_PATH / "tools"

# Load environment variables from .env file
load_dotenv(MODULE_PATH / ".env")

# Configuration
MOUNT_POINT = Path(os.getenv("MOUNT_POINT", Path.home() / "atomfs"))
MOUNT_DEV = Path(os.getenv("MOUNT_DEV", "/dev/pmem0"))
FS_TARGET = "atomfs-fuse"

# --- Dataset Information ---
DATASETS = {
    "qemu": {
        "url": "https://download.qemu.org/qemu-10.0.0.tar.xz",
        "tar": "qemu-10.0.0.tar.xz",
        "dir": "qemu-10.0.0"
    },
    "linux": {
        "url": "https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.14.4.tar.xz",
        "tar": "linux-6.14.4.tar.xz",
        "dir": "linux-6.14.4"
    }
}

# --- Logger Utilities (Migrated & Unified) ---

def setup_logger(log_dir: Path, file_name: str, to_console: bool = False) -> logging.Logger:
    log_dir.mkdir(parents=True, exist_ok=True)

    logger = logging.getLogger(file_name)
    logger.setLevel(logging.DEBUG)
    logger.handlers.clear()

    log_file = log_dir / f"{file_name}.log"
    file_handler = logging.FileHandler(log_file, mode='w', encoding='utf-8')
    file_handler.setLevel(logging.DEBUG)
    
    formatter = logging.Formatter(
        '%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    if to_console:
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.INFO)
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)

    return logger

def log_api_call(logger: logging.Logger, model: str, messages: list, response_content: str, call_type: str):
    logger.debug(f"=== {call_type} API Call ===")
    logger.debug(f"Model: {model}")
    logger.debug(f"Messages: {json.dumps(messages, ensure_ascii=False, indent=2)}")
    logger.debug(f"Response Content: {response_content}")
    logger.debug(f"=== End {call_type} API Call ===\n")

def log_round_summary(logger: logging.Logger, round_num: int, generated_code: str, is_good: bool, comments: str = ""):
    logger.debug(f"=== Round {round_num} Summary ===")
    logger.debug(f"Generated Code:\n{generated_code}")
    logger.debug(f"Is Good: {is_good}")
    if comments:
        logger.debug(f"Comments: {comments}")
    logger.debug(f"=== End Round {round_num} Summary ===\n")

def log_workflow_start(logger: logging.Logger, spec_content: str, use_workflow: bool):
    logger.debug("=== Workflow Started ===")
    logger.debug(f"Use Workflow: {use_workflow}")
    logger.debug(f"Original Spec:\n{spec_content}")
    logger.debug("=== End Workflow Start ===\n")

def log_workflow_end(logger: logging.Logger, final_code: str, total_rounds: int):
    logger.debug("=== Workflow Completed ===")
    logger.debug(f"Total Rounds: {total_rounds}")
    logger.debug(f"Final Generated Code:\n{final_code}")
    logger.debug("=== End Workflow ===")

# --- String/Parsing Utilities ---

def extract_code_block(text: str) -> str:
    match = re.search(r"```\w*\n([\s\S]*?)```", text)
    if match:
        return match.group(1).strip()
    return text.strip()

def extract_json(text: str) -> dict:
    match = re.search(r"\{[\s\S]*?\}", text)
    if match:
        try:
            return json.loads(match.group(0))
        except Exception:
            pass
    return {}

# --- UI/Progress Utilities ---

class ProgressTracker:
    def __init__(self, total_files: int):
        self.total_files = total_files
        self.current_count = 0
        self.lock = threading.Lock()

    def update(self, file_name: str):
        with self.lock:
            self.current_count += 1
            console.print(f"[bold blue][{self.current_count}/{self.total_files}][/] Generated {file_name}")

# --- Existing Shell Utilities ---

def run_cmd(cmd: list, cwd=None, show_output=True) -> bool:
    """Wrapper for executing Shell commands"""
    cmd_str = " ".join([str(x) for x in cmd])
    console.log(f"[dim]Executing: {cmd_str}[/]")

    try:
        process = subprocess.Popen(
            cmd,
            stdout=sys.stdout if show_output else subprocess.DEVNULL,
            stderr=sys.stderr if show_output else subprocess.DEVNULL,
            cwd=cwd,
            shell=False,
        )
        process.wait()
        return process.returncode == 0
    except Exception as e:
        console.print(f"[bold red]Execution Exception: {e}[/]")
        return False

def compile_filesystem(project_path: Path, target_name: str) -> bool:
    target_executable = project_path / target_name

    run_cmd(["make", "clean"], cwd=project_path, show_output=False)
    run_cmd(["make"], cwd=project_path, show_output=False)

    if not target_executable.exists():
        console.print(f"[bold red]Compilation failed. Target {target_name} not found.[/]")
        return False

    # console.print(f"[bold green]Compilation success! Generated: {target_executable}[/]")
    return True

def setup_filesystem(mount_point: Path, mount_dev: Path, project_path: Path, target_name: str) -> Optional[subprocess.Popen]:
    # console.print("[dim]Setting Up Filesystem[/]")
    mount_point.mkdir(parents=True, exist_ok=True)
    run_cmd(["umount", str(mount_point)], show_output=False)

    if not compile_filesystem(project_path, target_name):
        return None

    # console.print("[dim]Starting filesystem process...[/]")
    try:
        fs_process = subprocess.Popen(
            [str(project_path / target_name), "-n", str(mount_dev), str(mount_point)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            cwd=project_path
        )
        time.sleep(1)
        console.print("[dim]Filesystem started successfully![/]")
        return fs_process
    except Exception as e:
        console.print("[bold red]Filesystem process failed to start![/]")
        console.print(f"      └─> Error: {e}")
        return None

def prepare_dataset(name: str, url: str, tar_name: str) -> Optional[Path]:
    """Downloads and extracts a dataset if not present."""

    DATA_DIR.mkdir(parents=True, exist_ok=True)
    tar_path = DATA_DIR / tar_name
    extract_path = DATA_DIR / name
    max_retries = 3

    if not tar_path.exists():
        success = False
        for attempt in range(1, max_retries + 1):
            console.print(f"[dim]Downloading {name} source (Attempt {attempt}/{max_retries})...[/]")
            if run_cmd(["wget", "-O", str(tar_path), url], show_output=True):
                success = True
                break
            
            console.print("[bold yellow]Download failed. Cleaning up and retrying...[/]")
            if tar_path.exists():
                tar_path.unlink()
            time.sleep(2)

        if not success:
            console.print(f"[bold red]Failed to download {name} after {max_retries} attempts![/]")
            return None

    if not extract_path.exists():
        console.print(f"[dim]Extracting {name} source...[/]")
        if not run_cmd(["tar", "-xf", str(tar_path)], cwd=DATA_DIR):
            console.print(f"[bold red]Failed to extract {name}! Cleaning up files...[/]")
            # Cleanup tarball (likely corrupt)
            if tar_path.exists():
                tar_path.unlink()
            # Cleanup partial extraction if it exists
            if extract_path.exists():
                if extract_path.is_dir():
                    shutil.rmtree(extract_path)
                else:
                    extract_path.unlink()
            return None

    return extract_path

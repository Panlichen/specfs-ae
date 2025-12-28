#!/usr/bin/env python3
import subprocess
import sys
import time
import shutil
from pathlib import Path
from utils import console
from utils import MOUNT_DEV, FS_TARGET, MOUNT_POINT, TOOL_DIR, DATA_DIR, DATASETS

# --- Configuration ---
# PROJECT_ROOT is calculated based on this file's location.
PROJECT_ROOT = Path(__file__).parent.parent.resolve() / "sysspec" / "genfs"


# --- UI and Logging Helpers ---
def print_header(message, silent=False):
    """Prints a bold, blue header."""
    if not silent:
        console.print(f"\n[bold blue]===== {message} =====[/]")

def run_command(description, command, cwd=PROJECT_ROOT, check=True, silent=False):
    """
    Runs a command, displaying a status line that updates from "RUNNING" to "OK" or "FAILED".
    All command output is discarded.
    The `cwd` parameter is crucial for running commands in the correct directory without using os.chdir.
    """
    if not silent:
        padded_description = f"{description:<45}"
        console.print(f"  {padded_description} ... [[yellow]RUNNING[/]]", end='\r')

    try:
        subprocess.run(
            command,
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            cwd=str(cwd) # Explicitly set the working directory for the subprocess
        )
        if not silent:
            console.print(f"  {padded_description} ... [[green]   OK   [/]]")
        return True
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        if not silent:
            console.print(f"  {padded_description} ... [[red] FAILED [/]]")
        return False

# --- Main Logic Functions ---

def compile_project(silent=False):
    """Cleans and compiles the project using Makefile in PROJECT_ROOT."""
    print_header("Compiling Project", silent=silent)
    # Always run make commands from the project's root directory.
    if not run_command("Cleaning previous builds (make clean)", ["make", "clean"], cwd=PROJECT_ROOT, silent=silent):
        return False
    if not run_command("Compiling project (make)", ["make", FS_TARGET], cwd=PROJECT_ROOT, silent=silent):
        if not silent:
            console.print("[red]Compilation failed. Aborting tests.[/]")
        return False
    return True

def setup_filesystem(silent=False):
    """Prepares mount point and starts the filesystem process."""
    print_header("Setting Up Filesystem", silent=silent)
    MOUNT_POINT.mkdir(parents=True, exist_ok=True)
    run_command("Unmounting old filesystem (if any)", ["umount", str(MOUNT_POINT)], check=False, cwd=PROJECT_ROOT, silent=silent)

    target_executable = PROJECT_ROOT / FS_TARGET
    if not target_executable.exists():
        if not silent:
            console.print(f"[red]Target executable '{FS_TARGET}' not found. Please compile first.[/]")
        return None
        
    if not silent:
        console.print(f"  { 'Starting filesystem process':<45} ... [[yellow]RUNNING[/]]", end='\r')
    try:
        # Popen also needs the correct cwd to find the executable
        fs_process = subprocess.Popen(
            [str(target_executable), "-n", str(MOUNT_DEV), str(MOUNT_POINT)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            cwd=PROJECT_ROOT
        )
        time.sleep(1)
        if not silent:
            console.print(f"  {'Starting filesystem process':<45} ... [[green]   OK   [/]]")
        return fs_process
    except Exception as e:
        if not silent:
            console.print(f"  {'Starting filesystem process':<45} ... [[red] FAILED [/]]")
            console.print(f"    └─> Error: {e}")
        return None

def prepare_dependencies(silent=False):
    """Handles downloading and preparing test dependencies."""
    print_header("Preparing Test Dependencies", silent=silent)
    tar_path = DATA_DIR / DATASETS["qemu"]["tar"]
    extract_path = DATA_DIR / DATASETS["qemu"]["dir"]
        
    if not tar_path.is_file():
        run_command("Downloading QEMU source", ["wget", "-O", str(tar_path), DATASETS["qemu"]["url"]], cwd=DATA_DIR, silent=silent)
    else:
        if not silent:
            console.print(f"  {'Downloading QEMU source':<45} ... [[blue] SKIPPED (cached) [/]]")

    if not extract_path.is_dir():
        run_command("Extracting QEMU source", ["tar", "-xf", str(tar_path)], cwd=DATA_DIR, silent=silent)
    else:
        if not silent:
            console.print(f"  {'Extracting QEMU source':<45} ... [[blue] SKIPPED (cached) [/]]")
        
    run_command("Compiling LFStest tools", ["make"], cwd=TOOL_DIR, silent=silent)

def run_tests(silent=False):
    """Executes the main test suite."""
    print_header("Test Case 1: Applications/Workloads", silent=silent)
    if MOUNT_POINT.is_dir():
        for item in MOUNT_POINT.iterdir():
            if item.is_dir():
                shutil.rmtree(item)
            else:
                item.unlink()
    
    xv6_dir = MOUNT_POINT / "xv6"
    if not run_command("Test: git clone xv6", ["cp", "-r", DATA_DIR / "xv6", "xv6"], cwd=MOUNT_POINT, silent=silent):
        return False
    if not run_command("Test: compile xv6", ["make"], cwd=xv6_dir, silent=silent):
        return False
    if not run_command("Test: cp QEMU source", ["cp", "-r", str(DATA_DIR / DATASETS["qemu"]["dir"]), str(MOUNT_POINT)], silent=silent):
        return False
    if not run_command("Test: large-file", [str(TOOL_DIR / "largefile"), "-f", "10", "-i", "1", str(MOUNT_POINT)], silent=silent):
        return False
    if not run_command("Test: small-file", [str(TOOL_DIR / "smallfile"), "10000", "1024", str(MOUNT_POINT)], silent=silent):
        return False
    
    print_header("Test Case 2: Concurrency Test", silent=silent)
    base_path = MOUNT_POINT / "dd1/d2/d3/d4/d5/d6/d7/d8/d9"
    paths_to_create = [base_path / f"d{i}" for i in range(10, 14)]
    for p in paths_to_create:
        p.mkdir(parents=True, exist_ok=True)
    
    description = "Running 4 parallel largefile tests"
    if not silent:
        console.print(f"  {description:<45} ... [[yellow]RUNNING[/]]", end='\r')
    commands = [
        [str(TOOL_DIR / "largefile"), "-f", "10", "-i", "1", str(paths_to_create[0])],
        [str(TOOL_DIR / "largefile"), "-f", "10", "-i", "1", str(paths_to_create[1])],
        [str(TOOL_DIR / "largefile"), "-f", "10", "-i", "1", str(paths_to_create[2])],
        [str(TOOL_DIR / "largefile"), "-f", "10", "-i", "1", str(paths_to_create[3])],
    ]
    
    bg_processes = []
    for cmd in commands:
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            bg_processes.append(p)
        except FileNotFoundError:
            p = subprocess.Popen(cmd[2:], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            bg_processes.append(p)

    all_success = True
    try:
        for p in bg_processes:
            return_code = p.wait(timeout=300)
            if return_code != 0:
                all_success = False
    except subprocess.TimeoutExpired:
        if not silent:
            console.print(f"  {description:<45} ... [[red] FAILED (Timeout) [/]]")
        for p_kill in bg_processes:
            if p_kill.poll() is None:
                p_kill.kill()
        return False
    
    if all_success:
        if not silent:
            console.print(f"  {description:<45} ... [[green]   OK   [/]]")
    else:
        if not silent:
            console.print(f"  {description:<45} ... [[red] FAILED (Exit Code != 0) [/]]")
        return False
    return True

def cleanup(fs_process, silent=False):
    """Unmounts the filesystem and terminates the process."""
    print_header("Cleaning Up", silent=silent)
    run_command("Unmounting filesystem", ["umount", str(MOUNT_POINT)], check=False, silent=silent)
    if fs_process and fs_process.poll() is None:
        try:
            fs_process.terminate()
            fs_process.wait(timeout=5)
            if not silent:
                console.print(f"  {'Terminating filesystem process':<45} ... [[green]   OK   [/]]")
        except subprocess.TimeoutExpired:
            fs_process.kill()
            if not silent:
                console.print(f"  {'Terminating filesystem process':<45} ... [[yellow] KILLED [/]]")
    else:
        run_command(f"Force-stopping '{FS_TARGET}'", ["killall", FS_TARGET], check=False, silent=silent)

def run_compile_and_test(silent=False):
    """
    The main test function, designed to be called from other scripts.
    It orchestrates compilation, setup, testing, and cleanup.
    Returns:
        bool: True if all tests passed, False otherwise.
    """
    if not compile_project(silent=silent):
        return False
    
    prepare_dependencies(silent=silent)

    fs_process = setup_filesystem(silent=silent)
    if not fs_process:
        if not silent:
            console.print("\n[red]Failed to start the filesystem. Aborting.[/]")
        return False

    all_tests_ok = False
    try:
        all_tests_ok = run_tests(silent=silent)
    except Exception as e:
        if not silent:
            console.print(f"\n[red]An unexpected error occurred during tests: {e}[/]")
    finally:
        cleanup(fs_process, silent=silent)

    return all_tests_ok


if __name__ == "__main__":
    # This block executes only when the script is run directly
    # from the command line (e.g., `python3 run_tests.py`).
    if run_compile_and_test():
        console.print("\n[bold green]✅ All tests completed successfully![/]")
        sys.exit(0)
    else:
        console.print("\n[bold red]❌ Some tests failed. Please review the output above.[/]")
        sys.exit(1)
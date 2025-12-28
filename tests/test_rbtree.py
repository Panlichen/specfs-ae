import time
from pathlib import Path
from typing import Optional
from utils import console, run_cmd, setup_filesystem
from utils import MOUNT_POINT, MOUNT_DEV, FS_TARGET, TOOL_DIR

def run(file_size: int, write_times: int, project_path: Path) -> Optional[int]:
    """
    Test rbtree efficiency in the pre-allocation pool.
    """
    console.print(f"[dim]Testing rbtree with write size {file_size}, write times {write_times}...[/]")

    try:
        if not Path(TOOL_DIR / "rbtree_test.cpp").exists():
            console.print("[bold red]Benchmark source not found![/]")
            return
        run_cmd(["make", "rbtree_test"], show_output=False, cwd=TOOL_DIR)
    except Exception as e:
        console.print(f"[bold red]Benchmark compilation exception: {e}[/]")
        return

    try:
        fs_process = setup_filesystem(MOUNT_POINT, MOUNT_DEV, project_path, FS_TARGET)
        if fs_process is None:
            console.print("[bold red]Filesystem failed to start, aborting test![/]")
            return
    except Exception as e:
        console.print(f"[bold red]Filesystem exception: {e}[/]")
        return

    try:
        console.print(f"[dim]{file_size} KB random write, {write_times} contiguous r/w to test file...[/]")
        try:
            run_cmd([
                str(TOOL_DIR / "rbtree_test"),
                str(MOUNT_POINT / "testfile"),
                str(file_size * 4),
                str(write_times),
            ], show_output=False)
        except Exception as e:
            console.print(f"[bold red]Benchmark execution exception: {e}[/]")
            return

        time.sleep(10)
    finally:
        run_cmd(["umount", str(MOUNT_POINT)], show_output=False)

    count_file = project_path / "pool_access_count"
    if not count_file.exists():
        console.print("[bold red]Pool access count file not found![/]")
        return
    with open(count_file, "r") as f:
        count = int(f.read().strip())
    return count
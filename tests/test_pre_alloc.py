import time
from pathlib import Path
from typing import Optional
from utils import console, run_cmd, setup_filesystem
from utils import MOUNT_POINT, MOUNT_DEV, FS_TARGET, TOOL_DIR


def count_uncontig(filepath: str, page_size: int=0x1000) -> float:
    with open(filepath, 'r') as f:
        lines = f.readlines()

    entries = lines[1:]  # 跳过第一行

    total = 0
    continuous = 0
    non_continuous = 0

    current_syscall_accesses = []

    def process_syscall(accesses):
        nonlocal total, continuous, non_continuous
        if len(accesses) == 1:
            total += 1
            non_continuous += 1
            return

        if not accesses:
            return

        total += len(accesses)  # N 次访问有 N-1 个跳转
        non_continuous += 1

        last_extent, last_page = accesses[0]
        for extent, page in accesses[1:]:
            if extent == last_extent and page == last_page + page_size:
                continuous += 1
            else:
                non_continuous += 1
            last_extent = extent
            last_page = page

    for line in entries:
        line = line.strip()
        if line == "(nil) (nil)":
            # syscall 结束，处理当前 syscall 的访问
            process_syscall(current_syscall_accesses)
            current_syscall_accesses = []
        else:
            try:
                extent_str, page_str = line.split()
                extent = int(extent_str, 16)
                page = int(page_str, 16)
                current_syscall_accesses.append((extent, page))
            except ValueError:
                # 忽略非法行
                continue

    # 文件末尾也可能有访问未处理
    process_syscall(current_syscall_accesses)

    if total > 0:
        return (non_continuous / total) * 100
    else:
        return -1


def run(write_size: int, project_path: Path) -> Optional[float]:
    """
    Test pre-allocation efficiency by writing data and checking access patterns.
    """
    console.print(f"[dim]Testing Pre-allocation with write size {write_size}...[/]")

    try:
        if not Path(TOOL_DIR / "prealloc_test.cpp").exists():
            console.print("[bold red]Benchmark source not found![/]")
            return
        run_cmd(["make", "prealloc_test"], show_output=False, cwd=TOOL_DIR)
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
        console.print(f"[dim]{write_size} KB random write, 500 contiguous r/w to test file...[/]")
        try:
            run_cmd([
                str(TOOL_DIR / "prealloc_test"),
                str(MOUNT_POINT / "testfile"),
                str(write_size),
            ], show_output=False)
        except Exception as e:
            console.print(f"[bold red]Benchmark execution exception: {e}[/]")
            return

        time.sleep(10)
    finally:
        run_cmd(["umount", str(MOUNT_POINT)], show_output=False)

    # Analyze access patterns
    access_log_file = project_path / "access.log"
    if access_log_file.exists():
        console.print("[dim]Analyzing access patterns from log...[/]")
        return count_uncontig(str(access_log_file))
    else:
        console.print("[bold red]Access log file not found![/]")

    return 0

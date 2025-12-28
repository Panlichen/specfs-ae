import time
from pathlib import Path
from utils import console, run_cmd, setup_filesystem
from utils import MOUNT_POINT, MOUNT_DEV, FS_TARGET, DATA_DIR, TOOL_DIR


def get_block_count(project_path: Path) -> list[int]:
    block_count_file = project_path / "data_block_count"
    counts = []
    if block_count_file.exists():
        with open(block_count_file, "r") as f:
            for line in f:
                counts.append(int(line.strip()))
    else:
        console.print("[bold red]Block count file not found![/]")

    # counts: [metadata_read_count, metadata_write_count, data_read_count, data_write_count]
    return counts


def compile_xv6(project_path: Path) -> list[int]:
    xv6_scr = DATA_DIR / "xv6"
    run_cmd(["cp", "-r", str(xv6_scr) + "/.", str(MOUNT_POINT / "xv6")], show_output=False)
    time.sleep(5)
    before_compile = get_block_count(project_path)
    run_cmd(["make", "-C", str(MOUNT_POINT / "xv6")], show_output=False)
    time.sleep(5)
    after_compile = get_block_count(project_path)

    block_count = [after_compile[i] - before_compile[i] for i in range(len(before_compile))]
    return block_count

def copy_qemu(project_path: Path) -> list[int]:
    qemu_src = DATA_DIR / "qemu-10.0.0"
    run_cmd(["cp", "-r", str(qemu_src) + "/.", str(MOUNT_POINT / "qemu-10.0.0")], show_output=False)
    time.sleep(5)
    return get_block_count(project_path)

def run_large_file(project_path: Path) -> list[int]:
    # compile large file
    run_cmd(["make", "largefile"], show_output=False, cwd=TOOL_DIR)
    run_cmd([str(TOOL_DIR / "largefile"),
             "-f", "10",
             "-i", "1",
            str(MOUNT_POINT)],
            show_output=False)
    time.sleep(5)
    return get_block_count(project_path)

def run_small_file(project_path: Path) -> list[int]:
    # compile small file
    run_cmd(["make", "smallfile"], show_output=False, cwd=TOOL_DIR)
    run_cmd([str(TOOL_DIR / "smallfile"),
             "10000", "1024",
            str(MOUNT_POINT)],
            show_output=False)
    time.sleep(5)
    return get_block_count(project_path)


def run(project_path: Path) -> list[list[int]]:
    tests = [
        ("Compile XV6", compile_xv6),
        ("Copy QEMU", copy_qemu),
        ("Large File Operations", run_large_file),
        ("Small File Operations", run_small_file),
    ]

    results = []

    for test_name, test_func in tests:
        console.print(f"[dim]Running Test: {test_name}[/]")
        block_counts = []
        try:
            fs_process = setup_filesystem(MOUNT_POINT, MOUNT_DEV, project_path, FS_TARGET)
            if fs_process is None:
                console.print("[bold red]Filesystem failed to start, aborting test![/]")
                results.append([])
                continue
            block_counts = test_func(project_path)
        except Exception as e:
            console.print(f"[bold red]Filesystem exception: {e}[/]")
            results.append([])
            continue
        finally:
            run_cmd(["umount", str(MOUNT_POINT)], show_output=False)

        results.append(block_counts)

    return results

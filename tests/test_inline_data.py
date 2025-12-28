import time
from pathlib import Path
from typing import Optional
from utils import console, run_cmd, setup_filesystem, prepare_dataset
from utils import MOUNT_POINT, MOUNT_DEV, FS_TARGET, DATASETS

def run(dataset_key: str, project_path: Path) -> Optional[int]:
    """
    Test inline data efficiency by copying source code and checking block usage.
    """
    if dataset_key not in DATASETS:
        console.print(f"[bold red]Unknown dataset: {dataset_key}[/]")
        return

    info = DATASETS[dataset_key]
    src_path = prepare_dataset(info["dir"], info["url"], info["tar"])
    if not src_path:
        return

    console.print(f"[dim]Testing Inline Data with {dataset_key}...[/]")

    try:
        fs_process = setup_filesystem(MOUNT_POINT, MOUNT_DEV, project_path, FS_TARGET)
        if fs_process is None:
            console.print("[bold red]Filesystem failed to start, aborting test![/]")
            return
    except Exception as e:
        console.print(f"[bold red]Filesystem exception: {e}[/]")
        return

    try:
        console.print(f"[dim]Copying {dataset_key} source to filesystem...[/]")
        run_cmd(["cp", "-r", str(src_path) + "/.", str(MOUNT_POINT)], show_output=False)
        
        console.print("[dim]Triggering a Read syscall...[/]")
        # Assuming Makefile exists in both QEMU and Linux root
        run_cmd(["head", "-c", "4096", str(MOUNT_POINT / "Makefile")], show_output=False)
    finally:
        run_cmd(["umount", str(MOUNT_POINT)], show_output=False)

    # Check results
    block_count_file = project_path / "data_block_count"
    if block_count_file.exists():
        with open(block_count_file, "r") as f:
            count = f.read().strip()
        console.print(f"[dim]{dataset_key.upper()} Source Total Blocks Used: {count}[/]")
        return int(count)
    else:
        console.print("[bold red]Block count file 'data_block_count' not found[/]")
    return None

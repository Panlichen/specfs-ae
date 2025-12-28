#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from pathlib import Path

# Ensure we can import from local modules
sys.path.append(str(Path(__file__).parent))

from plot import performance, loc
from utils import console, MODULE_PATH
from tests import test_inline_data, test_pre_alloc, test_rbtree, test_block, stats_code

def main():
    console.print("[bold blue]Starting SpecFS Evaluation...[/]")

    inline_baseline = MODULE_PATH / "eval/inline_data/baseline"
    inline_optimization = MODULE_PATH / "eval/inline_data/optimization"
    console.print("[bold blue]\nInline Data Test: Baseline[/]")
    qemu_count = test_inline_data.run("qemu", inline_baseline)
    linux_count = test_inline_data.run("linux", inline_baseline)
    console.print("[bold blue]\nInline Data Test: Optimization[/]")
    qemu_inline_count = test_inline_data.run("qemu", inline_optimization)
    linux_inline_count = test_inline_data.run("linux", inline_optimization)

    inline_res = {
        'baseline': [qemu_count or 0, linux_count or 0],
        'optimization': [qemu_inline_count or 0, linux_inline_count or 0]
    }

    prealloc_baseline = MODULE_PATH / "eval/pre_alloc/baseline"
    prealloc_optimization = MODULE_PATH / "eval/pre_alloc/optimization"
    console.print("[bold blue]\nPre-allocation Test: Baseline[/]")
    base_16k = test_pre_alloc.run(16, prealloc_baseline)
    base_8k = test_pre_alloc.run(8, prealloc_baseline)
    console.print("[bold blue]\nPre-allocation Test: Optimization[/]")
    prealloc_8k = test_pre_alloc.run(8, prealloc_optimization)
    prealloc_16k = test_pre_alloc.run(16, prealloc_optimization)

    prealloc_res = {
        'baseline': [base_8k or 0, base_16k or 0],
        'optimization': [prealloc_8k or 0, prealloc_16k or 0]
    }

    rbtree_baseline = MODULE_PATH / "eval/rbtree/baseline"
    rbtree_optimization = MODULE_PATH / "eval/rbtree/optimization"
    console.print("[bold blue]\nRBTree Test: Baseline[/]")
    base_5m_500w = test_rbtree.run(5, 500, rbtree_baseline)
    base_20m_1000w = test_rbtree.run(20, 1000, rbtree_baseline)
    console.print("[bold blue]\nRBTree Test: Optimization[/]")
    rbtree_5m_500w = test_rbtree.run(5, 500, rbtree_optimization)
    rbtree_20m_1000w = test_rbtree.run(20, 1000, rbtree_optimization)

    rbtree_res = {
        'baseline': [base_5m_500w or 0, base_20m_1000w or 0],
        'optimization': [rbtree_5m_500w or 0, rbtree_20m_1000w or 0]
    }

    block_baseline = MODULE_PATH / "eval/extent/baseline"
    extent_optimization = MODULE_PATH / "eval/extent/optimization"
    delay_alloc_optimization = MODULE_PATH / "eval/delay_alloc/optimization"

    console.print("[bold blue]\nExtent & Delayed Allocation Test: Baseline[/]")
    res_block = test_block.run(block_baseline)
    console.print("[bold blue]\nExtent Test: Optimization[/]")
    res_extent = test_block.run(extent_optimization)
    console.print("[bold blue]\nDelayed Allocation Test: Optimization[/]")
    res_delay = test_block.run(delay_alloc_optimization)

    console.print("[bold blue]\nGenerating performance plots...[/]")
    performance.draw_performance_plot_1(inline_res, prealloc_res, rbtree_res, MODULE_PATH)
    performance.draw_performance_plot_2(res_block, res_extent, res_delay, MODULE_PATH)

    console.print("[bold blue]\nGenerating LOC plots...[/]")
    stats = stats_code.collect_all_stats()
    loc.plot_atomfs(stats)
    loc.plot_features(stats)


if __name__ == "__main__":
    main()

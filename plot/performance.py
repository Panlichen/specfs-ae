import matplotlib.pyplot as plt
import numpy as np
from plot.utils import setup_font, save_figure, safe_div


def draw_performance_plot_1(inline_res, prealloc_res, rbtree_res, output_dir):
    # Set font to Times New Roman
    setup_font()

    # Create figure with 1 row and 3 columns
    fig, axes = plt.subplots(1, 3, figsize=(9, 4), gridspec_kw={'width_ratios': [1, 1, 1]})

    # Define unified color scheme
    colors = {
        'common': '#ade8f4',
        'metadata_reads': '#48cae4',
        'metadata_writes': '#0096c7',
        'data_reads': '#0077b6',
        'data_writes': '#03045e',
    }

    # ========== Subplot 1: Original File Size Data ==========
    files = ['qemu', 'linux']
    # inline_res: {'baseline': [qemu, linux], 'optimization': [qemu, linux]}
    with_inline_data = inline_res['optimization']
    without_inline_data = inline_res['baseline']

    ratios = [safe_div(with_inline_data[i], without_inline_data[i]) for i in range(len(files))]
    percentages = [ratio * 100 for ratio in ratios]

    bars1 = axes[0].bar(files, percentages, color=[colors['common'], colors['common']],
                    alpha=0.8, edgecolor='black', linewidth=0.8, width=0.4)
    axes[0].set_ylim(0, 120)  # Bottom part: 0-120%
    axes[0].set_ylabel('Percentage (%)', fontsize=20, fontweight='bold')
    axes[0].set_xlabel('Files\n(a) Inline data', fontsize=22, fontweight='bold')
    axes[0].tick_params(axis='y', labelsize=18)
    axes[0].set_title('# blocks\n(w/wo. inline data)', fontsize=22, fontweight='bold')
    axes[0].tick_params(axis='x', labelsize=20)
    axes[0].grid(True, alpha=0.3, axis='y')
    axes[0].set_axisbelow(True)

    for bar, percentage in zip(bars1, percentages):
        height = bar.get_height()
        axes[0].text(bar.get_x() + bar.get_width()/2., height + 0.5,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    # ========== Subplot 2: Prealloc vs Extent Uncontig% ==========
    uncontig_workloads = ['8KB\n500 r/w', '16KB\n500 r/w']
    # prealloc_res: {'baseline': [8k, 16k], 'optimization': [8k, 16k]}
    prealloc_uncontig = prealloc_res['optimization']
    extent_uncontig = prealloc_res['baseline']

    ratios_uncontig = [safe_div(prealloc_uncontig[i], extent_uncontig[i]) for i in range(len(uncontig_workloads))]
    percentages_uncontig = [ratio * 100 for ratio in ratios_uncontig]

    bars2 = axes[1].bar(uncontig_workloads, percentages_uncontig, 
                    color=[colors['common']] * len(uncontig_workloads), 
                    alpha=0.8, edgecolor='black', linewidth=0.8, width=0.4)
    # axes[1].set_ylabel('Percentage (%)', fontsize=16, fontweight='bold')
    axes[1].set_ylim(0, 120)  # Bottom part: 0-120%
    axes[1].set_xlabel('(b) Pre-allocation', fontsize=22, fontweight='bold')
    axes[1].tick_params(axis='y', labelsize=18)
    axes[1].set_title('uncontig%\n(w/wo. preallocation)', fontsize=22, fontweight='bold')
    axes[1].tick_params(axis='x', labelsize=18)
    axes[1].grid(True, alpha=0.3, axis='y')
    axes[1].set_axisbelow(True)

    for bar, percentage in zip(bars2, percentages_uncontig):
        height = bar.get_height()
        axes[1].text(bar.get_x() + bar.get_width()/2., height + 0.5,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    # ========== Subplot 3: Access Times to Pre-allocation Pool ==========
    pool_workloads = ['5M\n500 w', '20M\n1000 w']
    # rbtree_res: {'baseline': [5m, 20m], 'optimization': [5m, 20m]}
    rbtree_times = rbtree_res['optimization']
    list_times = rbtree_res['baseline']

    ratios_pool = [safe_div(rbtree_times[i], list_times[i]) for i in range(len(pool_workloads))]
    percentages_pool = [ratio * 100 for ratio in ratios_pool]

    bars3 = axes[2].bar(pool_workloads, percentages_pool,
                    color=[colors['common'], colors['common']],
                    alpha=0.8, edgecolor='black', linewidth=0.8, width=0.4)
    # axes[2].set_ylabel('Percentage (%)', fontsize=16, fontweight='bold')
    axes[2].set_ylim(0, 120)  # Bottom part: 0-120%
    axes[2].set_xlabel('(c) rbtree', fontsize=22, fontweight='bold')
    axes[2].tick_params(axis='y', labelsize=20)
    axes[2].set_title('# access times\n(red-black tree /\n linked list)', fontsize=22, fontweight='bold')
    axes[2].tick_params(axis='x', labelsize=20)
    axes[2].grid(True, alpha=0.3, axis='y')

    for bar, percentage in zip(bars3, percentages_pool):
        height = bar.get_height()
        axes[2].text(bar.get_x() + bar.get_width()/2., height + 0.5,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=16)

    # Adjust layout
    plt.tight_layout()

    # Save as PDF
    save_figure(plt, 'eval-performance-1.pdf')



def draw_performance_plot_2(res_block, res_extent, res_delay, output_dir):
    # Set font to Times New Roman
    setup_font()

    # Create figure with 1 row and 2 columns
    fig, axes = plt.subplots(1, 2, figsize=(12, 4), gridspec_kw={'width_ratios': [1, 0.7]})

    # Define unified color scheme
    colors = {
        'common': '#ade8f4',
        'metadata_reads': '#48cae4',
        'metadata_writes': '#0096c7',
        'data_reads': '#0077b6',
        'data_writes': '#03045e',
    }

    # ========== Subplot 1: IO Operations w/wo extent ==========
    workloads = ['xv6\ncompilation', 'copy\nqemu', 'large\nfile', 'small\nfile']
    
    # Calculate ratios dynamically
    # res_block/extent: [meta_r, meta_w, data_r, data_w]
    metadata_reads = [safe_div(res_extent[i][0], res_block[i][0]) for i in range(4)]
    metadata_writes = [safe_div(res_extent[i][1], res_block[i][1]) for i in range(4)]
    data_reads = [safe_div(res_extent[i][2], res_block[i][2]) for i in range(4)]
    data_writes = [safe_div(res_extent[i][3], res_block[i][3]) for i in range(4)]

    # Convert to percentages
    metadata_reads_pct = [x * 100 for x in metadata_reads]
    metadata_writes_pct = [x * 100 for x in metadata_writes]
    data_reads_pct = [x * 100 for x in data_reads]
    data_writes_pct = [x * 100 for x in data_writes]

    x = np.arange(len(workloads))
    width = 0.2

    bars1a = axes[0].bar(x - 1.5*width, metadata_reads_pct, width, label='Metadata Reads', 
                        color=colors['metadata_reads'], alpha=0.8)
    bars1b = axes[0].bar(x - 0.5*width, metadata_writes_pct, width, label='Metadata Writes',
                        color=colors['metadata_writes'], alpha=0.8)
    bars1c = axes[0].bar(x + 0.5*width, data_reads_pct, width, label='Data Reads', 
                        color=colors['data_reads'], alpha=0.8)
    bars1d = axes[0].bar(x + 1.5*width, data_writes_pct, width, label='Data Writes', 
                        color=colors['data_writes'], alpha=0.8)

    axes[0].set_ylim(0, 120)  # Bottom part: 0-120%
    axes[0].set_ylabel('Percentage (%)', fontsize=20, fontweight='bold')
    axes[0].set_xlabel('(d) Extent', fontsize=22, fontweight='bold')
    axes[0].set_title('# IO operations\n(w/wo. extent)', fontsize=22, fontweight='bold')
    axes[0].set_xticks(x)
    axes[0].tick_params(axis='y', labelsize=20)
    axes[0].set_xticklabels(workloads, rotation=0, ha='center', fontsize=20)
    axes[0].grid(True, alpha=0.3, axis='y')

    # Add percentage labels for each bar with special handling for overlapping labels
    for i, (bar, percentage) in enumerate(zip(bars1a, metadata_reads_pct)):
        # Skip label for the third workload (index 2)
        if i == 2:
            continue
        height = bar.get_height()
        offset = 0
        axes[0].text(bar.get_x() + bar.get_width()/2., height + offset,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    for i, (bar, percentage) in enumerate(zip(bars1b, metadata_writes_pct)):
        if i == 3:
            continue
        if i == 1:
            axes[0].text(bar.get_x() + bar.get_width()/2. + 0.1,  metadata_writes_pct[1],
                        f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)
            continue
        height = bar.get_height()
        # offset = -8 if i in [1, 2] else 0
        offset = 0
        axes[0].text(bar.get_x() + bar.get_width()/2., height + offset,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    for i, (bar, percentage) in enumerate(zip(bars1c, data_reads_pct)):
        # Skip label for the third workload (index 2)
        if i == 1 or i == 2:
            continue
        height = bar.get_height()
        offset = -2 if i in [0] else 0
        # offset = 0
        axes[0].text(bar.get_x() + bar.get_width()/2., height + offset,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    for i, (bar, percentage) in enumerate(zip(bars1d, data_writes_pct)):
        if i == 3:
            continue
        height = bar.get_height()
        offset = 0
        axes[0].text(bar.get_x() + bar.get_width()/2., height + offset,
                    f'{percentage:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    # ========== Subplot 2: IO Operations w/wo delayed allocation ==========
    workloads_delayed = ['xv6\ncompilation', 'copy\nqemu', 'large\nfile', 'small\nfile']
    
    # res_delay: [data_r, data_w]
    # Compare against block baseline data_r (idx 2) and data_w (idx 3)
    data_reads_delayed = [safe_div(res_delay[i][0], res_block[i][2]) for i in range(4)]
    data_writes_delayed = [safe_div(res_delay[i][1], res_block[i][3]) for i in range(4)]

    data_reads_delayed_pct = [x * 100 for x in data_reads_delayed]
    data_writes_delayed_pct = [x * 100 for x in data_writes_delayed]

    x_delayed = np.arange(len(workloads_delayed))
    width_delayed = 0.35

    # Create the bars
    bars2a = axes[1].bar(x_delayed - width_delayed/2, data_reads_delayed_pct, width_delayed, 
                        label='Data Reads', color=colors['data_reads'], alpha=0.8)
    bars2b = axes[1].bar(x_delayed + width_delayed/2, data_writes_delayed_pct, width_delayed,
                        label='Data Writes', color=colors['data_writes'], alpha=0.8)

    # Set up broken axis for subplot 2
    axes[1].set_ylim(0, 120)  # Bottom part: 0-120%
    # axes[1].set_ylabel('Percentage (%)', fontsize=16, fontweight='bold')
    axes[1].set_xlabel('(e) Delayed allocation', fontsize=22, fontweight='bold')
    axes[1].set_title('# IO operations\n(w/wo. delayed allocation)', fontsize=22, fontweight='bold')
    axes[1].set_xticks(x_delayed)
    axes[1].tick_params(axis='y', labelsize=20)
    axes[1].set_xticklabels(workloads_delayed, rotation=0, ha='center', fontsize=20)
    axes[1].grid(True, alpha=0.3, axis='y')

    # Add value labels, with special handling for >120%
    for i, (bar_a, bar_b, pct_a, pct_b) in enumerate(zip(bars2a, bars2b, data_reads_delayed_pct, data_writes_delayed_pct)):
        # Data Reads
        if pct_a > 120:
            # Show arrow pointing up with actual value
            axes[1].annotate(f'{pct_a:.0f}%',
                            xy=(bar_a.get_x() + bar_a.get_width()/2, 120),
                            xytext=(bar_a.get_x() + bar_a.get_width()/2, 110),
                            ha='center', va='top', fontweight='bold', fontsize=18,
                            arrowprops=dict(arrowstyle='->', color=colors['data_reads'], lw=2))
            # Cap the bar height at 120
            bar_a.set_height(120)
        else:
            height_a = bar_a.get_height()
            axes[1].text(bar_a.get_x() + bar_a.get_width()/2., height_a + 1,
                        f'{pct_a:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)
        
        # Data Writes
        height_b = bar_b.get_height()
        if height_b < 120:
            axes[1].text(bar_b.get_x() + bar_b.get_width()/2., height_b + 1,
                        f'{pct_b:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=18)

    # Adjust layout
    plt.tight_layout()

    # Save as PDF
    save_figure(plt, 'eval-performance-2.pdf')


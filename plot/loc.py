import matplotlib.pyplot as plt
import numpy as np
from plot.utils import save_figure, setup_font


def get_data_lists(stats, mapping):
    labels = list(mapping.keys())
    keys = list(mapping.values())
    spec_loc = []
    impl_loc = []
    for k in keys:
        if k in stats:
            spec_loc.append(stats[k][0])
            impl_loc.append(stats[k][1])
        else:
            print(f"Warning: Category '{k}' not found in stats.")
            spec_loc.append(0)
            impl_loc.append(0)
    return labels, spec_loc, impl_loc


def plot_atomfs(stats):
    setup_font()
    mapping = {
        'File': 'file',
        'Inode': 'inode',
        'Interface\nAuxiliary': 'interface-util',
        'Interface': 'interface',
        'Path': 'path',
        'Utility': 'util'
    }
    implementations, spec_loc, impl_loc = get_data_lists(stats, mapping)

    fig, ax = plt.subplots(figsize=(8, 3))
    x = np.arange(len(implementations))
    width = 0.35
    colors = {'spec': '#e8d9d2', 'impl': '#ffbf7a'}

    bars_spec = ax.bar(x - width/2, spec_loc, width, label='Spec LoC', color=colors['spec'], alpha=1)
    bars_impl = ax.bar(x + width/2, impl_loc, width, label='Source Code LoC', color=colors['impl'], alpha=1)

    ax.set_ylim(0, max(max(spec_loc), max(impl_loc)) * 1.2)
    ax.set_ylabel('LOC', fontsize=16, fontweight='bold')
    ax.set_xlabel('(a)AtomFS', fontsize=20, fontweight='bold')
    ax.set_xticks(x)
    ax.tick_params(axis='y', labelsize=16)
    ax.set_xticklabels(implementations, rotation=0, ha='center', fontsize=16)
    ax.grid(True, alpha=0.3, axis='y')

    for i, (bar_spec, bar_impl, spec_val, impl_val) in enumerate(zip(bars_spec, bars_impl, spec_loc, impl_loc)):
        height_spec = bar_spec.get_height()
        ax.text(bar_spec.get_x() + bar_spec.get_width()/2., height_spec + 3,
                f'{spec_val}', ha='center', va='bottom', fontweight='bold', fontsize=18)
        
        height_impl = bar_impl.get_height()
        ax.text(bar_impl.get_x() + bar_impl.get_width()/2. + 0.1, height_impl + 3,
                f'{impl_val}', ha='center', va='bottom', fontweight='bold', fontsize=18)

    plt.tight_layout()
    save_figure(plt, 'eval-loc-atomfs.pdf')


def plot_features(stats):
    setup_font()
    mapping = {
        'Indirect\nBlock': 'indirect-block',
        'Inline\nData': 'inline-data',
        'Extent': 'extent',
        'Pre-Allocation': 'prealloc',
        'rbtree for\nPre-Allocation': 'rbtree',
        'Metadata\nChecksums': 'checksum',
        'Extent\nEncryption': 'encrypt',
        'Delayed\nAllocation': 'delay-alloc',
        'Timestamps': 'timestamp',
        'Logging': 'log'
    }
    implementations, spec_loc, impl_loc = get_data_lists(stats, mapping)

    fig, ax = plt.subplots(figsize=(12, 4))
    x = np.arange(len(implementations))
    width = 0.35
    colors = {'spec': '#e8d9d2', 'impl': '#ffbf7a'}

    bars_spec = ax.bar(x - width/2, spec_loc, width, label='Spec LoC', color=colors['spec'], alpha=1)
    bars_impl = ax.bar(x + width/2, impl_loc, width, label='Source Code LoC', color=colors['impl'], alpha=1)

    ax.set_ylim(0, max(max(spec_loc), max(impl_loc)) * 1.2)
    ax.set_ylabel('LOC', fontsize=16, fontweight='bold')
    ax.set_xlabel('(b)Features', fontsize=20, fontweight='bold')
    ax.set_xticks(x)
    ax.tick_params(axis='y', labelsize=16)
    ax.set_xticklabels(implementations, rotation=30, ha='right', fontsize=18)
    ax.legend(fontsize=16, loc='upper left', frameon=False, fancybox=False, shadow=False)
    ax.grid(True, alpha=0.3, axis='y')

    for i, (bar_spec, bar_impl, spec_val, impl_val) in enumerate(zip(bars_spec, bars_impl, spec_loc, impl_loc)):
        height_spec = bar_spec.get_height()
        ax.text(bar_spec.get_x() + bar_spec.get_width()/2., height_spec + 3,
                f'{spec_val}', ha='center', va='bottom', fontweight='bold', fontsize=16)
        
        height_impl = bar_impl.get_height()
        ax.text(bar_impl.get_x() + bar_impl.get_width()/2. + 0.1, height_impl + 3,
                f'{impl_val}', ha='center', va='bottom', fontweight='bold', fontsize=16)

    plt.tight_layout()
    save_figure(plt, 'eval-loc-feat.pdf')


from pathlib import Path
from matplotlib import font_manager, rcParams


def safe_div(n, d):
    if d == 0:
        return 1.0
    return n / d


def setup_font():
    # Add Times New Roman font
    font_path = Path(__file__).parent / "Times New Roman.ttf"
    font_manager.fontManager.addfont(font_path)
    font_name = font_manager.FontProperties(fname=font_path).get_name()
    rcParams['font.family'] = font_name


def save_figure(plt, filename):
    result_dir = Path(__file__).parent.parent / "result"
    result_dir.mkdir(parents=True, exist_ok=True)
    output_path = result_dir / filename
    plt.savefig(output_path, format='pdf', dpi=300, bbox_inches='tight')
    print(f"Figure saved as {output_path}")
    plt.close()

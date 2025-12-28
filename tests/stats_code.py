import os
from utils import MODULE_PATH

def get_file_lines(filepath, filter_empty=True):
    """Read file and return list of lines, optionally filtering empty lines."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
            if filter_empty:
                return [l.strip() for l in lines if l.strip()]
            return lines
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return []

def parse_spec_file(filepath):
    """Parse spec file, return count of normal lines and list of extracted C code blocks."""
    lines = get_file_lines(filepath, filter_empty=True)
    
    normal_lines_count = 0
    code_blocks = []
    
    is_in_code_block = False
    current_block = []
    
    # Simple state machine to extract code blocks
    for line in lines:
        if line.startswith("```c"):
            is_in_code_block = True
            current_block = []
        elif line.startswith("```") and is_in_code_block:
            is_in_code_block = False
            # Store code block content (as string for deduplication)
            code_blocks.append("\n".join(current_block))
        elif is_in_code_block:
            current_block.append(line)
        else:
            # Normal description lines outside code blocks
            normal_lines_count += 1

    return normal_lines_count, code_blocks

def count_spec_category(category_path):
    """Count lines of a category under specfs directory."""
    total_normal_lines = 0
    unique_code_blocks = set()
    
    if not os.path.exists(category_path):
        return 0

    for root, _, files in os.walk(category_path):
        for file in files:
            if file.endswith(".spec"):
                normal_count, blocks = parse_spec_file(os.path.join(root, file))
                total_normal_lines += normal_count
                for b in blocks:
                    unique_code_blocks.add(b)
    
    # Calculate total lines of deduplicated code blocks
    code_lines_count = 0
    for block in unique_code_blocks:
        # Count non-empty lines in code block
        code_lines_count += len([l for l in block.split('\n') if l.strip()])
        
    return total_normal_lines + code_lines_count

def count_gen_category(category_path):
    """Count total lines of a category under genfs directory."""
    total_lines = 0
    if not os.path.exists(category_path):
        return 0

    for root, _, files in os.walk(category_path):
        for file in files:
            # Count lines for all files (filtering empty lines)
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
                total_lines += len([l for l in lines if l.strip()])
    return total_lines

def collect_all_stats():
    """Collect stats for all categories and return as a dictionary."""
    specfs_root = MODULE_PATH / "eval" / "loc" / "spec"
    genfs_root = MODULE_PATH / "eval" / "loc" / "gen"
    
    # Get all categories (subdirectory names)
    categories = [d for d in os.listdir(specfs_root) if os.path.isdir(specfs_root / d)]
    categories.sort()
    
    stats = {}
    for cat in categories:
        spec_count = count_spec_category(os.path.join(specfs_root, cat))
        gen_count = count_gen_category(os.path.join(genfs_root, cat))
        stats[cat] = (spec_count, gen_count)
    
    return stats

def main():
    stats = collect_all_stats()
    
    print(f"{'Category':<20} | {'Spec Lines (Deduped)':<20} | {'Gen Lines (Total)':<20}")
    print("-" * 65)
    
    grand_total_spec = 0
    grand_total_gen = 0
    
    for cat, (spec_count, gen_count) in stats.items():
        grand_total_spec += spec_count
        grand_total_gen += gen_count
        
        print(f"{cat:<20} | {spec_count:<20} | {gen_count:<20}")

    print("-" * 65)
    print(f"{'TOTAL':<20} | {grand_total_spec:<20} | {grand_total_gen:<20}")

if __name__ == "__main__":
    main()
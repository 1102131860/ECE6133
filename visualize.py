#!/usr/bin/env python3
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import argparse
import random

def read_placement_file(filename):
    """Read placement file, return cell ID, x-coordinate, y-coordinate, width, height"""
    cells = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('#'):
                continue
            parts = line.strip().split()
            if len(parts) >= 5:
                cell_id = int(parts[0])
                x = float(parts[1])
                y = float(parts[2])
                width = float(parts[3])
                height = float(parts[4])
                cells.append((cell_id, x, y, width, height))
    return cells

def read_regions_file(filename):
    """Read region file, return list of regions, each containing cell IDs"""
    regions = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('#'):
                continue
            if line.startswith('Region'):
                parts = line.strip().split(':')
                if len(parts) == 2:
                    cell_ids = [int(x) for x in parts[1].strip().split()]
                    regions.append(cell_ids)
    return regions

def read_nets_file(filename):
    """Read net connection file, return list of nets, each containing cell IDs"""
    nets = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('#'):
                continue
            parts = line.strip().split()
            if len(parts) >= 2:
                net_id = int(parts[0])
                cell_ids = [int(x) for x in parts[1:]]
                nets.append((net_id, cell_ids))
    return nets

def plot_layout(cells, regions=None, nets=None, show_names=False, show_grid=True, save_path=None, 
                show_connections=True, max_nets_to_show=100, connection_alpha=0.3):
    """Plot layout diagram with connectivity information"""
    fig, ax = plt.subplots(figsize=(12, 10))
    
    # Calculate layout boundaries
    min_x = min(cell[1] for cell in cells)
    max_x = max(cell[1] + cell[3] for cell in cells)
    min_y = min(cell[2] for cell in cells)
    max_y = max(cell[2] + cell[4] for cell in cells)
    
    # Add some padding
    padding = max(max_x - min_x, max_y - min_y) * 0.1
    
    # Set axis ranges
    ax.set_xlim(min_x - padding, max_x + padding)
    ax.set_ylim(min_y - padding, max_y + padding)
    
    # If region information is available, assign different colors for each region
    region_colors = {}
    if regions:
        colors = plt.cm.tab20(np.linspace(0, 1, len(regions)))
        for i, region in enumerate(regions):
            for cell_id in region:
                region_colors[cell_id] = colors[i]
    
    # Define different colors for IO cells and standard cells
    io_color = 'red'         # IO cell color
    std_color = 'skyblue'    # Standard cell color
    
    # Create a dictionary to map cell_id to cell position for connection drawing
    cell_positions = {}
    
    # Draw all cells
    for cell_id, x, y, width, height in cells:
        # Store cell center position for connection drawing
        cell_positions[cell_id] = (x + width/2, y + height/2)
        
        # Determine if it's an IO cell (IO cells are typically on the periphery, with non-zero coordinates)
        is_io_cell = (x != 0 or y != 0) and cell_id < 102  # Assuming cells with ID < 102 and non-zero coordinates are IO cells
        
        # Determine cell color
        if cell_id in region_colors:
            # If the cell is in a region, use region color
            facecolor = region_colors[cell_id]
            alpha = 0.7
        else:
            # Otherwise use default IO or standard cell color
            facecolor = io_color if is_io_cell else std_color
            alpha = 0.9 if is_io_cell else 0.7
        
        # Create rectangle to represent the cell
        rect = patches.Rectangle(
            (x, y), width, height, 
            linewidth=1, edgecolor='black', 
            facecolor=facecolor, alpha=alpha
        )
        ax.add_patch(rect)
        
        # Add cell ID at the center of the cell
        if show_names:
            ax.text(x + width/2, y + height/2, str(cell_id), 
                    ha='center', va='center', fontsize=8)
    
    # Draw connections (nets)
    if nets and show_connections:
        # If there are too many nets, select a random subset
        nets_to_draw = nets
        if len(nets) > max_nets_to_show:
            print(f"Too many nets ({len(nets)}), showing only {max_nets_to_show} random nets")
            nets_to_draw = random.sample(nets, max_nets_to_show)
        
        # Draw connections for each net
        for net_id, cell_ids in nets_to_draw:
            # Find the center coordinates of each cell in this net
            valid_cell_ids = [cell_id for cell_id in cell_ids if cell_id in cell_positions]
            
            if len(valid_cell_ids) < 2:
                continue  # Skip nets with fewer than 2 connected cells
            
            # Choose a random color for this net
            net_color = np.random.rand(3,)
            
            # For nets with many cells, use a star topology (connect all to first cell)
            # to avoid too many crossing lines
            if len(valid_cell_ids) > 4:
                # Star topology: connect first cell to all others
                first_cell = valid_cell_ids[0]
                first_pos = cell_positions[first_cell]
                
                for other_cell in valid_cell_ids[1:]:
                    other_pos = cell_positions[other_cell]
                    ax.plot([first_pos[0], other_pos[0]], [first_pos[1], other_pos[1]], 
                            color=net_color, alpha=connection_alpha, linewidth=0.8)
            else:
                # For smaller nets, connect all cells in a chain
                for i in range(len(valid_cell_ids) - 1):
                    cell1 = valid_cell_ids[i]
                    cell2 = valid_cell_ids[i + 1]
                    
                    pos1 = cell_positions[cell1]
                    pos2 = cell_positions[cell2]
                    
                    ax.plot([pos1[0], pos2[0]], [pos1[1], pos2[1]], 
                            color=net_color, alpha=connection_alpha, linewidth=0.8)
    
    # Set grid
    if show_grid:
        ax.grid(True, linestyle=':')
    
    # Set title and labels
    ax.set_title('Circuit Layout Visualization')
    ax.set_xlabel('X Coordinate')
    ax.set_ylabel('Y Coordinate')
    ax.set_aspect('equal')
    
    # Add legend
    io_patch = patches.Patch(color=io_color, label='IO Cells')
    std_patch = patches.Patch(color=std_color, label='Standard Cells')
    legend_elements = [io_patch, std_patch]
    ax.legend(handles=legend_elements, loc='upper right')
    
    plt.tight_layout()
    
    # If save path is provided, save the image
    if save_path:
        plt.savefig(save_path)
        print(f"Layout image saved to {save_path}")
    
    plt.show()

def visualize_all_levels(benchmark_path, show_names=False, show_grid=True, output_dir=None, interval=10, 
                         show_connections=True, max_nets=100):
    """Visualize layouts for all levels"""
    # Parse directory and benchmark name
    benchmark_dir = os.path.dirname(benchmark_path)
    basename = os.path.basename(benchmark_path)
    
    # Check if using new directory structure
    layout_dir = ''
    if os.path.exists(os.path.join(benchmark_path, 'layouts')):
        # New directory structure: output/benchmark/layouts/
        layout_dir = os.path.join(benchmark_path, 'layouts')
        print(f"Detected new directory structure, using layout directory: {layout_dir}")
    else:
        # Old directory structure: benchmark_layouts/
        layout_dir = os.path.join(benchmark_dir, f"{basename}_layouts")
        if not os.path.exists(layout_dir):
            print(f"Error: Layout directory {layout_dir} does not exist")
            return
    
    # Set output directory
    save_dir = output_dir if output_dir else layout_dir
    os.makedirs(save_dir, exist_ok=True)
    
    # Find all level layout files
    level_files = []
    for filename in os.listdir(layout_dir):
        if filename.endswith(".placement") and "level" in filename:
            level_files.append(os.path.join(layout_dir, filename))
    
    # Find all level region files
    region_files = []
    for filename in os.listdir(layout_dir):
        if filename.endswith(".regions"):
            region_files.append(os.path.join(layout_dir, filename))
    
    # Find nets file
    nets_file = None
    for filename in os.listdir(layout_dir):
        if filename == "nets.txt":
            nets_file = os.path.join(layout_dir, filename)
            break
    
    # Load net data if available
    nets_data = None
    if nets_file and os.path.exists(nets_file):
        print(f"Found nets file: {nets_file}")
        nets_data = read_nets_file(nets_file)
    else:
        print("No nets.txt file found, connectivity information will not be displayed")
    
    # Sort by level
    level_files.sort()
    region_files.sort()
    
    # Extract all levels
    levels = []
    for layout_file in level_files:
        try:
            if "level" in layout_file:
                # Handle different file naming formats
                if "_level" in layout_file:
                    # Old format: structP_level0.placement
                    level = int(layout_file.split("_level")[1].split(".")[0])
                else:
                    # New format: level0.placement
                    level = int(os.path.basename(layout_file).split("level")[1].split(".")[0])
                levels.append((level, layout_file))
        except:
            pass
    
    # Sort by level
    levels.sort()
    
    # Only process levels matching the interval
    filtered_levels = []
    for i, (level, path) in enumerate(levels):
        if level == 0 or level % interval == 0 or i == len(levels) - 1:
            filtered_levels.append((level, path))
    
    # Visualize selected levels
    for level, layout_file in filtered_levels:
        print(f"Visualizing level {level}")
        cells = read_placement_file(layout_file)
        
        # Find corresponding region file
        region_data = None
        for region_file in region_files:
            # Compatible with both old and new naming formats
            if f"_level{level}" in region_file or f"/level{level}" in region_file:
                region_data = read_regions_file(region_file)
                break
        
        # Build save path
        save_path = None
        if output_dir:
            save_path = os.path.join(save_dir, f"level{level}.png")
        
        # Draw layout
        plot_layout(cells, region_data, nets_data, show_names, show_grid, save_path, 
                    show_connections, max_nets)
    
    # Visualize final global layout
    final_global_patterns = ["final_global.placement", "_final_global.placement"]
    final_global_file = None
    
    for pattern in final_global_patterns:
        for filename in os.listdir(layout_dir):
            if pattern in filename:
                final_global_file = os.path.join(layout_dir, filename)
                break
        if final_global_file:
            break
    
    if final_global_file and os.path.exists(final_global_file):
        print(f"Visualizing final global layout: {os.path.basename(final_global_file)}")
        cells = read_placement_file(final_global_file)
        
        # Find corresponding region file
        final_region_file = None
        region_patterns = ["final.regions", "_final.regions"]
        for pattern in region_patterns:
            for filename in os.listdir(layout_dir):
                if pattern in filename:
                    final_region_file = os.path.join(layout_dir, filename)
                    break
            if final_region_file:
                break
        
        region_data = None
        if final_region_file and os.path.exists(final_region_file):
            region_data = read_regions_file(final_region_file)
        
        # Build save path
        save_path = None
        if output_dir:
            save_path = os.path.join(save_dir, "final_global.png")
        
        plot_layout(cells, region_data, nets_data, show_names, show_grid, save_path,
                    show_connections, max_nets)
    
    # Visualize row structure layout
    row_structure_patterns = ["row_structure.placement", "_row_structure.placement"]
    row_structure_file = None
    
    for pattern in row_structure_patterns:
        for filename in os.listdir(layout_dir):
            if pattern in filename:
                row_structure_file = os.path.join(layout_dir, filename)
                break
        if row_structure_file:
            break
    
    if row_structure_file and os.path.exists(row_structure_file):
        print(f"Visualizing row structure layout: {os.path.basename(row_structure_file)}")
        cells = read_placement_file(row_structure_file)
        
        # Build save path
        save_path = None
        if output_dir:
            save_path = os.path.join(save_dir, "row_structure.png")
        
        plot_layout(cells, None, nets_data, show_names, show_grid, save_path,
                    show_connections, max_nets)

def main():
    parser = argparse.ArgumentParser(description='Visualize Gordian placement results')
    parser.add_argument('benchmark_path', type=str, help='Benchmark path or output directory')
    parser.add_argument('--names', action='store_true', help='Show cell IDs')
    parser.add_argument('--no-grid', action='store_true', help='Hide grid lines')
    parser.add_argument('--output', '-o', type=str, help='Directory to save images')
    parser.add_argument('--interval', '-i', type=int, default=5, help='Visualization interval, default is 5')
    parser.add_argument('--no-connections', action='store_true', help='Hide connectivity lines')
    parser.add_argument('--max-nets', type=int, default=100, help='Maximum number of nets to display (default: 100)')
    
    args = parser.parse_args()
    
    # Set random seed for reproducible colors
    random.seed(42)
    
    # Check if it's a file or directory
    if os.path.isfile(args.benchmark_path):
        # Single file mode
        print(f"Visualizing single file: {args.benchmark_path}")
        cells = read_placement_file(args.benchmark_path)
        
        # Check if we can find a nets file in the same directory
        nets_file = os.path.join(os.path.dirname(args.benchmark_path), "nets.txt")
        nets_data = None
        if os.path.exists(nets_file):
            print(f"Found nets file: {nets_file}")
            nets_data = read_nets_file(nets_file)
        
        # Build save path
        save_path = None
        if args.output:
            os.makedirs(args.output, exist_ok=True)
            filename = os.path.basename(args.benchmark_path).split('.')[0] + '.png'
            save_path = os.path.join(args.output, filename)
        
        plot_layout(cells, None, nets_data, args.names, not args.no_grid, save_path, 
                    not args.no_connections, args.max_nets)
    else:
        # Multi-level layout mode
        visualize_all_levels(args.benchmark_path, args.names, not args.no_grid, args.output, args.interval,
                             not args.no_connections, args.max_nets)

if __name__ == "__main__":
    main() 
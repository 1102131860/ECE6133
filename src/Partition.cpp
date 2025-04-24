#include "Partition.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include <iostream>
#include <set>
#include <queue>
#include <map>

// Global variable definition
int klMaxIterations = 10; // Reduced maximum iterations to speed up convergence

// Helper function: Check if a region can fit in one row
bool regionFitsOneRow(const std::vector<int>& region_indices,
                     const std::vector<CellDim>& cellDims,
                     double coreWidth) {
    // Calculate total width of all cells in the region
    double totalWidth = 0.0;
    for (int idx : region_indices) {
        if (idx >= 0 && idx < static_cast<int>(cellDims.size())) {
            totalWidth += cellDims[idx].width;
        }
    }
    
    // Region can fit in one row if total width is less than or equal to core width
    return totalWidth <= coreWidth;
}

// Median Cut partitioning
void medianCutPartition(const std::vector<int>& region_indices,
                       const std::vector<double>& global_coords,
                       std::vector<bool>& partition_result) {
    
    // Ensure output vector has correct size
    partition_result.resize(region_indices.size());
    
    // Create vector of index-coordinate pairs for sorting
    std::vector<std::pair<int, double>> index_coord_pairs;
    for (size_t i = 0; i < region_indices.size(); ++i) {
        int idx = region_indices[i];
        if (idx >= 0 && idx < static_cast<int>(global_coords.size())) {
            index_coord_pairs.push_back({i, global_coords[idx]});
        }
    }
    
    // Sort by coordinate
    std::sort(index_coord_pairs.begin(), index_coord_pairs.end(),
             [](const auto& a, const auto& b) { return a.second < b.second; });
    
    // Find median position
    size_t median_pos = index_coord_pairs.size() / 2;
    
    // Partition into two parts
    for (size_t i = 0; i < index_coord_pairs.size(); ++i) {
        int orig_idx = index_coord_pairs[i].first;
        partition_result[orig_idx] = (i < median_pos); // First half is true, second half is false
    }
}

// KL algorithm helper function: Calculate gain for moving a cell from one partition to another
double calculateGain(int cell_idx, 
                    const std::vector<int>& region_indices, 
                    const std::vector<bool>& partition,
                    const PlacementProblem& problem) {
    
    // Safety check
    if (cell_idx < 0 || cell_idx >= (int)region_indices.size() || cell_idx >= (int)partition.size()) {
        std::cerr << "Error: Invalid cell_idx in calculateGain: " << cell_idx << std::endl;
        return 0.0;
    }
    
    double gain = 0.0;
    int global_cell_idx = region_indices[cell_idx];
    
    // Check if global_cell_idx is valid
    if (global_cell_idx < 0 || global_cell_idx >= (int)problem.cellDims.size()) {
        std::cerr << "Error: Invalid global_cell_idx in calculateGain: " << global_cell_idx << ", cellDims.size = " 
                  << problem.cellDims.size() << ", cellToNets.size = " << problem.cellToNets.size() << std::endl;
        return 0.0;
    }
    
    // cellToNets may be larger than cellDims because it might have an extra dummy element (index 0)
    if (global_cell_idx >= (int)problem.cellToNets.size()) {
        std::cerr << "Error: global_cell_idx exceeds cellToNets range" << std::endl;
        return 0.0;
    }
    
    bool current_part = partition[cell_idx];
    
    // Skip if cellToNets is empty
    if (problem.cellToNets[global_cell_idx].empty()) {
        return 0.0;
    }
    
    // Iterate through all nets connected to this cell
    const auto& cell_nets = problem.cellToNets[global_cell_idx];
    
    // Track invalid nets we've already reported to avoid duplicate warnings
    static std::set<int> reported_invalid_nets;
    
    for (int net_idx : cell_nets) {
        // Safety check
        if (net_idx < 0 || net_idx >= (int)problem.nets.size()) {
            // Only output warning once per invalid net_idx
            if (reported_invalid_nets.find(net_idx) == reported_invalid_nets.end()) {
                std::cerr << "Warning: Skipping invalid net_idx: " << net_idx << std::endl;
                reported_invalid_nets.insert(net_idx);
            }
            continue;
        }
        
        const auto& net = problem.nets[net_idx];
        
        // Count cells in part A and part B for this net (excluding current cell)
        int part_A_count = 0;
        int part_B_count = 0;
        
        for (int cell_id : net.cellIDs) {
            // Index adjustment: cell_id is 1-based, need to convert to 0-based
            int cell_id_0based = cell_id - 1;
            
            // Find position of cell_id in region_indices
            auto it = std::find(region_indices.begin(), region_indices.end(), cell_id_0based);
            if (it == region_indices.end()) continue; // Not in current region
            
            int pos = std::distance(region_indices.begin(), it);
            
            // Safety check
            if (pos < 0 || pos >= (int)partition.size()) {
                std::cerr << "Warning: Invalid partition index in calculateGain: " << pos << std::endl;
                continue;
            }
            
            if (pos == cell_idx) continue; // Skip current cell
            
            if (partition[pos]) {
                part_A_count++;
            } else {
                part_B_count++;
            }
        }
        
        // Number of cuts before move
        int cut_before = (current_part ? part_B_count : part_A_count);
        
        // Number of cuts after move (current cell moves from current partition to the other)
        int cut_after = (current_part ? part_A_count : part_B_count);
        
        // Gain = cuts before - cuts after
        gain += (cut_before - cut_after);
    }
    
    return gain;
}

// KL partitioning
void klPartition(const std::vector<int>& region_indices,
                const std::vector<double>& global_coords,
                const PlacementProblem& problem,
                std::vector<bool>& partition_result,
                int maxPasses) {
    
    // If region is too small, use Median Cut directly
    if (region_indices.size() <= 2) {
        medianCutPartition(region_indices, global_coords, partition_result);
        return;
    }
    
    // Initialize with Median Cut
    std::cout << "Initializing KL partitioning with Median Cut" << std::endl;
    medianCutPartition(region_indices, global_coords, partition_result);
    
    // Check partition result
    if (partition_result.size() != region_indices.size()) {
        std::cerr << "Error: Partition result size mismatch, using initial partition." << std::endl;
        return;
    }
    
    // Output partition sizes
    int part_A_count = 0;
    for (bool in_A : partition_result) {
        if (in_A) part_A_count++;
    }
    int part_B_count = partition_result.size() - part_A_count;
    std::cout << "Initial partition: A=" << part_A_count << ", B=" << part_B_count << std::endl;
    
    // Current best partition
    std::vector<bool> best_partition = partition_result;
    
    // Add cumulative total gain variable
    double cumulative_total_gain = 0.0;
    
    // KL algorithm main loop
    for (int pass = 0; pass < maxPasses; ++pass) {
        std::cout << "KL Pass " << (pass+1) << "/" << maxPasses << std::endl;
        
        // Current working partition
        std::vector<bool> current_partition = best_partition;
        std::vector<bool> locked(region_indices.size(), false);
        
        // Find maximum gain for each unlocked node pair
        bool improved = false;
        double pass_gain = 0;
        
        while (true) {
            // Select first unlocked node pair with positive gain
            int best_a = -1, best_b = -1;
            double best_gain = 0; // Only accept positive gain
            
            // Fast search mode: stop after finding first positive gain
            bool found_positive_gain = false;
            
            for (size_t i = 0; i < region_indices.size() && !found_positive_gain; ++i) {
                if (locked[i]) continue;
                
                for (size_t j = 0; j < region_indices.size() && !found_positive_gain; ++j) {
                    if (locked[j] || i == j) continue;
                    
                    // Only consider nodes in different partitions
                    if (current_partition[i] == current_partition[j]) continue;
                    
                    try {
                        // Calculate swap gain
                        double gain = calculateGain(i, region_indices, current_partition, problem) + 
                                     calculateGain(j, region_indices, current_partition, problem);
                        
                        if (gain > 0) {
                            best_gain = gain;
                            best_a = i;
                            best_b = j;
                            found_positive_gain = true; // Found positive gain, stop search immediately
                            break;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Exception during gain calculation: " << e.what() << std::endl;
                        continue;
                    }
                }
                if (found_positive_gain) break;
            }
            
            // If no swappable node pair found or swap has no gain
            if (best_a == -1 || best_b == -1 || best_gain <= 0) {
                break;
            }
            
            // Perform swap
            std::swap(current_partition[best_a], current_partition[best_b]);
            locked[best_a] = true;
            locked[best_b] = true;
            
            pass_gain += best_gain;
            
            std::cout << "  Swapping nodes " << best_a << " and " << best_b << ", gain: " << best_gain << std::endl;
            
            // After finding a positive gain swap, apply it immediately and end this pass
            if (found_positive_gain) {
                improved = true;
                break;
            }
        }
        
        // If this pass improved total gain, update best partition
        if (pass_gain > 0) {
            // Update cumulative total gain
            cumulative_total_gain += pass_gain;
            
            std::cout << "  Pass complete, gain: " << pass_gain << ", cumulative total gain: " << cumulative_total_gain << std::endl;
            best_partition = current_partition;
            improved = true;
        } else {
            std::cout << "  Pass did not improve gain, stopping KL algorithm" << std::endl;
            break;
        }
        
        // If no improvement, terminate KL algorithm
        if (!improved) {
            break;
        }
    }
    
    // Return best partition result
    partition_result = best_partition;
    
    // Output final partition sizes
    part_A_count = 0;
    for (bool in_A : partition_result) {
        if (in_A) part_A_count++;
    }
    part_B_count = partition_result.size() - part_A_count;
    std::cout << "KL partitioning complete: A=" << part_A_count << ", B=" << part_B_count << ", total gain: " << cumulative_total_gain << std::endl;
}

// Recursively partition regions
void partitionRegions(std::vector<std::vector<int>>& regions,
                     const std::vector<double>& global_coords,
                     const PlacementProblem& problem,
                     PartitionMethod partitionMethod,
                     int maxDepth) {
    
    if (regions.empty()) {
        // Initial case: create a single region containing all non-IO cells
        std::vector<int> all_cells;
        int num_io_cells = problem.ioCells.size();
        for (size_t i = num_io_cells; i < problem.cellDims.size(); ++i) {
            all_cells.push_back(i);
        }
        regions.push_back(all_cells);
    }
    
    // If max depth is 0, don't perform any partitioning
    if (maxDepth == 0) {
        return;
    }
    
    // Queue of regions to process further, each element is a pair: <region index, current depth>
    std::queue<std::pair<size_t, int>> regionQueue;
    for (size_t i = 0; i < regions.size(); ++i) {
        regionQueue.push({i, 1}); // Initial depth is 1
    }
    
    // Process each region in the queue
    while (!regionQueue.empty()) {
        auto [region_idx, current_depth] = regionQueue.front();
        regionQueue.pop();
        
        // If current depth exceeds max depth, don't partition further
        if (maxDepth > 0 && current_depth > maxDepth) {
            continue;
        }
        
        // Skip empty regions
        if (regions[region_idx].empty()) continue;
        
        // Check if region already fits in one row
        if (regionFitsOneRow(regions[region_idx], problem.cellDims, problem.chip.coreW)) {
            continue; // Already small enough, no need to partition further
        }
        
        // Partition current region
        std::vector<bool> partition;
        size_t region_size = regions[region_idx].size();
        
        // Use Median Cut for large regions, otherwise use specified method
        bool use_median_cut = partitionMethod == PartitionMethod::MEDIAN_CUT || region_size > 200;
        
        if (use_median_cut) {
            medianCutPartition(regions[region_idx], global_coords, partition);
        } else {
            // Use KL algorithm
            std::cout << "Using KL algorithm for region " << region_idx << " (" << region_size << " cells)" << std::endl;
            klPartition(regions[region_idx], global_coords, problem, partition, klMaxIterations);
        }
        
        // Create two new regions based on partition result
        std::vector<int> region_A, region_B;
        for (size_t i = 0; i < regions[region_idx].size(); ++i) {
            if (i < partition.size()) { // Safety check
                if (partition[i]) {
                    region_A.push_back(regions[region_idx][i]);
                } else {
                    region_B.push_back(regions[region_idx][i]);
                }
            }
        }
        
        // Replace original region with A, and add B to regions list
        regions[region_idx] = region_A;
        regions.push_back(region_B);
        
        // If new regions still need further partitioning, add to queue (depth+1)
        if (!regionFitsOneRow(region_A, problem.cellDims, problem.chip.coreW)) {
            regionQueue.push({region_idx, current_depth + 1});
        }
        if (!regionFitsOneRow(region_B, problem.cellDims, problem.chip.coreW)) {
            regionQueue.push({regions.size() - 1, current_depth + 1});
        }
    }
}

// Calculate optimal chip dimensions and row count
void calculateOptimalDimensions(const std::vector<CellDim>& cellDims,
                              double& optimalWidth,
                              double& optimalHeight,
                              int& numRows) {
    
    // Skip IO cells (typically the first N are IO cells)
    std::vector<CellDim> stdCells;
    int numIOCells = 0;
    for (const auto& cell : cellDims) {
        if (cell.height == 1.4) { // Standard cell row height
            stdCells.push_back(cell);
        } else {
            numIOCells++;
        }
    }
    
    // Calculate total area
    double totalArea = 0.0;
    double totalWidth = 0.0;
    for (const auto& cell : stdCells) {
        totalArea += cell.width * cell.height;
        totalWidth += cell.width;
    }
    
    // Calculate standard cell row height (typically fixed)
    double rowHeight = 1.4; // Based on data file
    
    // Calculate optimal chip shape (close to square)
    double estimatedRows = std::sqrt(totalArea / (rowHeight * rowHeight));
    numRows = std::max(1, static_cast<int>(std::round(estimatedRows)));
    
    // Calculate chip dimensions
    optimalWidth = totalWidth / numRows;
    optimalHeight = numRows * rowHeight;
    
    // Adjust to be closer to square
    double aspectRatio = optimalWidth / optimalHeight;
    if (aspectRatio > 1.2) {
        // Width too large, increase row count
        numRows++;
        optimalWidth = totalWidth / numRows;
        optimalHeight = numRows * rowHeight;
    } else if (aspectRatio < 0.8) {
        // Height too large, decrease row count
        numRows = std::max(1, numRows - 1);
        optimalWidth = totalWidth / numRows;
        optimalHeight = numRows * rowHeight;
    }
    
    // Fine-tune to get aspect ratio close to 1
    int adjustments = 0;
    while (adjustments < 5) {
        aspectRatio = optimalWidth / optimalHeight;
        if (aspectRatio > 1.05) {
            numRows++;
        } else if (aspectRatio < 0.95) {
            numRows = std::max(1, numRows - 1);
        } else {
            break; // Already good enough
        }
        
        optimalWidth = totalWidth / numRows;
        optimalHeight = numRows * rowHeight;
        adjustments++;
    }
}

#include "PlacementProblem.h"
#include "QPSolver.h"
#include "Partition.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>

// Calculate target positions for regions
std::pair<std::vector<double>, std::vector<double>> calculateTargetPositions(
    const std::vector<std::vector<int>>& regions,
    const PlacementProblem& problem,
    int level,
    int totalLevels) {
    
    std::vector<double> targetX, targetY;
    
    if (regions.empty()) {
        return {targetX, targetY};
    }
    
    if (level == 0) {
        // First level: target is the chip center
        targetX.push_back(problem.chip.coreW / 2.0);
        targetY.push_back(problem.chip.coreH / 2.0);
        return {targetX, targetY};
    }
    
    // Chip dimensions
    double coreW = problem.chip.coreW;
    double coreH = problem.chip.coreH;
    
    // Calculate number of valid regions (skip empty regions)
    size_t validRegions = 0;
    for (const auto& region : regions) {
        if (!region.empty()) {
            validRegions++;
        }
    }
    
    if (validRegions == 0) {
        return {targetX, targetY};
    }
    
    // Calculate target positions for each region
    for (size_t i = 0; i < regions.size(); ++i) {
        if (regions[i].empty()) continue;
        
        // Calculate region horizontal position ratio
        double region_ratio = static_cast<double>(i) / regions.size();
        
        // Horizontal uniform distribution
        // Divide chip width into regions.size() parts, place each region at the center of these divisions
        double x_target = coreW * (region_ratio + 0.5 / regions.size());
        
        // Vertical layering strategy
        double y_target;
        
        if (level < totalLevels) {
            // In Gordian's different levels: distribute vertically based on level and region index
            double row_height = coreH / std::min(static_cast<size_t>(level + 1), regions.size());
            y_target = coreH - row_height * (0.5 + i % (level + 1));
        } else {
            // Final layout strategy: try to distribute evenly in vertical direction
            double y_ratio = static_cast<double>(i % (totalLevels+1)) / (totalLevels+1);
            y_target = coreH * (y_ratio + 0.5 / (totalLevels+1));
        }
        
        targetX.push_back(x_target);
        targetY.push_back(y_target);
    }
    
    return {targetX, targetY};
}

// Helper function: Print current placement status
void printPlacementStatus(const PlacementProblem& problem, 
                         const std::vector<double>& xCoords, 
                         const std::vector<double>& yCoords,
                         const std::vector<std::vector<int>>& regions = {}) {
    
    int numCells = problem.cellDims.size();
    int numIOs = problem.ioCells.size();
    
    // Print non-IO cell coordinates
    std::cout << "Standard Cell Coordinates (first 10):" << std::endl;
    int displayCount = 0;
    // Start from the first non-IO cell (usually IO cells have indices from 0 to numIOs-1)
    for (int i = numIOs; i < numCells && displayCount < 10; ++i) {
        // Skip cells with coordinates still at (0,0), unless all cells are at (0,0)
        if ((xCoords[i] == 0.0 && yCoords[i] == 0.0) && i < numCells - 1) {
            continue;
        }
        std::cout << "Cell " << i << ": (" << std::fixed << std::setprecision(2)
                  << xCoords[i] << ", " << yCoords[i] << "), Size: " 
                  << problem.cellDims[i].width << " x " << problem.cellDims[i].height
                  << std::endl;
        displayCount++;
    }
    
    // If all non-IO cells are at (0,0), display at least one
    if (displayCount == 0 && numIOs < numCells) {
        int i = numIOs;
        std::cout << "Cell " << i << ": (" << std::fixed << std::setprecision(2)
                  << xCoords[i] << ", " << yCoords[i] << "), Size: " 
                  << problem.cellDims[i].width << " x " << problem.cellDims[i].height
                  << std::endl;
    }
    
    // Print region information (if available)
    if (!regions.empty()) {
        std::cout << "Current number of regions: " << regions.size() << std::endl;
        // Print the number of cells in each region
        for (size_t i = 0; i < std::min(regions.size(), size_t(10)); ++i) {
            std::cout << "Region " << i << " contains " << regions[i].size() << " cells"
                      << std::endl;
        }
    }
}

// Helper function: Get base name from path (without extension)
std::string getBaseName(const std::string& path) {
    size_t pos = path.find_last_of('/');
    std::string name = (pos == std::string::npos) ? path : path.substr(pos + 1);
    
    size_t dot_pos = name.find_last_of('.');
    if (dot_pos != std::string::npos) {
        name = name.substr(0, dot_pos);
    }
    
    return name;
}

// Convert global placement to standard cell row structure
void convertToRowStructure(PlacementProblem& problem,
                          std::vector<double>& xCoords,
                          std::vector<double>& yCoords,
                          const std::vector<std::vector<int>>& regions) {
    
    // Ensure all regions' cells can fit in one row
    for (const auto& region : regions) {
        if (!regionFitsOneRow(region, problem.cellDims, problem.chip.coreW)) {
            // std::cerr << "Error: Some regions cannot fit in one row, please complete partitioning first" << std::endl;
            // return;
        }
    }
    
    // Place cells in each region into rows
    double rowHeight = 1.4; // Standard cell row height
    double rowSpacing = rowHeight; // Row spacing equals row height
    
    // Determine the row number for each region
    int numRows = problem.chip.numStdRows;
    std::vector<int> regionToRow(regions.size(), -1);
    
    // Sort regions by their average y-coordinate
    std::vector<std::pair<double, int>> regionYCoords;
    for (size_t i = 0; i < regions.size(); ++i) {
        const auto& region = regions[i];
        if (region.empty()) continue;
        
        // Calculate region's center y-coordinate
        double sumY = 0.0;
        for (int cellIdx : region) {
            sumY += yCoords[cellIdx];
        }
        double avgY = sumY / region.size();
        regionYCoords.push_back({avgY, i});
    }
    
    // Sort regions by y-coordinate
    std::sort(regionYCoords.begin(), regionYCoords.end());
    
    // Assign row numbers (from bottom to top)
    for (size_t i = 0; i < regionYCoords.size(); ++i) {
        int regionIdx = regionYCoords[i].second;
        // Even rows are actual cell rows, odd rows are empty
        regionToRow[regionIdx] = i % (numRows / 2);
    }
    
    // Update cell coordinates
    for (size_t regionIdx = 0; regionIdx < regions.size(); ++regionIdx) {
        const auto& region = regions[regionIdx];
        if (region.empty() || regionToRow[regionIdx] == -1) continue;
        
        int row = regionToRow[regionIdx];
        double yPos = row * 2 * rowHeight + rowHeight / 2; // Row's center y-coordinate
        
        // Sort cells within the region by x-coordinate
        std::vector<std::pair<double, int>> cellXCoords;
        for (int cellIdx : region) {
            cellXCoords.push_back({xCoords[cellIdx], cellIdx});
        }
        std::sort(cellXCoords.begin(), cellXCoords.end());
        
        // Place cells from left to right
        double currentX = 0.0;
        for (const auto& [_, cellIdx] : cellXCoords) {
            // Update cell coordinates
            xCoords[cellIdx] = currentX + problem.cellDims[cellIdx].width / 2; // Cell center x-coordinate
            yCoords[cellIdx] = yPos;
            
            // Move to next position
            currentX += problem.cellDims[cellIdx].width;
        }
    }
}

// Save layout to file (add level information)
void saveLayoutToFile(const std::string& filename, 
                     const std::vector<double>& xCoords, 
                     const std::vector<double>& yCoords,
                     const std::vector<CellDim>& cellDims) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << "# Cell_ID X Y Width Height" << std::endl;
        for (size_t i = 0; i < cellDims.size(); ++i) {
            outFile << i << " " << xCoords[i] << " " << yCoords[i] << " "
                    << cellDims[i].width << " " << cellDims[i].height
                    << std::endl;
        }
        outFile.close();
        std::cout << "Layout saved to: " << filename << std::endl;
    } else {
        std::cerr << "Could not create output file: " << filename << std::endl;
    }
}

// Save region information to file
void saveRegionsToFile(const std::string& filename,
                      const std::vector<std::vector<int>>& regions) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << "# Number of regions: " << regions.size() << std::endl;
        for (size_t i = 0; i < regions.size(); ++i) {
            outFile << "Region " << i << ":";
            for (int cellId : regions[i]) {
                outFile << " " << cellId;
            }
            outFile << std::endl;
        }
        outFile.close();
        std::cout << "Region information saved to: " << filename << std::endl;
    } else {
        std::cerr << "Could not create output file: " << filename << std::endl;
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <benchmark_prefix> [partition_method]" << std::endl;
        std::cerr << "partition_method can be KL or MEDIAN (default is KL)" << std::endl;
        return 1;
    }
    
    std::string benchmarkPrefix = argv[1];
    PlacementProblem problem;
    
    // Get benchmark name (remove path and extension)
    std::string benchmarkName = getBaseName(benchmarkPrefix);
    
    // Determine partitioning method
    PartitionMethod partitionMethod = PartitionMethod::KL; // Default to KL
    if (argc >= 3) {
        std::string method = argv[2];
        if (method == "MEDIAN" || method == "median") {
            partitionMethod = PartitionMethod::MEDIAN_CUT;
            std::cout << "Using MEDIAN_CUT partitioning method" << std::endl;
        } else if (method == "KL" || method == "kl") {
            partitionMethod = PartitionMethod::KL;
            std::cout << "Using KL partitioning method" << std::endl;
        } else {
            std::cerr << "Unknown partitioning method: " << method << ", using default KL" << std::endl;
        }
    } else {
        std::cout << "No partitioning method specified, using default KL" << std::endl;
    }
    
    // Load benchmark data
    if (!problem.loadFromFiles(benchmarkPrefix)) {
        std::cerr << "Failed to load benchmark files: " << benchmarkPrefix << std::endl;
        return 1;
    }
    
    std::cout << "Successfully loaded benchmark file: " << benchmarkName << std::endl;
    std::cout << "Number of cells: " << problem.cellDims.size() << std::endl;
    std::cout << "Number of IO cells: " << problem.ioCells.size() << std::endl;
    std::cout << "Number of nets: " << problem.nets.size() << std::endl;
    
    // Initialize cell coordinates
    std::vector<double> xCoords(problem.cellDims.size(), 0.0);
    std::vector<double> yCoords(problem.cellDims.size(), 0.0);
    
    // Initial setup of IO cell coordinates
    for (const auto& ioCell : problem.ioCells) {
        int idx = ioCell.id - 1; // Convert to 0-based index
        xCoords[idx] = ioCell.prefX;
        yCoords[idx] = ioCell.prefY;
    }
    
    // Create unified output directory structure
    std::string outputBaseDir = "output";
    std::string outputDir = outputBaseDir + "/" + benchmarkName;
    std::string layoutDir = outputDir + "/layouts";
    
    // Create output directory
    std::string mkdirCmd = "mkdir -p " + outputDir;
    system(mkdirCmd.c_str());
    
    // Create layouts subdirectory
    mkdirCmd = "mkdir -p " + layoutDir;
    system(mkdirCmd.c_str());
    
    // Save initial layout
    std::string initLayoutFile = outputDir + "/init.placement";
    saveLayoutToFile(initLayoutFile, xCoords, yCoords, problem.cellDims);
    
    // Gordian algorithm parameters
    double alpha = 1.0; // over-relaxation parameter
    int maxIterations = 100; // maximum iterations
    int levels = 20; // partitioning levels
    
    // Initialize region partitioning (start with a single region containing all standard cells)
    std::vector<std::vector<int>> regions;
    // Create a single region containing all non-IO cells
    std::vector<int> all_cells;
    int num_io_cells = problem.ioCells.size();
    for (size_t i = num_io_cells; i < problem.cellDims.size(); ++i) {
        all_cells.push_back(i);
    }
    regions.push_back(all_cells);
    
    // Main steps of Gordian algorithm
    for (int level = 0; level < levels; ++level) {
        std::cout << "\n===== Gordian Algorithm Level " << level << " =====" << std::endl;
        
        // Print current placement status
        printPlacementStatus(problem, xCoords, yCoords, regions);
        
        // Calculate region target positions
        auto [targetX, targetY] = calculateTargetPositions(regions, problem, level, levels);
        
        // Solve QP for x direction
        std::cout << "Solving QP for x direction..." << std::endl;
        solveGlobalPlacementQP(problem, xCoords, true, alpha, maxIterations, regions, targetX);
        
        // Solve QP for y direction
        std::cout << "Solving QP for y direction..." << std::endl;
        solveGlobalPlacementQP(problem, yCoords, false, alpha, maxIterations, regions, targetY);
        
        // Save layout for current level
        std::string levelLayoutFile = layoutDir + "/level" + std::to_string(level) + ".placement";
        saveLayoutToFile(levelLayoutFile, xCoords, yCoords, problem.cellDims);
        
        // Update region partitioning, level 0 isn't partitioned, level 1 is partitioned once, and so on
        if (level > 0) {
            std::cout << "Performing region partitioning..." << std::endl;
            // Maximum partitioning depth for the level is the level number
            partitionRegions(regions, xCoords, problem, partitionMethod, level);
            
            // Save region information
            std::string regionFile = layoutDir + "/level" + std::to_string(level) + ".regions";
            saveRegionsToFile(regionFile, regions);
            
            std::cout << "Number of regions after partitioning: " << regions.size() << std::endl;
        } else {
            std::cout << "Level " << level << " does not perform partitioning, current number of regions: " << regions.size() << std::endl;
            
            // Still save region information, even if unchanged
            std::string regionFile = layoutDir + "/level" + std::to_string(level) + ".regions";
            saveRegionsToFile(regionFile, regions);
        }
    }
    
    // Final QP optimization
    std::cout << "\n===== Final Global Placement Optimization =====" << std::endl;
    
    // Calculate final region target positions
    auto [finalTargetX, finalTargetY] = calculateTargetPositions(regions, problem, levels, levels);
    
    solveGlobalPlacementQP(problem, xCoords, true, alpha, maxIterations, regions, finalTargetX);
    solveGlobalPlacementQP(problem, yCoords, false, alpha, maxIterations, regions, finalTargetY);
    
    // Save global placement
    std::string finalGlobalLayoutFile = layoutDir + "/final_global.placement";
    saveLayoutToFile(finalGlobalLayoutFile, xCoords, yCoords, problem.cellDims);
    
    // Print global placement status
    std::cout << "Global placement completed" << std::endl;
    printPlacementStatus(problem, xCoords, yCoords, regions);
    
    // Save final region information
    std::string finalRegionFile = layoutDir + "/final.regions";
    saveRegionsToFile(finalRegionFile, regions);
    
    // Convert to standard cell row structure
    std::cout << "\n===== Converting to Standard Cell Row Structure =====" << std::endl;
    convertToRowStructure(problem, xCoords, yCoords, regions);
    
    // Print final placement status
    std::cout << "Final placement completed" << std::endl;
    printPlacementStatus(problem, xCoords, yCoords);
    
    // Output final results to file
    std::string outputFile = outputDir + "/placement";
    std::ofstream outFile(outputFile);
    if (outFile.is_open()) {
        outFile << "# Cell_ID X Y Width Height" << std::endl;
        for (size_t i = 0; i < problem.cellDims.size(); ++i) {
            outFile << i << " " << xCoords[i] << " " << yCoords[i] << " "
                    << problem.cellDims[i].width << " " << problem.cellDims[i].height
                    << std::endl;
        }
        outFile.close();
        std::cout << "Layout result saved to: " << outputFile << std::endl;
    } else {
        std::cerr << "Could not create output file: " << outputFile << std::endl;
    }
    
    // Calculate optimal chip dimensions and row count
    double optimalWidth, optimalHeight;
    int optimalRows;
    calculateOptimalDimensions(problem.cellDims, optimalWidth, optimalHeight, optimalRows);
    
    std::cout << "Optimal chip dimensions: " << optimalWidth << " x " << optimalHeight 
              << ", Number of rows: " << optimalRows << std::endl;

    // Save row structure layout
    std::string rowLayoutFile = layoutDir + "/row_structure.placement";
    saveLayoutToFile(rowLayoutFile, xCoords, yCoords, problem.cellDims);
    
    // Generate a file specifically for chip information, for visualization
    std::string chipInfoFile = layoutDir + "/chip_info.txt";
    std::ofstream chipFile(chipInfoFile);
    if (chipFile.is_open()) {
        chipFile << "# Chip and Row Information" << std::endl;
        chipFile << "CoreWidth " << problem.chip.coreW << std::endl;
        chipFile << "CoreHeight " << problem.chip.coreH << std::endl;
        chipFile << "NumRows " << problem.chip.numStdRows << std::endl;
        chipFile << "OptimalWidth " << optimalWidth << std::endl;
        chipFile << "OptimalHeight " << optimalHeight << std::endl;
        chipFile << "OptimalRows " << optimalRows << std::endl;
        chipFile.close();
        std::cout << "Chip information saved to: " << chipInfoFile << std::endl;
    }
    
    // Save net connection information
    std::string netFile = layoutDir + "/nets.txt";
    std::ofstream netsFile(netFile);
    if (netsFile.is_open()) {
        netsFile << "# Net Connection Information" << std::endl;
        netsFile << "# NetID CellIDs..." << std::endl;
        for (const auto& net : problem.nets) {
            netsFile << net.id;
            for (int cellId : net.cellIDs) {
                netsFile << " " << cellId;
            }
            netsFile << std::endl;
        }
        netsFile.close();
        std::cout << "Net connection information saved to: " << netFile << std::endl;
    }
    
    std::cout << "All layout data has been saved to the " << outputDir << " directory" << std::endl;
    std::cout << "Use the visualization script to view layout results: python visualize_all.py " << outputDir << std::endl;
    
    return 0;
}

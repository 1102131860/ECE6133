#include "Parser.h"
#include "PlacementProblem.h"
#include <cmath>
#include <algorithm>
#include <iostream>

static void buildCellToNets(const std::vector<Net>& nets, std::vector<std::vector<int>>& cellToNets, int num_cells) {
    cellToNets.assign(num_cells + 1, {});
    for (auto &net : nets) {
        for (int cid : net.cellIDs) {
            // Convert 1-based IDs to 0-based indices
            int cell_idx = cid - 1;
            int net_idx = net.id - 1;
            if (cell_idx >= 0 && cell_idx < num_cells) {
                cellToNets[cell_idx].push_back(net_idx);
            }
        }
    }
}

static void snapIORows(std::vector<IOCell>& ioCells, const ChipInfo& chip) {
    double rowHeight = chip.coreH / chip.numStdRows;
#ifdef DEBUG
    std::cout << "Debug: snapIORows - rowHeight=" << rowHeight << std::endl;
#endif
    
    for (auto &io : ioCells) {
        double relY = io.prefY - chip.ioToBottom;
        int row = std::round(relY / rowHeight);
        io.snappedRow = std::max(0, std::min(chip.numStdRows - 1, row));
#ifdef DEBUG
        std::cout << "IO Cell " << io.id << ": prefY=" << io.prefY 
                  << ", relY=" << relY 
                  << ", row=" << row 
                  << ", snappedRow=" << io.snappedRow << std::endl;
#endif
    }
}

bool PlacementProblem::loadFromFiles(const std::string& prefix) {
    try {
        parseHGR(prefix + ".hgr", nets);
        parseDIM(prefix + ".dim", chip, cellDims);
        parseIO(prefix + ".io", ioCells);
        
        // Assign IDs to IO cells (starting from 1)
        for (size_t i = 0; i < ioCells.size(); ++i) {
            ioCells[i].id = i + 1;  // Assign 1-based IDs to IO cells
        }
        
        // auxiliary functions
        buildCellToNets(nets, cellToNets, cellDims.size());
        snapIORows(ioCells, chip);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading benchmark: " << e.what() << std::endl;
        return false;
    }
}
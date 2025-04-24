#ifndef PLACEMENT_PROBLEM_H
#define PLACEMENT_PROBLEM_H

#include <vector>
#include <string>

// ChipInfo: Chip geometric information read from the first line of .dim file
struct ChipInfo {
    double coreW;
    double coreH;
    double ioToLeft;
    double ioToBottom;
    int    numStdRows;
};

// CellDim: Each cell's width/height/power (from .dim file)
struct CellDim {
    double width;
    double height;
    double power;
};

// Net: List of cell indices contained in each net (from .hgr file)
struct Net {
    int              id;
    std::vector<int> cellIDs;
};

// IOCell: Preferred boundary coordinates for the first N cells (from .io file)
struct IOCell {
    int    id;
    double prefX;
    double prefY;
    int    snappedRow;
};

class PlacementProblem {
public:
    ChipInfo                         chip;
    std::vector<CellDim>             cellDims;
    std::vector<Net>                 nets;
    std::vector<IOCell>              ioCells;
    std::vector<std::vector<int>>    cellToNets;

    bool loadFromFiles(const std::string& prefix);
};

#endif // PLACEMENT_PROBLEM_H 
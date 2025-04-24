#include "Parser.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>

void parseHGR(const std::string& path, std::vector<Net>& nets) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Unable to open HGR file: " + path);
    }

    // read first line and reserve vector
    int numNets, numCells;
    in >> numNets >> numCells;
    nets.reserve(numNets);

#ifdef DEBUG
    std::cout << "Debug: Parsing HGR file " << path << std::endl;
    std::cout << "Number of nets: " << numNets << ", Number of cells: " << numCells << std::endl;
#endif

    // skip the rest of first line
    std::string line;
    std::getline(in, line); 
    
    // read rest of lines
    for (int i = 0; i < numNets; ++i) {
        std::getline(in, line);
        std::istringstream iss(line);

        // read net id
        Net net; 
        net.id = i;
        // read cell ids
        int cid;
        while (iss >> cid) {
            net.cellIDs.push_back(cid);
        }
        
#ifdef DEBUG
        std::cout << "Net " << i << " connects cells: ";
        for (int cid : net.cellIDs) {
            std::cout << cid << " ";
        }
        std::cout << std::endl;
#endif
        
        // add to nets vector
        nets.push_back(net);
    }
}

void parseDIM(const std::string& path, ChipInfo &chip, std::vector<CellDim> &cellDims) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Unable to open DIM file: " + path);
    }
    
    // read first line
    in >> chip.coreW >> chip.coreH >> chip.ioToLeft >> chip.ioToBottom >> chip.numStdRows;

#ifdef DEBUG
    std::cout << "Debug: Parsing DIM file " << path << std::endl;
    std::cout << "Chip info: coreW=" << chip.coreW 
              << ", coreH=" << chip.coreH 
              << ", ioToLeft=" << chip.ioToLeft 
              << ", ioToBottom=" << chip.ioToBottom 
              << ", numStdRows=" << chip.numStdRows << std::endl;
#endif

    // skip the rest of first line
    std::string line;
    std::getline(in, line);

    // ensure cellDims is clear
    cellDims.clear();
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        // replace comma with space
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream iss(line);

        // add width, height, and power
        double w, h, p;
        iss >> w >> h >> p;
#ifdef DEBUG
        std::cout << "Cell dim: w=" << w << ", h=" << h << ", p=" << p << std::endl;
#endif
        cellDims.push_back({w, h, p});
    }
}

void parseIO(const std::string& path, std::vector<IOCell>& ioCells) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Unable to open IO file: " + path);
    }

    // read number of IO points
    int numIO;
    in >> numIO;
    ioCells.reserve(numIO);

#ifdef DEBUG
    std::cout << "Debug: Parsing IO file " << path << std::endl;
    std::cout << "Number of IO cells: " << numIO << std::endl;
#endif

    // read x and y axis
    std::string line;
    std::getline(in, line); // skip the rest of first line
    for (int i = 0; i < numIO; ++i) {
        std::getline(in, line);
        // replace comma with space
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream iss(line);
        
        double x, y;
        iss >> x >> y;
#ifdef DEBUG
        std::cout << "IO Cell " << i << ": x=" << x << ", y=" << y << std::endl;
#endif
        ioCells.push_back({i, x, y, -1});   // let's firstly initialize snappedRow with -1
    }
}

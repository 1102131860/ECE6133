#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "PlacementProblem.h"

// Parse net list from .hgr file
void parseHGR(const std::string& path, std::vector<Net>& nets);

// Parse chip information and cell dimensions from .dim file
void parseDIM(const std::string& path, ChipInfo& chip, std::vector<CellDim>& cellDims);

// Parse IO cell preferred coordinates from .io file, uses chip.numStdRows for row mapping
void parseIO(const std::string& path, std::vector<IOCell>& ioCells);

#endif // PARSER_H
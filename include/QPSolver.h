#ifndef QPSOLVER_H
#define QPSOLVER_H

#include <vector>
#include "PlacementProblem.h"

/**
 * Global placement QP solver.
 * @param prob    Problem data (nets, cellDims, ioCells, chip)
 * @param coords  Output: x or y coordinates for each cell
 * @param isXCoord true for solving x direction, false for y direction
 * @param alpha   Over-relaxation parameter
 * @param maxIter Maximum number of iterations
 * @param regions Optional regions parameter
 * @param u_target Target center-of-gravity vector for regions
 */
void solveGlobalPlacementQP(const PlacementProblem &prob,
                           std::vector<double> &coords,
                           bool isXCoord,
                           double alpha,
                           int maxIterations,
                           const std::vector<std::vector<int>>& regions = std::vector<std::vector<int>>(),
                           const std::vector<double>& u_target = std::vector<double>());

#endif // QPSOLVER_H
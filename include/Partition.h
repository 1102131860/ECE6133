#ifndef PARTITION_H
#define PARTITION_H

#include <vector>
#include "PlacementProblem.h"

// Enum for partition methods
enum class PartitionMethod {
    MEDIAN_CUT,
    KL
};

// Global variable for KL partition iterations
extern int klMaxIterations;

/**
 * Check if a region of cells can fit in one row
 * @param region_indices Global indices of cells in the region
 * @param cellDims Dimensions of all cells
 * @param coreWidth Width of the core area
 * @return true if the region fits in one row, false otherwise
 */
bool regionFitsOneRow(const std::vector<int>& region_indices,
                      const std::vector<CellDim>& cellDims,
                      double coreWidth);

/**
 * Partition regions recursively until they fit in rows or cannot be split further.
 * @param regions Input/Output: List of regions (each a list of global cell indices). Modified in place.
 * @param global_coords Global coordinates (e.g., x-coords) used for partitioning decisions.
 * @param problem Const reference to the full placement problem data (nets, cellDims, etc.).
 * @param partitionMethod The method to use for partitioning (KL or Median Cut).
 * @param maxDepth Maximum depth of partitioning (default -1 for no limit)
 */
void partitionRegions(std::vector<std::vector<int>>& regions,
                     const std::vector<double>& global_coords,
                     const PlacementProblem& problem,
                     PartitionMethod partitionMethod,
                     int maxDepth = -1);

/**
 * Calculate optimal chip dimensions and number of rows based on cell data.
 * @param cellDims Dimensions of all cells.
 * @param optimalWidth Output: calculated optimal width.
 * @param optimalHeight Output: calculated optimal height.
 * @param numRows Output: calculated number of rows.
 */
void calculateOptimalDimensions(const std::vector<CellDim>& cellDims,
                              double& optimalWidth,
                              double& optimalHeight,
                              int& numRows);

/**
 * Kernighan-Lin partitioning refinement.
 * Refines an initial partition within a given region.
 * @param region_indices Global indices of cells in the region to partition.
 * @param global_coords Global coordinates used for initial split and potentially gains.
 * @param problem Const reference to the full placement problem data.
 * @param partition_result Input/Output: Initial partition (boolean vector, size=region_indices.size()), refined partition on output.
 * @param maxPasses Maximum number of KL passes to perform.
 */
void klPartition(const std::vector<int>& region_indices,
                const std::vector<double>& global_coords,
                const PlacementProblem& problem,
                std::vector<bool>& partition_result,
                int maxPasses);

/**
 * Median cut partitioning.
 * Partitions cells within a region based on coordinate values.
 * @param region_indices Global indices of cells in the region to partition.
 * @param global_coords Global coordinates (e.g., x-coords) to partition by.
 * @param partition_result Output: Resulting partition (boolean vector, size=region_indices.size(), true=part A, false=part B).
 */
void medianCutPartition(const std::vector<int>& region_indices,
                       const std::vector<double>& global_coords,
                       std::vector<bool>& partition_result);

#endif // PARTITION_H
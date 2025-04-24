#include "QPSolver.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "osqp.h"

// Helper function: Convert triplets to CSC format
static void tripletsToCSC(OSQPInt nCols,
                         const std::vector<OSQPInt>& Ti,
                         const std::vector<OSQPInt>& Tj,
                         const std::vector<OSQPFloat>& Tx,
                         OSQPInt& nnz,
                         std::vector<OSQPInt>& P_p,
                         std::vector<OSQPInt>& P_i,
                         std::vector<OSQPFloat>& P_x) {
    // Filter upper triangular matrix elements
    std::vector<OSQPInt> filtered_Ti, filtered_Tj;
    std::vector<OSQPFloat> filtered_Tx;
    
    for (size_t idx = 0; idx < Tx.size(); ++idx) {
        if (Ti[idx] <= Tj[idx]) {
            filtered_Ti.push_back(Ti[idx]);
            filtered_Tj.push_back(Tj[idx]);
            filtered_Tx.push_back(Tx[idx]);
        }
    }
    
    nnz = (OSQPInt)filtered_Tx.size();
    P_p.assign(nCols + 1, 0);
    P_i.resize(nnz);
    P_x.resize(nnz);
    
    // Sort by column
    std::vector<size_t> indices(nnz);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), 
        [&](size_t a, size_t b) {
            if (filtered_Tj[a] != filtered_Tj[b]) return filtered_Tj[a] < filtered_Tj[b];
            return filtered_Ti[a] < filtered_Ti[b];
        });
    
    // Count non-zero elements per column
    for (OSQPInt idx = 0; idx < nnz; ++idx) {
        P_p[filtered_Tj[indices[idx]] + 1]++;
    }
    
    // Calculate cumulative sum
    for (OSQPInt col = 1; col <= nCols; ++col) {
        P_p[col] += P_p[col - 1];
    }
    
    // Fill row indices and values
    std::vector<OSQPInt> w = P_p;
    for (OSQPInt idx = 0; idx < nnz; ++idx) {
        OSQPInt col = filtered_Tj[indices[idx]];
        OSQPInt dest = w[col]++;
        P_i[dest] = filtered_Ti[indices[idx]];
        P_x[dest] = filtered_Tx[indices[idx]];
    }
}

// Main solver function
void solveGlobalPlacementQP(const PlacementProblem &prob,
                           std::vector<double> &coords,
                           bool isXCoord,
                           double alpha,
                           int maxIterations,
                           const std::vector<std::vector<int>>& regions,
                           const std::vector<double>& u_target) {
    
    int n = prob.cellDims.size();
    std::vector<bool> is_io_cell(n, false);
    
    // Mark and set IO cell coordinates
    for (const auto &io : prob.ioCells) {
        if (io.id - 1 < n) {
            is_io_cell[io.id - 1] = true;
            coords[io.id - 1] = isXCoord ? io.prefX : io.prefY;
        }
    }
    
    // Count non-IO cells
    int num_non_io = 0;
    for (int i = 0; i < n; i++) {
        if (!is_io_cell[i]) num_non_io++;
    }
    
    if (num_non_io == 0) {
        std::cout << "All cells are IO cells, no optimization needed" << std::endl;
        return;
    }
    
    // Build Laplacian matrix and linear term
    std::vector<OSQPInt> P_Ti, P_Tj;
    std::vector<OSQPFloat> P_Tx;
    std::vector<OSQPFloat> degrees(n, 0.0);
    std::vector<OSQPFloat> q(n, 0.0);
    
    for (const auto& net : prob.nets) {
        int sz = net.cellIDs.size();
        if (sz < 2) continue;
        
        double weight = 2 * alpha / (sz - 1); // alpha / sz;
        
        for (int i = 0; i < sz; ++i) {
            int node_i = net.cellIDs[i] - 1;
            if (node_i >= n || is_io_cell[node_i]) continue;
            
            degrees[node_i] += weight * (sz - 1);
            
            for (int j = i + 1; j < sz; ++j) {
                int node_j = net.cellIDs[j] - 1;
                if (node_j >= n) continue;
                
                bool is_io_j = is_io_cell[node_j];
                
                if (!is_io_j) {  // Both are non-IO cells
                    P_Ti.push_back(node_i);
                    P_Tj.push_back(node_j);
                    P_Tx.push_back(-weight);
                    
                    P_Ti.push_back(node_j);
                    P_Tj.push_back(node_i);
                    P_Tx.push_back(-weight);
                } else {  // node_j is an IO cell, add linear term to node_i
                    double pos = isXCoord ? prob.ioCells[node_j].prefX : prob.ioCells[node_j].prefY;
                    q[node_i] += -2.0 * weight * pos;
                }
            }
        }
    }
    
    // Add diagonal elements
    for (int i = 0; i < n; i++) {
        if (is_io_cell[i]) continue;
        P_Ti.push_back(i);
        P_Tj.push_back(i);
        P_Tx.push_back(degrees[i]);
    }
    
    // Build constraint matrix
    std::vector<OSQPInt> A_Ti, A_Tj;
    std::vector<OSQPFloat> A_Tx;
    std::vector<OSQPFloat> l, u;
    int current_row = 0;
    
    // Add IO cell fixed constraints
    for (int i = 0; i < n; ++i) {
        if (is_io_cell[i]) {
            A_Ti.push_back(current_row);
            A_Tj.push_back(i);
            A_Tx.push_back(1.0);
            l.push_back(coords[i]);
            u.push_back(coords[i]);
            current_row++;
        }
    }
    
    // Add non-IO cell boundary constraints
    double upper_bound = isXCoord ? prob.chip.coreW : prob.chip.coreH;
    for (int i = 0; i < n; ++i) {
        if (is_io_cell[i]) continue;
        
        // x_i >= 0
        A_Ti.push_back(current_row);
        A_Tj.push_back(i);
        A_Tx.push_back(1.0);
        l.push_back(0.0);
        u.push_back(OSQP_INFTY);
        current_row++;
        
        // x_i <= upper_bound
        A_Ti.push_back(current_row);
        A_Tj.push_back(i);
        A_Tx.push_back(1.0);
        l.push_back(-OSQP_INFTY);
        u.push_back(upper_bound);
        current_row++;
    }
    
    // Add region constraints
    for (int r = 0; r < (int)regions.size(); ++r) {
        const auto& region = regions[r];
        if (region.empty() || r >= (int)u_target.size()) continue;
        
        double region_size_inv = 1.0 / region.size();
        double target_center = u_target[r];
        
        for (int cellId : region) {
            if (cellId < 0 || cellId >= n) continue;
            A_Ti.push_back(current_row);
            A_Tj.push_back(cellId);
            A_Tx.push_back(region_size_inv);
        }
        
        l.push_back(target_center);
        u.push_back(target_center);
        current_row++;
    }
    
    // Convert to CSC format
    OSQPInt nnzP, nnzA;
    std::vector<OSQPInt> P_p, P_i;
    std::vector<OSQPFloat> P_x;
    tripletsToCSC(n, P_Ti, P_Tj, P_Tx, nnzP, P_p, P_i, P_x);
    
    std::vector<OSQPInt> A_p, A_i;
    std::vector<OSQPFloat> A_x;
    tripletsToCSC(n, A_Ti, A_Tj, A_Tx, nnzA, A_p, A_i, A_x);
    
    // Create OSQP data structures
    OSQPCscMatrix* Pmat = OSQPCscMatrix_new(n, n, nnzP, P_x.data(), P_i.data(), P_p.data());
    OSQPCscMatrix* Amat = OSQPCscMatrix_new(current_row, n, nnzA, A_x.data(), A_i.data(), A_p.data());
    
    // Set OSQP parameters
    OSQPSettings* settings = OSQPSettings_new();
    settings->max_iter = maxIterations;
    settings->eps_abs = 1e-3;
    settings->eps_rel = 1e-3;
    settings->verbose = false;
    
    // Run solver
    OSQPSolver* solver = nullptr;
    int status = osqp_setup(&solver, Pmat, q.data(), Amat, l.data(), u.data(), current_row, n, settings);
    
    if (status != 0) {
        std::cerr << "OSQP initialization failed: " << status << std::endl;
        goto cleanup;
    }
    
    osqp_solve(solver);
    
    // Process results
    if (solver->solution && (solver->info->status_val == OSQP_SOLVED || 
                           solver->info->status_val == OSQP_SOLVED_INACCURATE)) {
        for (int i = 0; i < n; ++i) {
            if (!is_io_cell[i]) {
                coords[i] = solver->solution->x[i];
            }
        }
    } else {
        // std::cerr << "Solving failed, using fallback solution" << std::endl;
        if (!u_target.empty()) {
            for (size_t r = 0; r < regions.size() && r < u_target.size(); ++r) {
                double target_pos = u_target[r];
                for (int cellId : regions[r]) {
                    if (cellId >= 0 && cellId < n) {
                        coords[cellId] = target_pos + (std::rand() % 100 - 50) / 500.0 * target_pos;
                    }
                }
            }
        }
    }
    
cleanup:
    osqp_cleanup(solver);
    OSQPCscMatrix_free(Pmat);
    OSQPCscMatrix_free(Amat);
    OSQPSettings_free(settings);
}

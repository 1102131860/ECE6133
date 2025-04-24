# Gordian Placement Algorithm

This project implements the Gordian placement algorithm for VLSI cell placement. The algorithm uses a combination of quadratic programming (QP) and recursive partitioning to efficiently place standard cells on a chip.

## Project Structure

`include/` & `src/`
  - `main.cpp` - Main implementation of the Gordian algorithm
  - `Parser.h/cpp` - Parses benchmark circuit files
  - `PlacementProblem.h/cpp` - Stores circuit data and placement information
  - `QPSolver.h/cpp` - Implements the quadratic programming solver using OSQP
  - `Partition.h/cpp` - Implements the partitioning algorithms (Median Cut, KL)

`visualize.py` - Visualizes the placement results

## Dependencies

- C++ compiler with C++17 support
- OSQP solver
- CMake build system
- Python 3 for visualization

## Building the Project

### Download OSQP

For submission, the `osqp` has been download, skip this step.

```bash
git clone https://github.com/osqp/osqp
```

### Using CMake

```bash
cmake -B build
cmake --build build
```

## Running the Program

```bash
./gpsim <benchmark_file> [KL|median]
```

Example:
```bash
./gpsim benchmarks/structP/structP KL
```

It will put outputs into `output` folder

## Visualizing the Results

### Install required library:

```bash
pip3 install -r requirements.txt --user
```

### Visualize:

```bash
python3 visualize.py <output_dir> [-o] [images_dir] [-i] [level intervals] [--no-connections] [--max-nets] [# of nets]
```

Example:

```bash
python3 visualize.py output/structP -o images/structP -i 5 --max-nets 200
```

# Gordian Placement EDA

## 0. Install Cmake (if already installed, skip it)

Check Cmake is installed or not

```bash
cmake --version
```

You should see the something like `cmake version 3.26.5`

If you fail to see it, please install Cmake first.

## 1. Configure OSQP library

Download the compressed library OSQP from Github (clone the repository)

```bash
git clone https://github.com/osqp/osqp
```

Create the `build` directory in the repository and generate cmake configurations inside it

```bash
cd osqp && cmake -B build
```

Compile OSQP source codes, and generate OSQP library inside `build` directory

```
cmake --build build
```

The generated static library `libosqpstatic` and dynamic library `libosqp` are in the `build/out`. The configuration file `osqp_configure.h` is in the `build/include/public`. (You don't need to change file path).

Now, the OSQP library is configured.

## 2. Build executable gpsim

Exit from `osqp` repository
 
Create a `build` directory and generate cmake configuration inside

```bash
cd .. && cmake -B build
```

Generate the executable file `gpsim`

```bash
cmake --build build
```

Now, the `gpsim` executable is built.

## 3. Run the program



cmake_minimum_required(VERSION 3.10)
project(ECE6133_project)

# Add DEBUG option
# cmake -B build -DDEBUG=ON to enable debug
option(DEBUG "Enable debug output" OFF)

# C++ 17 compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if OSQP library exists, build it with shell script if not found
set(OSQP_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/osqp/build/out/libosqpstatic.a")
if(NOT EXISTS ${OSQP_LIB_PATH})
    message(STATUS "OSQP library not found. Building OSQP...")
    
    # Create build directory for OSQP
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build
        RESULT_VARIABLE MKDIR_RESULT
    )
    
    if(NOT MKDIR_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create build directory for OSQP")
    endif()
    
    # Run CMake configuration for OSQP
    execute_process(
        COMMAND ${CMAKE_COMMAND} ..
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build
        RESULT_VARIABLE CMAKE_CONFIG_RESULT
    )
    
    if(NOT CMAKE_CONFIG_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to configure OSQP build")
    endif()
    
    # Build OSQP library
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build
        RESULT_VARIABLE CMAKE_BUILD_RESULT
    )
    
    if(NOT CMAKE_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build OSQP library")
    endif()
    
    message(STATUS "OSQP library built successfully")
endif()

# Add src code
add_executable(gpsim
    src/Parser.cpp
    src/PlacementProblem.cpp
    src/Partition.cpp
    src/QPSolver.cpp
    src/main.cpp
)

# Include the header files 
target_include_directories(gpsim PRIVATE
    # require our head files
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # require osqp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/osqp/include/public
    # generated osqp_configure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build/include/public
)

# Include library
target_link_directories(gpsim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build/out)

# Link libraries (osqpstatic and math library)
# Note: Math library 'm' is only needed on non-Windows platforms
if(WIN32)
    target_link_libraries(gpsim PRIVATE osqpstatic)
else()
    target_link_libraries(gpsim PRIVATE osqpstatic m)
endif()

# Add debug definition if DEBUG is ON
if(DEBUG)
    target_compile_definitions(gpsim PRIVATE DEBUG)
endif()

# Move executable after build (use rename instead of mv)
add_custom_command(TARGET gpsim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:gpsim> ${CMAKE_CURRENT_SOURCE_DIR}/gpsim
    COMMENT "Move executable gpsim to source directory."
)

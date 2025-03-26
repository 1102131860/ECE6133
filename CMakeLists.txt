cmake_minimum_required(VERSION 3.10)
project(ECE6133_project)

# C++ 11 compiler
set(CMAKE_CXX_STANDARD 11)

# Add src code
add_executable(gpsim
    src/main.cpp
)

# Include the header files 
target_include_directories(gpsim PRIVATE
    # required osqp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/osqp/include/public
    # generated osqp_configure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build/include/public
)

# Include library
target_link_directories(gpsim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/osqp/build/out)

# Link libraries (osqpstatic and math library)
target_link_libraries(gpsim PRIVATE osqpstatic m)

# Move executable after build (use rename instead of mv)
add_custom_command(TARGET gpsim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:gpsim> ${CMAKE_CURRENT_SOURCE_DIR}/gpsim
    COMMENT "Move executable gpsim to source directory."
)

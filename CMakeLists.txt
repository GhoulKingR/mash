cmake_minimum_required(VERSION 4.0)
project(mash)

add_compile_options(
    "$<$<CONFIG:Debug>:-O0;-g;-ggdb>" # Disable optimizations and add debug symbols for Debug builds
    "$<$<CONFIG:Debug>:-DDEBUG_MODE>" # Define a preprocessor macro for Debug builds
)

add_executable(${PROJECT_NAME} mash.cpp)

cmake_minimum_required(VERSION 3.7)
project(example_project)

add_executable(main WIN32 main.cpp dimensions.hpp levels.txt resources.rc)
target_compile_features(main PRIVATE cxx_std_17)

add_executable(level_generator level_generator.cpp dimensions.hpp)
target_compile_features(level_generator PRIVATE cxx_std_17)

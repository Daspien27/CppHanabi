cmake_minimum_required (VERSION 3.14)

project("CppHanabi")

add_subdirectory(deps)

add_executable(CppHanabi "main.cpp")

target_compile_features(CppHanabi PUBLIC cxx_std_20)
target_link_libraries(CppHanabi PUBLIC termcolor)
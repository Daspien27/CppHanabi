cmake_minimum_required (VERSION 3.14)

project("CppHanabi")

add_executable(CppHanabi "main.cpp")

target_compile_features(CppHanabi PUBLIC cxx_std_17)
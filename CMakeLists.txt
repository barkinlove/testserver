cmake_minimum_required(VERSION 3.14)

project(
  test
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src bin)

file(GENERATE OUTPUT file.txt CONTENT "0123456789abcdef")

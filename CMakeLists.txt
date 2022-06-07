cmake_minimum_required(VERSION 3.22)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 14)

add_executable(mandelbrot main.cpp vec2.h)
target_link_libraries(mandelbrot -lglut -lglfw -lGLEW -lGL -lGLU)
cmake_minimum_required(VERSION 3.16)
project(DuzyProjekt C)

set(CMAKE_C_STANDARD 11)

add_executable(DuzyProjekt poly.c poly.h calc.c stack.c stack.h make_poly.c make_poly.h make_command.c make_command.h poly_example.c)
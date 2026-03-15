cmake_minimum_required(VERSION 3.10)
project(EjercicioEvaluable1 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Las subcarpetas gestionan sus propios directorios de salida
add_subdirectory(Versión\ no\ distribuida)
add_subdirectory(Versión\ distribuida)

cmake_minimum_required(VERSION 3.15)

project(raytracer)

find_package(SDL2 REQUIRED)

add_executable(raytracer code/main.cc)

target_include_directories(raytracer PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(raytracer ${SDL2_LIBRARIES})

cmake_minimum_required(VERSION 2.8)
project(VisualizeFreeSurferSurface)

#Find Visualization Toolkits
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

AUX_SOURCE_DIRECTORY(. SRC_LIST)
add_executable(visualize ${SRC_LIST})
target_link_libraries(visualize ${VTK_LIBRARIES})

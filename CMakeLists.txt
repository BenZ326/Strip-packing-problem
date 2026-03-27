cmake_minimum_required(VERSION 3.16)

project(StripPackingProblem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STRIP_PACKING_BUILD_TESTS "Build GoogleTest-based unit tests" ON)

set(STRIP_PACKING_CORE_SOURCES
	src/BLEU.cpp
	src/datareader.cpp
	src/heuristic.cpp
	src/knapsack.cpp
	src/master_milp.cpp
	src/preprocessing.cpp
	src/skyline.cpp
	src/spp.cpp
)

add_library(strip_packing_core STATIC ${STRIP_PACKING_CORE_SOURCES})
target_include_directories(strip_packing_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(spp src/main.cpp)
target_link_libraries(spp PRIVATE strip_packing_core)

if(STRIP_PACKING_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()


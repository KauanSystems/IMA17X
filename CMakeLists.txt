cmake_minimum_required(VERSION 3.25)
project(InfernalMemoryAllocator VERSION 1.1 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(BENCHMARKS
    "IMA17X_vs_Malloc_Benchmark"          "src/IMA17X_vs_Malloc_Benchmark.cpp"
    "Malloc_Benchmark"                   "src/Malloc_Benchmark.cpp"
    "IMA17X_Benchmark"                   "src/Infernal_Memory_Allocator_Benchmark.cpp"
)

enable_testing()

while(BENCHMARKS)
    list(POP_FRONT BENCHMARKS TARGET_NAME SOURCE_FILE)

    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)

    target_include_directories(${TARGET_NAME} PRIVATE include .)

    if(NOT MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            -O3
            -march=native
            -mcx16
            -pthread
            -Wall
            -ftls-model=initial-exec
            -fomit-frame-pointer
        )
    endif()

    if(UNIX)
        target_link_libraries(${TARGET_NAME} PRIVATE atomic pthread)
    endif()

    add_test(NAME "Run_${TARGET_NAME}" COMMAND ${TARGET_NAME})
endwhile()

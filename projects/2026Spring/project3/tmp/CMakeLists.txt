cmake_minimum_required(VERSION 3.16)
project(project3_matmul C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(USE_OPENBLAS "Enable OpenBLAS comparison backend" ON)

add_executable(project3
    main.c
    matrix.c
    matmul.c
    benchmark.c
)

target_compile_options(project3 PRIVATE -O3 -Wall -Wextra -Wpedantic)

include(CheckCCompilerFlag)
check_c_compiler_flag("-mavx2" HAS_AVX2)
check_c_compiler_flag("-mfma" HAS_FMA)
if(HAS_AVX2)
    target_compile_options(project3 PRIVATE -mavx2)
endif()
if(HAS_FMA)
    target_compile_options(project3 PRIVATE -mfma)
endif()

find_package(OpenMP)
if(OpenMP_C_FOUND)
    target_link_libraries(project3 PRIVATE OpenMP::OpenMP_C)
endif()

if(USE_OPENBLAS)
    find_package(BLAS)
    if(BLAS_FOUND)
        target_compile_definitions(project3 PRIVATE USE_OPENBLAS)
        target_link_libraries(project3 PRIVATE ${BLAS_LIBRARIES})
        if(BLAS_LINKER_FLAGS)
            target_link_options(project3 PRIVATE ${BLAS_LINKER_FLAGS})
        endif()
    else()
        message(WARNING "BLAS/OpenBLAS not found. Building without OpenBLAS backend.")
    endif()
endif()
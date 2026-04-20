cmake_minimum_required(VERSION 3.16)

project(sgemm_bench C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
    main.c
    interface.c
    time.c
    sgemm.c
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra)
endif()

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

find_package(BLAS REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${BLAS_LIBRARIES})
if(BLAS_LINKER_FLAGS)
    separate_arguments(_blas_link_flags NATIVE_COMMAND "${BLAS_LINKER_FLAGS}")
    target_link_options(${PROJECT_NAME} PRIVATE ${_blas_link_flags})
endif()

find_path(CBLAS_INCLUDE_DIR
    NAMES cblas.h
    HINTS
        $ENV{OpenBLAS_ROOT}
        /opt/local
        /opt/homebrew
        /usr/local
        /usr
    PATH_SUFFIXES
        include
        include/openblas
        opt/openblas/include
)

if(CBLAS_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CBLAS_INCLUDE_DIR})
endif()

set(RUN_SIZE "1000" CACHE STRING "Matrix size for run target")
add_custom_target(run
    COMMAND ${PROJECT_NAME} ${RUN_SIZE}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
)

message(STATUS "BLAS libraries: ${BLAS_LIBRARIES}")
if(CBLAS_INCLUDE_DIR)
    message(STATUS "CBLAS include dir: ${CBLAS_INCLUDE_DIR}")
else()
    message(STATUS "CBLAS include dir not explicitly found; relying on compiler defaults")
endif()

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
SET(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules )
message(${CMAKE_MODULE_PATH})
set(EXECUTABLE_NAME hash_collision)

find_package(OpenCL REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(DEFINITIONS "")
add_definitions(${DEFINITIONS})

set(CMAKE_CXX_FLAGS "")

set(INCLUDE_DIRS 
	"${PROJECT_SOURCE_DIR}/include"
        ${OpenCL_INCLUDE_DIR})

set(LIBS ${OpenCL_LIBRARIES})

set(SRC_FILES
        "src/main.c")
        
include_directories(${INCLUDE_DIRS})

add_executable(${EXECUTABLE_NAME} 
	${SRC_FILES} 
	src/main.c)

add_library("${EXECUTABLE_NAME}_core" 
	${SRC_FILES})

target_link_libraries("${EXECUTABLE_NAME}_core" 
	${LIBS})

target_link_libraries(${EXECUTABLE_NAME} 
	${LIBS})

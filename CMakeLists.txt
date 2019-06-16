cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(ilang-repl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(deps/ilang-base)
add_subdirectory(deps/replxx)

set(
	ILANG_REPL_HEADERS
	include/ilang/Repl.hpp
)

set(
	ILANG_REPL_SOURCES
	src/main.cpp
)

add_executable(ilang-repl ${ILANG_REPL_SOURCES})

target_link_libraries(ilang-repl ilang-base fmt replxx)

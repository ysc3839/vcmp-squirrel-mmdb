cmake_minimum_required(VERSION 3.1)
project(vcmp-squirrel-mmdb C)

include(maxminddb.cmake)

add_library(${PROJECT_NAME} SHARED main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE maxminddb)
target_include_directories(${PROJECT_NAME} PRIVATE squirrel)

math(EXPR _CMAKE_BITS "${CMAKE_SIZEOF_VOID_P} * 8")

set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME_DEBUG   "${PROJECT_NAME}-dbg${_CMAKE_BITS}"
	OUTPUT_NAME_RELEASE "${PROJECT_NAME}-rel${_CMAKE_BITS}"
	PREFIX ""
)

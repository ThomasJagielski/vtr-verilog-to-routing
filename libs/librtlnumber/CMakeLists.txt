cmake_minimum_required(VERSION 3.16)

project("librtlnumber")

option(RTL_ALLOW_UNKNOWN_COMPARE "Skips initial check for unknowns in comparison and compares MSB-to-LSB" OFF)

if(RTL_ALLOW_UNKNOWN_COMPARE)
    add_definitions(-DRTL_ALLOW_UNKNOWN_COMPARE)
endif()

file(GLOB_RECURSE EXEC_SOURCES main.cpp)
file(GLOB_RECURSE LIB_SOURCES src/*.cpp)
file(GLOB_RECURSE LIB_HEADERS src/include/*.hpp)
files_to_dirs(LIB_HEADERS LIB_INCLUDE_DIRS)

#Create the library
add_library(librtlnumber STATIC
             ${LIB_HEADERS}
             ${LIB_SOURCES})
target_include_directories(librtlnumber PUBLIC ${LIB_INCLUDE_DIRS})
set_target_properties(librtlnumber PROPERTIES PREFIX "") #Avoid extra 'lib' prefix

#Create the test executable
target_link_libraries(librtlnumber)

#Create the executable
add_executable(rtl_number ${EXEC_SOURCES})


target_link_libraries(rtl_number
                        librtlnumber)

install(TARGETS rtl_number librtlnumber DESTINATION bin)
install(FILES ${LIB_HEADERS} DESTINATION include/librtlnumber)


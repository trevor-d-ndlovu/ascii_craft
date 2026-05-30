cmake_minimum_required(VERSION 3.16)
project(ascii_craft CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ascii_craft main.cpp)

target_include_directories(ascii_craft PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/AsciGraphics
)

if(WIN32)
    target_link_libraries(ascii_craft PRIVATE winmm)
endif()

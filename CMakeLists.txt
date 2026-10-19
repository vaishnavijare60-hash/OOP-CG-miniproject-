cmake_minimum_required(VERSION 3.10)
project(BankingSystem_OOP_CG VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(banking_system ${SOURCES})

if (WIN32)
    target_link_libraries(banking_system PRIVATE gdi32 user32)
endif()

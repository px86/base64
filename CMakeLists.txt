cmake_minimum_required(VERSION 3.18)

project(Base64)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)

include_directories(include/)

add_library(base64 src/base64.cpp)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE base64)

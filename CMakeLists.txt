cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(JsonSerializerTest LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Network)

add_executable(JsonSerializerTest "")

file(GLOB SRC "*.cpp")
file(GLOB HEADER "*.h" "*.hpp")

target_sources(JsonSerializerTest
    PRIVATE
    ${SRC}
    PUBLIC
    ${HEADER}
)

target_link_libraries(JsonSerializerTest
    PRIVATE
    Qt5::Core
)
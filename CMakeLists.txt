cmake_minimum_required(VERSION 3.14)
project(trashcli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(tr
    src/main.cpp
    src/trash.cpp
)

target_link_libraries(tr stdc++fs)

enable_testing()
add_executable(test_trash test/test_trash.cpp src/trash.cpp)
target_link_libraries(test_trash stdc++fs)
add_test(NAME test_trash COMMAND test_trash)

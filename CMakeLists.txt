cmake_minimum_required(VERSION 3.14)
project(trashcli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Bat toi uu cho ban phat hanh
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2 -Wall)
endif()

add_executable(tr
    src/main.cpp
    src/trash.cpp
)

# Luc rut (strip) neu build release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(tr PROPERTIES
        BUILD_RPATH ""
        INSTALL_RPATH ""
    )
endif()

target_link_libraries(tr stdc++fs)

# ==================================================
# Cau hinh cai dat (install)
# ==================================================
install(TARGETS tr RUNTIME DESTINATION bin)

install(FILES SPEC.MD README.md LICENSE
    DESTINATION share/trashcli
)

install(DIRECTORY docs/
    DESTINATION share/trashcli/docs
)

# ==================================================
# Kiem thu
# ==================================================
enable_testing()
add_executable(test_trash test/test_trash.cpp src/trash.cpp)
target_link_libraries(test_trash stdc++fs)
add_test(NAME test_trash COMMAND test_trash)
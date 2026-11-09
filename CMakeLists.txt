cmake_minimum_required(VERSION 3.18)

option(ENABLE_DEBUG "Enable Debug" OFF)

project(GenerationProcedurale)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Les textes de l'interface ont des accents : MSVC doit lire les fichiers en UTF-8.
if (MSVC)
    add_compile_options(/utf-8)
endif (MSVC)

add_subdirectory(core/)
add_subdirectory(api/)
add_subdirectory(src/)

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
        COMMENT "Copying resources to build directory"
)

add_executable(GenerationProcedurale main/main.cc)
add_dependencies(GenerationProcedurale copy_resources)
target_link_libraries(GenerationProcedurale PRIVATE src)

file(GLOB_RECURSE SRC_FILES src/*.cc)
file(GLOB_RECURSE HEADER_FILES include/*.h)

find_package(SFML COMPONENTS Graphics Window Audio System CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# imgui-SFML (https://github.com/SFML/imgui-sfml, v3.0, licence MIT) compile ICI,
# en bibliotheque statique, avec l'ImGui de vcpkg.
# Le port vcpkg "imgui-sfml" en fait une DLL qui embarque SA propre copie d'ImGui :
# on se retrouve avec 2 ImGui, et le notre n'a pas de contexte -> crash au demarrage.
add_library(imgui_sfml STATIC external/imgui-sfml/imgui-SFML.cpp)
target_include_directories(imgui_sfml PUBLIC external/imgui-sfml/)
target_link_libraries(imgui_sfml PUBLIC imgui::imgui SFML::Graphics SFML::Window SFML::System OpenGL::GL)

add_library(api ${SRC_FILES} ${HEADER_FILES})
target_include_directories(api PUBLIC include/)
target_link_libraries(api PUBLIC core SFML::Graphics SFML::Window SFML::Audio SFML::System imgui_sfml)

if (ENABLE_DEBUG)
    target_compile_definitions(api PUBLIC DEBUG_ENABLE=1)
endif (ENABLE_DEBUG)

file(GLOB_RECURSE SRC_FILES src/*.cc)
file(GLOB_RECURSE HEADER_FILES include/*.h)

add_library(src ${SRC_FILES} ${HEADER_FILES})
target_include_directories(src PUBLIC include/)
target_link_libraries(src PUBLIC api)

if (ENABLE_DEBUG)
    target_compile_definitions(src PUBLIC DEBUG_ENABLE=1)
endif (ENABLE_DEBUG)

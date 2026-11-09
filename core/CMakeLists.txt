file(GLOB_RECURSE SRC_FILES src/*.cc)
file(GLOB_RECURSE HEADER_FILES include/*.h)

add_library(core ${SRC_FILES} ${HEADER_FILES})
target_include_directories(core PUBLIC include/)

if (MSVC)
    target_compile_options(core PUBLIC /WX)
endif (MSVC)

if (ENABLE_DEBUG)
    target_compile_definitions(core PUBLIC DEBUG_ENABLE=1)
endif (ENABLE_DEBUG)

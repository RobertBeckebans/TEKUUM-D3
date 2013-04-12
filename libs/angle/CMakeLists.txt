
add_definitions(-DLIBEGL_EXPORTS)

file(GLOB_RECURSE libEGL_INCLUDES src/libEGL/*.h)
file(GLOB_RECURSE libEGL_SOURCES src/libEGL/*.cpp)

file(GLOB_RECURSE libGLESv2_INCLUDES src/libGLESv2/*.h)
file(GLOB_RECURSE libGLESv2_SOURCES src/libGLESv2/*.cpp)

file(GLOB_RECURSE common_INCLUDES src/common/*.h)
file(GLOB_RECURSE common_SOURCES src/common/*.cpp)

file(GLOB_RECURSE compiler_INCLUDES src/compiler/*.h)
file(GLOB_RECURSE compiler_SOURCES src/compiler/*.cpp)

list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/ossource_posix.cpp)
list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/CodeGenGLSL.cpp)
list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/OutputGLSL.cpp)

# source_group("libEGL" FILES ${libEGL_INCLUDES})
# source_group("libEGL" FILES ${libEGL_SOURCES})

# source_group("libGLESv2" FILES ${libGLESv2_INCLUDES})
# source_group("libGLESv2" FILES ${libGLESv2_SOURCES})

source_group("common" FILES ${common_INCLUDES})
source_group("common" FILES ${common_SOURCES})

source_group("compiler" FILES ${compiler_INCLUDES})
source_group("compiler" FILES ${compiler_SOURCES})

include_directories(include src)

add_library(libGLESv2 ${libGLESv2_SOURCES} ${libGLESv2_INCLUDES}
			${common_SOURCES} ${common_INCLUDES}
			${compiler_SOURCES} ${compiler_INCLUDES}
			)

add_library(libEGL ${libEGL_SOURCES} ${libEGL_INCLUDES})

add_dependencies(libEGL libGLESv2)


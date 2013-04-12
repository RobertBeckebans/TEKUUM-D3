
add_definitions(-DANGLE_STATIC)
add_definitions(-DANGLE_DISABLE_TRACE)
add_definitions(-DANGLE_DISABLE_PERF)

file(GLOB_RECURSE libEGL_INCLUDES src/libEGL/*.h)
file(GLOB_RECURSE libEGL_SOURCES src/libEGL/*.cpp)
#list(REMOVE_ITEM libEGL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/libEGL/main.cpp)

file(GLOB_RECURSE libGLESv2_INCLUDES src/libGLESv2/*.h)
file(GLOB_RECURSE libGLESv2_SOURCES src/libGLESv2/*.cpp)
#list(REMOVE_ITEM libGLESv2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/libGLESv2/main.cpp)

file(GLOB_RECURSE common_INCLUDES src/common/*.h)
file(GLOB_RECURSE common_SOURCES src/common/*.cpp)

file(GLOB_RECURSE compiler_INCLUDES src/compiler/*.h)
file(GLOB_RECURSE compiler_SOURCES src/compiler/*.cpp)

list(APPEND compiler_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/compiler/ArrayBoundsClamper.h)
list(APPEND compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/compiler/ArrayBoundsClamper.cpp)

list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/ossource_posix.cpp)
list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/CodeGenGLSL.cpp)
list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/OutputGLSL.cpp)
list(REMOVE_ITEM compiler_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/TranslatorGLSL.cpp)

source_group("libEGL" FILES ${libEGL_INCLUDES})
source_group("libEGL" FILES ${libEGL_SOURCES})

source_group("libGLESv2" FILES ${libGLESv2_INCLUDES})
source_group("libGLESv2" FILES ${libGLESv2_SOURCES})

source_group("common" FILES ${common_INCLUDES})
source_group("common" FILES ${common_SOURCES})

source_group("compiler" FILES ${compiler_INCLUDES})
source_group("compiler" FILES ${compiler_SOURCES})

include_directories(include src)

add_library(libANGLE STATIC ${libGLESv2_SOURCES} ${libGLESv2_INCLUDES}
			${common_SOURCES} ${common_INCLUDES}
			${compiler_SOURCES} ${compiler_INCLUDES}
			${libEGL_SOURCES} ${libEGL_INCLUDES}
			)
target_link_libraries(libANGLE ${DirectX_DXGUID_LIBRARY} ${DirectX_D3D9_LIBRARY})

# set(CMAKE_LINK_DEF_FILE_FLAG "/DEF:src/libGLESv2/libGLESv2.def")
# add_library(libGLESv2 SHARED ${libGLESv2_SOURCES} ${libGLESv2_INCLUDES}
			# ${common_SOURCES} ${common_INCLUDES}
			# ${compiler_SOURCES} ${compiler_INCLUDES}
			# )

# find_package(DirectX REQUIRED)

# set(CMAKE_LINK_DEF_FILE_FLAG "/DEF:src/libEGL/libEGL.def")
# add_library(libEGL SHARED ${libEGL_SOURCES} ${libEGL_INCLUDES})

# add_dependencies(libEGL libGLESv2)

# target_link_libraries(libEGL ${DirectX_DXGUID_LIBRARY} ${DirectX_D3D9_LIBRARY})


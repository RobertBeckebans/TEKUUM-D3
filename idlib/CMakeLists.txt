
add_definitions(-D__IDLIB__ -D__DOOM_DLL__)

file(GLOB_RECURSE ID_INCLUDES *.h)
file(GLOB_RECURSE ID_SOURCES *.cpp)

if(MSVC)
	list(REMOVE_ITEM ID_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/math/Simd_AltiVec.h)
	list(REMOVE_ITEM ID_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/math/Simd_AltiVec.cpp)
	list(REMOVE_ITEM ID_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/bv/Frustum_gcc.cpp)
elseif(NOT APPLE)
	list(REMOVE_ITEM ID_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/math/Simd_AltiVec.h)
	list(REMOVE_ITEM ID_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/math/Simd_AltiVec.cpp)
endif()

if(STANDALONE)
	add_definitions(-DSTANDALONE)
endif()

if(DEBUG_MEMORY)
	add_definitions(-DID_DEBUG_MEMORY)
	add_definitions(-DID_REDIRECT_NEWDELETE)
endif()
	
if(MSVC)
	
	#set_target_properties(idlib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
	
	set(ID_PRECOMPILED_SOURCES ${ID_SOURCES})
	#list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/geometry/RenderMatrix.cpp)
	#list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.cpp)
	
	foreach( src_file ${ID_PRECOMPILED_SOURCES} )
		#message(STATUS "/Yuprecompiled.h for ${src_file}")
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yuprecompiled.h"
            )
    endforeach()
	
	set_source_files_properties(precompiled.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycprecompiled.h"
        )
endif()
	
add_library(idlib ${ID_SOURCES} ${ID_INCLUDES})

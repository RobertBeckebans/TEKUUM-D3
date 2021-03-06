
add_definitions(-D__IDLIB__ -D__DOOM_DLL__)

file(GLOB ID__INCLUDES *.h)
file(GLOB ID__SOURCES *.cpp)
file(GLOB ID_BV_INCLUDES bv/*.h)
file(GLOB ID_BV_SOURCES bv/*.cpp)
file(GLOB ID_CONTAINERS_INCLUDES containers/*.h)
file(GLOB ID_CONTAINERS_SOURCES containers/*.cpp)
file(GLOB ID_GEOMETRY_INCLUDES geometry/*.h)
file(GLOB ID_GEOMETRY_SOURCES geometry/*.cpp)
file(GLOB ID_HASHING_INCLUDES hashing/*.h)
file(GLOB ID_HASHING_SOURCES hashing/*.cpp)
file(GLOB ID_MATH_INCLUDES math/*.h)
file(GLOB ID_MATH_SOURCES math/*.cpp)
file(GLOB ID_SYS_INCLUDES sys/*.h)
file(GLOB ID_SYS_SOURCES sys/*.cpp)

if(MSVC)
	file(GLOB ID_SYS_WIN32_INCLUDES sys/win32/*.h)
	file(GLOB ID_SYS_WIN32_SOURCES sys/win32/*.cpp)
else()
	file(GLOB ID_SYS_POSIX_INCLUDES sys/posix/*.h)
	file(GLOB ID_SYS_POSIX_SOURCES sys/posix/*.cpp)
endif()

set(ID_INCLUDES_ALL
			${ID__INCLUDES}
			${ID_BV_INCLUDES}
			${ID_CONTAINERS_INCLUDES}
			${ID_GEOMETRY_INCLUDES}
			${ID_HASHING_INCLUDES}
			${ID_MATH_INCLUDES}
			${ID_SYS_INCLUDES}
			)
			
set(ID_SOURCES_ALL
			${ID__SOURCES}
			${ID_BV_SOURCES}
			${ID_CONTAINERS_SOURCES}
			${ID_GEOMETRY_SOURCES}
			${ID_HASHING_SOURCES}
			${ID_MATH_SOURCES}
			${ID_SYS_SOURCES}
			)
			
if(MSVC)
	list(APPEND ID_INCLUDES_ALL ${ID_SYS_WIN32_INCLUDES})
	list(APPEND ID_SOURCES_ALL ${ID_SYS_WIN32_SOURCES})
else()
	list(APPEND ID_INCLUDES_ALL ${ID_SYS_POSIX_INCLUDES})
	list(APPEND ID_SOURCES_ALL ${ID_SYS_POSIX_SOURCES})
endif()

source_group("" FILES ${ID__INCLUDES})
source_group("" FILES ${ID__SOURCES})
source_group("bv" FILES ${ID_BV_INCLUDES})
source_group("bv" FILES ${ID_BV_SOURCES})
source_group("containers" FILES ${ID_CONTAINERS_INCLUDES})
source_group("containers" FILES ${ID_CONTAINERS_SOURCES})
source_group("geometry" FILES ${ID_GEOMETRY_INCLUDES})
source_group("geometry" FILES ${ID_GEOMETRY_SOURCES})
source_group("hashing" FILES ${ID_HASHING_INCLUDES})
source_group("hashing" FILES ${ID_HASHING_SOURCES})
source_group("math" FILES ${ID_MATH_INCLUDES})
source_group("math" FILES ${ID_MATH_SOURCES})
source_group("sys" FILES ${ID_SYS_INCLUDES})
source_group("sys" FILES ${ID_SYS_SOURCES})

if(MSVC)
	source_group("sys\\win32" FILES ${ID_SYS_WIN32_INCLUDES})
	source_group("sys\\win32" FILES ${ID_SYS_WIN32_SOURCES})
else()
	source_group("sys\\posix" FILES ${ID_SYS_POSIX_INCLUDES})
	source_group("sys\\posix" FILES ${ID_SYS_POSIX_SOURCES})
endif()

if(STANDALONE)
	add_definitions(-DSTANDALONE)
endif()

if(MESA)
	add_definitions(-DUSE_GLES3)
	add_definitions(-DUSE_MESA)
endif()

if(DEBUG_MEMORY)
	add_definitions(-DID_DEBUG_MEMORY)
	add_definitions(-DID_REDIRECT_NEWDELETE)
endif()

if(MSVC)
	
	#set_target_properties(idlib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
	
	set(ID_PRECOMPILED_SOURCES ${ID_SOURCES_ALL})
	list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/geometry/RenderMatrix.cpp)
	list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.cpp)
	
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

elseif(UNIX)

    include_directories(.)

endif()
	
add_library(idlib ${ID_SOURCES_ALL} ${ID_INCLUDES_ALL})

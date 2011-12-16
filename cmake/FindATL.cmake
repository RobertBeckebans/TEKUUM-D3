
if(WIN32)
  
	find_path(ATL_INCLUDE_DIR NAMES atlbase.h
		HINTS
		$ENV{WINDDK_DIR}
		C:/WinDDK/7600.16385.1
		PATH_SUFFIXES
		inc/atl71
		)
		
	if(CMAKE_CL_64)
		set(ATL_LIBPATH_SUFFIX "lib/atl/amd64")
	else()
		set(ATL_LIBPATH_SUFFIX "lib/atl/i386")
	endif()
	
	find_path(ATL_LIBRARY_DIR NAMES atl.lib
		HINTS
		$ENV{WINDDK_DIR}
		C:/WinDDK/7600.16385.1
		PATH_SUFFIXES
		${ATL_LIBPATH_SUFFIX}
		)

	# handle the QUIETLY and REQUIRED arguments and set ATL_FOUND to TRUE if
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(ATL 
		DEFAULT_MSG
		ATL_INCLUDE_DIR
		ATL_LIBRARY_DIR
		)

	mark_as_advanced(ATL_INCLUDE_DIR ATL_LIBRARY_DIR)
  
endif(WIN32)

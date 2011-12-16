
if(WIN32)
  
	find_path(MFC_INCLUDE_DIR NAMES afxwin.h
		HINTS
		$ENV{WINDDK_DIR}
		C:/WinDDK/7600.16385.1
		PATH_SUFFIXES
		inc/mfc42
		)
		
	if(CMAKE_CL_64)
		set(MFC_LIBPATH_SUFFIX "lib/mfc/amd64")
	else()
		set(MFC_LIBPATH_SUFFIX "lib/Mfc/i386")
	endif()
	
	# find_library(MFC_MFC42 NAMES mfc42
		# HINTS 
		# $ENV{WINDDK_DIR}
		# C:/WinDDK/7600.16385.1
		# PATH_SUFFIXES
		# ${DirectX_LIBPATH_SUFFIX})
		
	find_path(MFC_LIBRARY_DIR NAMES mfc42.lib
		HINTS
		$ENV{WINDDK_DIR}
		C:/WinDDK/7600.16385.1
		PATH_SUFFIXES
		${MFC_LIBPATH_SUFFIX}
		)

	# handle the QUIETLY and REQUIRED arguments and set MFC_FOUND to TRUE if
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(MFC 
		DEFAULT_MSG
		MFC_INCLUDE_DIR
		MFC_LIBRARY_DIR
		)

	mark_as_advanced(MFC_INCLUDE_DIR MFC_LIBRARY_DIR)
  
endif(WIN32)

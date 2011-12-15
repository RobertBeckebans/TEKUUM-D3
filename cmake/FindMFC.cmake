
if(WIN32)
  
	find_path(MFC_INCLUDE_DIR NAMES afxwin.h
		HINTS
		$ENV{PLATFORMSDK_DIR}
		C:/Program Files/Microsoft Platform SDK
		PATH_SUFFIXES
		include/mfc
		)

	# handle the QUIETLY and REQUIRED arguments and set MFC_FOUND to TRUE if
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(MFC 
		DEFAULT_MSG
		MFC_INCLUDE_DIR
		)

	mark_as_advanced(MFC_INCLUDE_DIR)
  
endif(WIN32)

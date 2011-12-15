
if(WIN32)
  
	find_path(ATL_INCLUDE_DIR NAMES atlbase.h
		HINTS
		$ENV{PLATFORMSDK_DIR}
		C:/Program Files/Microsoft Platform SDK
		PATH_SUFFIXES
		include/atl
		)

	# handle the QUIETLY and REQUIRED arguments and set ATL_FOUND to TRUE if
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(ATL 
		DEFAULT_MSG
		ATL_INCLUDE_DIR
		)

	mark_as_advanced(ATL_INCLUDE_DIR) # ATL_LIBRARIES)
  
endif(WIN32)

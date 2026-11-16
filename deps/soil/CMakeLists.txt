message(STATUS "Fetching SOIL...")

FetchContent_Declare(
	soil2
	GIT_REPOSITORY https://github.com/SpartanJ/SOIL2
	GIT_TAG master
	GIT_SHALLOW 1
	GIT_PROGRESS TRUE
)

# FetchContent_MakeAvailable(soil2)
FetchContent_GetProperties(soil2)
if(NOT soil2_POPULATED)
	FetchContent_Populate(soil2)
	# add_subdirectory(${soil2_SOURCE_DIR} ${soil2_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
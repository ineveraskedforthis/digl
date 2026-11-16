project(fetch_content)

message(STATUS "Fetching GLEW ...")

include(FetchContent)
FetchContent_Declare(
	glew
	GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
	GIT_TAG d06782b910213d675925e6e51a69ad0fd1fe1f23
	GIT_PROGRESS TRUE
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
	FetchContent_Populate(glew)
	add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(glewinfo ${glew_SOURCE_DIR}/src/glewinfo.c)
target_link_libraries(glewinfo PRIVATE libglew_static)

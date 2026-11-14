#GLEW <- DO NOT REMOVE; NEEDED FOR A TEST


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

# set_target_properties(libglew_static PROPERTIES COMPILE_FLAGS "/MTd /wd4100 /wd4189 /wd4065 /GR- /W3 /permissive- /GF /w34388 /w34389 /RTC1 /EHsc /Od /RTC1 -Wno-reserved-identifier -Wno-nonportable-system-include-path -Wno-unsafe-buffer-usage -Wno-strict-prototypes")

add_executable(glewinfo ${glew_SOURCE_DIR}/src/glewinfo.c)
target_link_libraries(glewinfo PRIVATE libglew_static)

cmake_minimum_required(VERSION 3.27)
project (DIGL_libraries LANGUAGES CXX C)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(deps)
add_library(
	DIGL_libraries
	"dummy.cpp"
	"alice/conversions.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/imgui.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
	"${PROJECT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.cpp"
)

set_property(TARGET DIGL_libraries PROPERTY CXX_STANDARD 20)


message(STATUS "GLEW SOURCE DIR " ${glew_SOURCE_DIR})

find_program(LEAN lean REQUIRED)

message(STATUS "LEAN TOOLCHAIN DIR " $ENV{LEAN_TOOLCHAIN_DIR})

target_include_directories(
	DIGL_libraries
	PRIVATE
	"${glew_SOURCE_DIR}/include"
	"${glfw_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/imgui"
	"${PROJECT_SOURCE_DIR}/include"
	"$ENV{LEAN_TOOLCHAIN_DIR}/include"
)

# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

target_link_libraries(DIGL_libraries PRIVATE libglew_static)
target_link_libraries(DIGL_libraries PRIVATE glfw)

# Lean libraries
target_link_libraries(DIGL_libraries PRIVATE
	$ENV{LEAN_TOOLCHAIN_DIR}/lib/lean/libleanshared.dll.a
	$ENV{LEAN_TOOLCHAIN_DIR}/lib/lean/libInit_shared.dll.a
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_custom_command(
# 	TARGET DIGL_libraries POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy
# 	${CMAKE_CURRENT_BINARY_DIR}/DIGL_libraries.dll
# 	${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
# )
# add_custom_command(
# 	TARGET DIGL_libraries POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy
# 	${CMAKE_CURRENT_BINARY_DIR}/DIGL_libraries.dll
# 	${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
# )
# add_custom_command(
# 	TARGET DIGL_libraries POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy
# 	${CMAKE_CURRENT_BINARY_DIR}/DIGL_libraries.dll
# 	${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
# )

install(
	TARGETS DIGL_libraries
	RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
	LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
	ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.lake/build/bin
)

# add_custom_command(
# 	TARGET DIGL_libraries POST_BUILD
# 	COMMAND copy ${CMAKE_CURRENT_SOURCE_DIR}
# 	VERBATIM
# )

# add_custom_target(GENERATE_LEAN_EXE DEPENDS ./lake/build/bin/digl.exe)
# add_dependencies(DIGL_libraries GENERATE_LEAN_EXE)
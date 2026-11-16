message(STATUS "Fetching GLFW...")

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(BUILD_SHARED_LIBS ON)
# set(GLFW_BUILD_WAYLAND OFF)
# set(GLFW_BUILD_WIN32 ON)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
    GIT_SHALLOW 1
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glfw)

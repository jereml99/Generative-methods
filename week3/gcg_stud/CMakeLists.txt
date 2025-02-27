cmake_minimum_required(VERSION 3.10)
project(GCG)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PROJECT_VERSION "")
set(CMAKE_PROJECT_VERSION_MAJOR "")
set(CMAKE_PROJECT_VERSION_MINOR "")
set(CMAKE_PROJECT_VERSION_PATCH "")
set(CMAKE_INSTALL_PREFIX "../")
set(CMAKE_BUILD_TYPE Release)
set(OpenGL_GL_PREFERENCE GLVND) 

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include_directories(./libGCG)
endif ()

find_package(OpenGL REQUIRED)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.2
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

add_compile_definitions(GL_SILENCE_DEPRECATION)

include_directories(./libGCG)
aux_source_directory(./libGCG/Mesh LIBGCG_SRC_LIST)
aux_source_directory(./libGCG/GLGraphics LIBGCG_SRC_LIST)
aux_source_directory(./libGCG/CGLA LIBGCG_SRC_LIST)
aux_source_directory(./libGCG/SOIL LIBGCG_SRC_LIST)
aux_source_directory(./libGCG/GL LIBGCG_SRC_LIST)

add_library(GCG STATIC ${LIBGCG_SRC_LIST})

include_directories(./exercises/Implicits ./exercises/PoolScene)
include_directories(${glfw_SOURCE_DIR}/include)
file(GLOB IMP_SRC ./exercises/Implicits/*.cpp)
add_executable(implicits ${IMP_SRC} )
target_link_libraries(implicits GCG OpenGL::GL glfw ${GLFW_LIBRARIES})

file(GLOB POOL_SRC ./exercises/PoolScene/*.cpp)
add_executable(pool_scene ${POOL_SRC}) 
target_link_libraries(pool_scene  GCG OpenGL::GL glfw ${GLFW_LIBRARIES})

install(TARGETS implicits pool_scene)

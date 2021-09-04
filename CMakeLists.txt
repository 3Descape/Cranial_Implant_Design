cmake_minimum_required(VERSION 3.21)

project(
    CranialImplant
    VERSION 1.0
    LANGUAGES CXX
)

cmake_policy(SET CMP0054 NEW)

include(CMakePrintHelpers)

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        VS_STARTUP_PROJECT ${PROJECT_NAME}
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>"
        COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB;$<$<CONFIG:DEBUG>:DEBUG_BUILD>;_USE_MATH_DEFINES;GLFW_INCLUDE_NONE"
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj )
endif()

if (WIN32)
  add_compile_definitions(NOMINMAX)
  set(USE_PKGCONFIG FALSE)
  set(SHARED_LIB TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Boost REQUIRED)
find_package(OpenVDB REQUIRED)
find_package(glm REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PCL REQUIRED)
find_package(tinyply REQUIRED)
find_package(NanoVDB REQUIRED)
find_package(TBB REQUIRED)
# find_package(OpenCL REQUIRED)

add_subdirectory(vendor/glad)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/nativefiledialog/src/include
    ${PCL_INCLUDE_DIRS}
    ${NANOVDB_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::boost
    OpenVDB::openvdb
    glad
    glfw
    OpenGL::GL
    glm::glm
    NRRD
    IMGUI
    nfd
    tinyply
    TBB::tbb
    ${PCL_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${PCL_LIBRARY_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${PCL_DEFINITIONS}
)

add_subdirectory(app)
add_subdirectory(src)

set(PROJECT_SOURCES)
get_target_property(PROJECT_SOURCES ${PROJECT_NAME} INTERFACE_SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES})


add_custom_target(copy-runtime-files ALL
    COMMAND ${CMAKE_COMMAND}
    -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>/shaders"

    # COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS
)

add_dependencies(${PROJECT_NAME} copy-runtime-files)
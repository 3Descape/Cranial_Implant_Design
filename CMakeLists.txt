cmake_minimum_required(VERSION 3.21)

project(
    CranialImplant
    VERSION 1.0
    LANGUAGES CXX
)

cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0074 NEW) # find_package() uses <PackageName>_ROOT variables.
set(CMAKE_POLICY_DEFAULT_CMP0144 NEW)
cmake_policy(SET CMP0144 NEW) # find_package() uses upper-case <PACKAGENAME>_ROOT variables.

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
endif()

set(CID_DEPENDENCIES_DIRECTORY "" CACHE PATH "The directory containing the libraries.")
set(OPENVDB_ROOT ${CID_DEPENDENCIES_DIRECTORY}/openvdb CACHE PATH "Path to openvdb directory.")
set(TBB_ROOT ${CID_DEPENDENCIES_DIRECTORY}/onetbb CACHE PATH "Path to tbb directory.")
set(BLOSC_ROOT ${CID_DEPENDENCIES_DIRECTORY}/blosc CACHE PATH "Path to blosc directory.")
set(ZLIB_ROOT ${CID_DEPENDENCIES_DIRECTORY}/zlib CACHE PATH "Path to zlib directory.")
set(GLM_ROOT ${CID_DEPENDENCIES_DIRECTORY}/glm CACHE PATH "Path to glm directory.")
set(GLAD_ROOT ${CID_DEPENDENCIES_DIRECTORY}/glad CACHE PATH "Path to glad directory.")
set(GLFW3_ROOT ${CID_DEPENDENCIES_DIRECTORY}/glfw CACHE PATH "Path to glfw directory.")
set(EIGEN_ROOT ${CID_DEPENDENCIES_DIRECTORY}/eigen CACHE PATH "Path to eigen directory.")
set(PCL_ROOT ${CID_DEPENDENCIES_DIRECTORY}/pcl CACHE PATH "Path to pcl directory.")
set(TINYPLY_ROOT ${CID_DEPENDENCIES_DIRECTORY}/tinyply CACHE PATH "Path to tinyply directory.")
set(NFD_ROOT ${CID_DEPENDENCIES_DIRECTORY}/nfd CACHE PATH "Path to ndf directory.")
set(LZ4_ROOT ${CID_DEPENDENCIES_DIRECTORY}/lz4 CACHE PATH "Path to lz4 directory.")
set(IMGUI_ROOT ${CID_DEPENDENCIES_DIRECTORY}/imgui CACHE PATH "Path to imgui directory.")
set(NRRD_ROOT ${CID_DEPENDENCIES_DIRECTORY}/nrrd CACHE PATH "Path to nrrd directory.")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${OPENVDB_ROOT}/lib/cmake/OpenVDB)

find_package(Boost REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenVDB REQUIRED)
find_package(GLM REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED)
find_package(TINYPLY REQUIRED)
find_package(GLAD REQUIRED)
find_package(NFD REQUIRED)
find_package(LZ4 REQUIRED)
find_package(imgui REQUIRED)
find_package(NRRD REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${PCL_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::boost
    OpenVDB::openvdb
    glad::glad
    glfw
    OpenGL::GL
    glm::glm
    nrrd::nrrd
    imgui::imgui
    nfd::nfd
    tinyply
    TBB::tbb
    ${PCL_LIBRARIES}
    Eigen3::Eigen
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${PCL_LIBRARY_DIRS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${PCL_DEFINITIONS}
)

add_subdirectory(src)

set(PROJECT_SOURCES)
get_target_property(PROJECT_SOURCES ${PROJECT_NAME} INTERFACE_SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCES})

set(CID_RUNTIME_DEPENTENCIES_DEBUG "")
set(CID_RUNTIME_DEPENTENCIES_RELEASE "")
list(APPEND CID_RUNTIME_DEPENTENCIES_DEBUG
    ${CID_DEPENDENCIES_DIRECTORY}/openvdb/bin/openvdb.dll
    ${CID_DEPENDENCIES_DIRECTORY}/blosc/bin/blosc.dll
    ${CID_DEPENDENCIES_DIRECTORY}/zlib/bin/zlib.dll
    ${CID_DEPENDENCIES_DIRECTORY}/onetbb/bin/tbb12.dll
    ${CID_DEPENDENCIES_DIRECTORY}/lz4/bin/lz4_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/onetbb/bin/tbb12_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/onetbb/bin/tbb12_debug.pdb
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_filters_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_search_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_octree_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_kdtree_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_common_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_features_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_registration_debug.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_sample_consensus_debug.dll
)
list(APPEND CID_RUNTIME_DEPENTENCIES_RELEASE
    ${CID_DEPENDENCIES_DIRECTORY}/blosc/bin/blosc.dll
    ${CID_DEPENDENCIES_DIRECTORY}/openvdb/bin/openvdb.dll
    ${CID_DEPENDENCIES_DIRECTORY}/lz4/bin/lz4.dll
    ${CID_DEPENDENCIES_DIRECTORY}/zlib/bin/zlib.dll
    ${CID_DEPENDENCIES_DIRECTORY}/onetbb/bin/tbb12.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_filters.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_search.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_octree.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_kdtree.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_common.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_features.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_registration.dll
    ${CID_DEPENDENCIES_DIRECTORY}/pcl/bin/pcl_sample_consensus.dll
)

add_custom_target(copy-runtime-files ALL
    COMMAND ${CMAKE_COMMAND}
    -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy "$<IF:$<CONFIG:Debug>,${CID_RUNTIME_DEPENTENCIES_DEBUG},${CID_RUNTIME_DEPENTENCIES_RELEASE}>" ${CMAKE_CURRENT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release> COMMAND_EXPAND_LISTS
)
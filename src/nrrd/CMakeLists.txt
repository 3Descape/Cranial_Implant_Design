cmake_minimum_required(VERSION 3.21)

project(
    NRRD
	VERSION 1.0
	LANGUAGES CXX
)

add_library(${PROJECT_NAME} INTERFACE)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

target_sources(${PROJECT_NAME} INTERFACE
    NRRD/nrrd_image_view.hxx
	NRRD/nrrd_image.hxx
	NRRD/nrrd_metaio.hxx
	NRRD/nrrd.hxx
    GetSet/BaseTypes.hxx
	GetSet/StringConversion.hxx
	GetSet/StringType.hxx
	GetSet/StringUtil.hxx
)

set(NRRD_SOURCES)
get_target_property(NRRD_SOURCES ${PROJECT_NAME} INTERFACE_SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${NRRD_SOURCES})
add_custom_target("${PROJECT_NAME}_INTERFACE" SOURCES ${NRRD_SOURCES})

find_package(Boost REQUIRED)

target_include_directories(${PROJECT_NAME} INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} INTERFACE
	Boost::boost
)
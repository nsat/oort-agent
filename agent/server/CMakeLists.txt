 cmake_minimum_required (VERSION 3.2)

project(api-server)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE MinsizeRel)
set(CMAKE_EXE_LINKER_FLAGS "-s" )
# set(CMAKE_BUILD_TYPE Debug)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../external)

add_definitions(-DUAVCAN_TOSTRING=0)
add_definitions(-DUAVCAN_NO_ASSERTIONS=1)
add_definitions(-DUAVCAN_CPP_VERSION=2014)
add_definitions(-DSPIRE_MODIFIED=1)

set(UAVCAN_DSDL_DEFINITIONS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan/dsdl/uavcan)
set(UAVCAN_CPP_VERSION 2014)

ExternalProject_Add(ONION
    PREFIX ../external
    GIT_REPOSITORY https://github.com/davidmoreno/onion
    GIT_TAG v0.8
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DONION_USE_SSL=false
        -DONION_USE_PAM=false
        -DONION_USE_PTHREADS=true
        -DONION_USE_PNG=false
        -DONION_USE_JPEG=false
        -DONION_USE_XML2=false
        -DONION_USE_SQLITE3=false
        -DONION_USE_REDIS=false
        -DONION_USE_GC=false
        -DONION_USE_TESTS=false
        -DONION_EXAMPLES=false
    PATCH_COMMAND patch -N -p1 -i ${CMAKE_SOURCE_DIR}/patch/onion.patch || true
)

ExternalProject_Add(NLOHMANN
    PREFIX ../external
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.7.3
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DJSON_BuildTests=off
)

set(PreviousBuildType ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan)
set(CMAKE_BUILD_TYPE ${PreviousBuildType})

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories(model)
include_directories(router)
include_directories(impl)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan/libuavcan/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan/libuavcan/include/dsdlc_generated
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan_linux/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan_linux/apps
)

# Invoke the DSDL compiler to generate headers for our custom data types.
add_custom_target(dsdlc ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuavcan/libuavcan/dsdl_compiler/libuavcan_dsdlc
                  ${CMAKE_CURRENT_SOURCE_DIR}/ussp -I${UAVCAN_DSDL_DEFINITIONS}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dsdlc_generated) # Adding the default output directory to the include paths

file(GLOB SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/router/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main-api-server.cpp
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/impl/AgentUAVCANServer.cpp PROPERTIES
    OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dsdlc_generated/ussp/payload/PayloadHealthCheck.hpp)

add_executable(${PROJECT_NAME} ${SRCS} )
add_dependencies(${PROJECT_NAME} ONION NLOHMANN)
add_dependencies(${PROJECT_NAME} dsdlc)
add_dependencies(${PROJECT_NAME} uavcan)
target_link_libraries(${PROJECT_NAME} onioncpp_static onion_static pthread z uavcan)

file(GLOB TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/router/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main-api-server-test.cpp
)

ExternalProject_Add(Catch2
    PREFIX ../external/catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.11.3
    CMAKE_ARGS
         -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} 
         -DBUILD_TESTING=off
         -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
)
find_library(Catch2 SHARED IMPORTED)

# find_package(Catch2 REQUIRED)
# find_package(Catch2 REQUIRED PATHS ../external/lib/cmake)
# include_directories(../external/include)
add_executable(test_agent ${TEST_SRCS})
ExternalProject_Get_Property(Catch2 source_dir)
target_include_directories(test_agent PUBLIC ${source_dir}/single_include  ${source_dir}/single_include/catch2)
add_dependencies(test_agent Catch2 ONION NLOHMANN)
add_dependencies(test_agent dsdlc)
add_dependencies(test_agent uavcan)
target_link_libraries(test_agent onioncpp onion pthread z uavcan)

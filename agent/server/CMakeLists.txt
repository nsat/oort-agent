 cmake_minimum_required (VERSION 3.2)

project(api-server)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE MinsizeRel)
set(CMAKE_EXE_LINKER_FLAGS "-s" )
# set(CMAKE_BUILD_TYPE Debug)

include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../external)

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

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories(model)
include_directories(router)
include_directories(impl)

file(GLOB SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/router/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/model/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main-api-server.cpp
)

add_executable(${PROJECT_NAME} ${SRCS} )
add_dependencies(${PROJECT_NAME} ONION NLOHMANN)
target_link_libraries(${PROJECT_NAME} onioncpp_static onion_static pthread z)

file(GLOB TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/router/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp
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
target_link_libraries(test_agent onioncpp onion pthread z)

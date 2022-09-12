cmake_minimum_required (VERSION 2.8.12)
project ("JutSuNewBot")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wno-narrowing")

find_package(Threads REQUIRED)

find_package(ZLIB REQUIRED)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(CURL 7.56.0)
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

set(Boost_USE_MULTITHREADED ON)
if (ENABLE_TESTS)
    find_package(Boost 1.59.0 COMPONENTS system unit_test_framework REQUIRED)
else()
    find_package(Boost 1.59.0 COMPONENTS system REQUIRED)
endif()
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR_RELEASE})
if(NOT Boost_USE_STATIC_LIBS)
	add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

include_directories(include)
link_directories(lib)
add_executable (JutSuNewBot  "src/main.cpp" "src/server.cpp" "src/iconvlite.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JutSuNewBot PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(JutSuNewBot libTgBot.a stdc++fs ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES})
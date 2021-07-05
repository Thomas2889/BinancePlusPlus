cmake_minimum_required (VERSION 3.18)


project ("Binance++" VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


if(NOT EXISTS "${CMAKE_BINARY_DIR}/#Headers")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/#Headers")
endif()

if(NOT EXISTS "#wd")
file(MAKE_DIRECTORY "#wd")
endif()


add_subdirectory ("Binance++")
add_subdirectory ("DevTest")

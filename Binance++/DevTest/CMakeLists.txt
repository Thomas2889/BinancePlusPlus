cmake_minimum_required (VERSION 3.18)

set(progName DevTest)

add_executable (${progName} "src/Main.cpp")

target_include_directories (${progName} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/#in/Include")
target_include_directories (${progName} PRIVATE "${CMAKE_SOURCE_DIR}/#in/Include")
target_include_directories (${progName} PRIVATE "${CMAKE_BINARY_DIR}/#Headers")

target_link_libraries(${progName}
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/CppLog.lib"
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/libcurl_imp.lib"
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/ta_abstract_cdr.lib"
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/ta_common_cdr.lib"
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/ta_func_cdr.lib"
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/ta_libc_cdr.lib"
	)


target_link_libraries (${progName} PRIVATE Binance++)


if(${CMAKE_OS_NAME} STREQUAL "Windows")
	add_custom_command(
		TARGET ${progName}
		POST_BUILD
		COMMAND call "${CMAKE_CURRENT_SOURCE_DIR}/#buildsteps/CopyBinaries.bat"
					"${CMAKE_CURRENT_SOURCE_DIR}/#in/Bin/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}"
					"${CMAKE_CURRENT_BINARY_DIR}"
					"dll"
		COMMENT "Adds dynamic libraries to the same folder as the exe"
		VERBATIM
	)
	add_custom_command(
		TARGET ${progName}
		POST_BUILD
		COMMAND call "${CMAKE_CURRENT_SOURCE_DIR}/#buildsteps/CopyBinaries.bat"
					"${CMAKE_SOURCE_DIR}/#in/Bin/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}"
					"${CMAKE_CURRENT_BINARY_DIR}"
					"dll"
		COMMENT "Adds dynamic libraries to the same folder as the exe"
		VERBATIM
	)
endif()

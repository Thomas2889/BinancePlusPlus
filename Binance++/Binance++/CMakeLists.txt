cmake_minimum_required (VERSION 3.18)

add_library (Binance++
	"src/Binance++.cpp"
)

target_include_directories (Binance++ PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/#in/Include")
target_include_directories (Binance++ PRIVATE "${CMAKE_SOURCE_DIR}/#in/Include")

target_link_libraries(Binance++
	PRIVATE "${CMAKE_SOURCE_DIR}/#in/Lib/${CMAKE_OS_NAME}/${CMAKE_PLATFORM_NAME}-${CMAKE_BUILD_TYPE}/CppLog.lib"
	)


if(${CMAKE_OS_NAME} STREQUAL "Windows")
	execute_process(COMMAND
		cmd /c call "${CMAKE_CURRENT_SOURCE_DIR}/#buildsteps/CopyHeaders.bat"
			"${CMAKE_CURRENT_SOURCE_DIR}/src"
			"${CMAKE_BINARY_DIR}/#Headers/Core"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()

# qmlplugin dump
#TODO: hack #2 - ustawianie PATH na sztywną wartość w skrypcie instalującym
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set(ENV{PATH} "${ENV_PATH}")
	
	execute_process(COMMAND
			${QT_QMAKE_EXECUTABLE}/../qmlplugindump
		-nonrelocatable
		${PROJECT_NAME}
		${PROJECT_VERSION}
		${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}
		OUTPUT_VARIABLE qmlplugindump_output
		RESULT_VARIABLE qmlplugindump_result
	)
	
	if(qmlplugindump_result)
		message(WARNING "qmlplugindump exited with code ${qmlplugindump_result}")
	else()
		file(WRITE ${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}/${PROJECT_NAME}/${PROJECT_NAME}.qmltypes "${qmlplugindump_output}")
		message("Qmltypes file generated.")
	endif()
endif()

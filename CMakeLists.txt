# cmake minumum version
cmake_minimum_required(VERSION 3.10)

# gnu directories
include(GNUInstallDirs)

# project name and version
project(IzSQLTableView VERSION 1.0 LANGUAGES CXX)

# project architecture
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(PLATFORM_TARGET "x64")
else()
	set(PLATFORM_TARGET "x86")
endif()

# Qt's modules
find_package(Qt5 COMPONENTS Core Qml Quick Concurrent Gui REQUIRED)

# IzSQLUtilities
find_package(IzSQLUtilities REQUIRED)
set(IzSQLUtilities_Binary ${IzSQLUtilities_DIR}/${CMAKE_INSTALL_BINDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET})

# IzSQLUtilities
find_package(IzModels REQUIRED)
set(IzModels_Binary ${IzModels_DIR}/${CMAKE_INSTALL_BINDIR}/${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET})

# zlib
set (
	ZLIB_ROOT
	${CMAKE_CURRENT_SOURCE_DIR}/../deploy/libraries/zlib
	CACHE
	PATH
	"zlib root path"
	FORCE
)

find_path(
	ZLIB_INCLUDE_DIR
	NAMES "zlib.h"
	PATHS "${ZLIB_ROOT}"
	PATH_SUFFIXES "include"
)

find_library(
	ZLIB_LIBRARY
	NAMES "zlibstatic.lib"
	PATHS "${ZLIB_ROOT}"
	PATH_SUFFIXES "lib"
)

# xlsxwriter
set (
	XLSXWRITER_ROOT
	${CMAKE_CURRENT_SOURCE_DIR}/../deploy/libraries/xlsxwriter
	CACHE
	PATH
	"libxlsxwriter root path"
	FORCE
)

find_path(
	XLSXWRITER_INCLUDE_DIR
	NAMES "xlsxwriter.h"
	PATHS "${XLSXWRITER_ROOT}"
	PATH_SUFFIXES "include"
)

find_library(
	XLSXWRITER_LIBRARY
	NAMES "xlsxwriter.lib"
	PATHS "${XLSXWRITER_ROOT}"
	PATH_SUFFIXES "lib/${PLATFORM_TARGET}/${CMAKE_BUILD_TYPE}"
)

# Qt's rcc files
qt5_add_resources(QRC_FILE
	"qml_src.qrc"
)

# if CMAKE_INSTALL_PREFIX is set to default value I override it
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	# prefix path for windows platform
	if (${CMAKE_SYSTEM_NAME} MATCHES "^Windows$")
		set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../deploy/plugins CACHE PATH "CMAKE_INSTALL_PREFIX path" FORCE)
	endif()
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Qt's moc files
qt5_wrap_cpp(MOC_files
	"include/IzSQLTableView/QmlPlugin.h"
	"include/IzSQLTableView/IzSQLTableViewPlugin_Enums.h"
	"private/SQLTableViewImpl.h"
	"private/TableHeaderModel.h"
	"private/ColumnsSettingsWrapper.h"
	"private/TableHeaderProxyModel.h"
)

# library definition
add_library(${PROJECT_NAME}Plugin SHARED
	"private/SQLTableViewImpl.cpp"
	"private/QmlPlugin.cpp"
	"private/TableHeaderModel.cpp"
	"private/ColumnsSettingsWrapper.cpp"
	"private/TableHeaderProxyModel.cpp"
	${MOC_files}
	${QRC_FILE}
)

set_target_properties(${PROJECT_NAME}Plugin PROPERTIES PREFIX "")

# xlsx setup
if(XLSXWRITER_LIBRARY AND ZLIB_LIBRARY)
	message("xlsxwriter and zlib found")

	target_compile_definitions(
		${PROJECT_NAME}Plugin
		PUBLIC
			LIBXLSXWRITER_ENABLED
	)

	target_include_directories(
		${PROJECT_NAME}Plugin
		PUBLIC
			${XLSXWRITER_INCLUDE_DIR}
			${ZLIB_INCLUDE_DIR}
	)

	target_link_libraries(
		${PROJECT_NAME}Plugin
		PUBLIC
			${XLSXWRITER_LIBRARY}
			${ZLIB_LIBRARY}
	)
else()
	message("could not find xlsxwriter and/or zlib")
endif()

# include directories for target
target_include_directories(${PROJECT_NAME}Plugin
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/private
)

# target compile features
target_compile_features(${PROJECT_NAME}Plugin
	PRIVATE
		cxx_std_17
)

# target links
target_link_libraries(${PROJECT_NAME}Plugin
	PUBLIC
		Qt5::Core
		Qt5::Gui
		Qt5::Qml
		Qt5::Quick
		Qt5::Concurrent
		IzSQLUtilities::IzSQLUtilities
)

# target compile definitions
target_compile_definitions(${PROJECT_NAME}Plugin
	PRIVATE
		QT_USE_QSTRINGBUILDER
)

# installs
install (TARGETS ${PROJECT_NAME}Plugin
	RUNTIME DESTINATION ${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}/${PROJECT_NAME}
)
install (FILES "private/qmldir"
	DESTINATION ${CMAKE_BUILD_TYPE}/${Qt5Core_VERSION}/${CMAKE_CXX_COMPILER_ID}/${PLATFORM_TARGET}/${PROJECT_NAME}
)

#TODO: hack #1 - przekazywanie zmiennych do skryptu instalacyjnego
install (CODE "set(PROJECT_NAME \"${PROJECT_NAME}\")")
install (CODE "set(PROJECT_VERSION \"${PROJECT_VERSION}\")")
install (CODE "set(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")")
install (CODE "set(Qt5Core_VERSION \"${Qt5Core_VERSION}\")")
install (CODE "set(CMAKE_CXX_COMPILER_ID \"${CMAKE_CXX_COMPILER_ID}\")")
install (CODE "set(PLATFORM_TARGET \"${PLATFORM_TARGET}\")")
install (CODE "set(QT_QMAKE_EXECUTABLE \"${QT_QMAKE_EXECUTABLE}\")")
install (CODE "set(ENV_PATH \"${IzSQLUtilities_Binary};${IzModels_Binary}\")")
install (SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/QmlPluginDump.cmake")

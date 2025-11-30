#PROJECT WAS CREATED WITH PYMAKE
#PYMAKE: https://github.com/EndMy5uffering/PyCmakeToolKit
#THIS FILE CAN BE CHANGED TO FIT YOUR PROJECT. THIS IS ONLY A TEMPLATE FOR A QUICK SETUP.
cmake_minimum_required (VERSION 3.5)
project(JsonParser VERSION 0.0.1 LANGUAGES CXX)

#find_package(PACKAGE_NAME REQUIRED)

#set(SOME_LIB_HEADERS "" CACHE PATH "Path to lib" )
#set(SOME_LIB_FILE "" CACHE FILEPATH "*.a or *.lib files" )

set(OTHER_HEADER_FILES
	# put all .h here.
)

set(OTHER_SOURCE_FILES
	# put all .c/.cpp here.
)

include("${CMAKE_SOURCE_DIR}/include/local.cmake")
include("${CMAKE_SOURCE_DIR}/src/local.cmake")

# FOR ADDING IN THE REFS OF ALL HEADER FILES OF LIBRARYS
include_directories(
	"${CMAKE_SOURCE_DIR}/include/"
	#${SOME_LIB_HEADERS}
)

set(EXECUTABLE_BUILD ON CACHE BOOL "Build an executable")
set(LIBRARY_STATIC_BUILD OFF CACHE BOOL "Build a static library")
set(LIBRARY_SHARED_BUILD OFF CACHE BOOL "Build a shared library")
set(LIBRARY_MODULE_BUILD OFF CACHE BOOL "Build a module library")

set(EXPORT_INCLUDES
	${HEADER_FILES_INCLUDE_FOLDER}
	${SOURCE_FILES_SRC_FOLDER}
	${OTHER_HEADER_FILES}
	${OTHER_SOURCE_FILES}
)

if(EXECUTABLE_BUILD)
message("Target is build as executable")
add_executable("${CMAKE_PROJECT_NAME}" ${EXPORT_INCLUDES})
endif(EXECUTABLE_BUILD)

if(LIBRARY_STATIC_BUILD)
message("Target is build as library [STATIC]")
add_library("${CMAKE_PROJECT_NAME}_STATIC" STATIC ${EXPORT_INCLUDES})
endif(LIBRARY_STATIC_BUILD)

if(LIBRARY_SHARED_BUILD)
message("Target is build as library [SHARED]")
add_library("${CMAKE_PROJECT_NAME}_SHARED" SHARED ${EXPORT_INCLUDES})
endif(LIBRARY_SHARED_BUILD)

if(LIBRARY_MODULE_BUILD)
message("Target is build as library [SHARED]")
add_library("${CMAKE_PROJECT_NAME}_MODULE" MODULE ${EXPORT_INCLUDES})
endif(LIBRARY_MODULE_BUILD)

# FOR LINKING TO LIBRARY FILES (*.a | *.lib)
target_link_libraries("${CMAKE_PROJECT_NAME}"
	#${SOME_LIB_FILE} 
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message("Building on Mac!")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message("Building on Linux!")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message("You're on Windows !")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

set(SOME_STUFF OFF CACHE BOOL "Some flag")

if(SOME_STUFF)
	add_definitions(
		-DCOMPILE_WITH_SOME_PREPROCESSOR_DIRECTIVE
	)
endif(SOME_STUFF)
set(LIB_INCS "")
set(LIB_LIBS "")
set(LIB_OBJS "")
set(LIB_PATH "")
set(LIB_STATIC c m z rt dl)

### add_pkg libzmq3 zmq
macro(add_pkg name libs)
    message(STATUS "add pkg ${name}")
    set(${name}_ROOT "/opt/${name}" CACHE PATH "${name}")
    
    if(NOT EXISTS  /opt/${name})
    	message(FATAL_ERROR "/opt/${name} not found")
    endif()
    
    list(APPEND LIB_INCS ${${name}_ROOT}/include)
    
    if(STATIC_BUILD)
        list(APPEND LIB_LIBS ${${name}_ROOT}/lib/lib${libs}.a)
    else(STATIC_BUILD)
        list(APPEND LIB_PATH ${${name}_ROOT}/lib)
        list(APPEND LIB_LIBS ${libs})
    endif(STATIC_BUILD)
endmacro(add_pkg name libs)

### add_objs base src/base/*.cpp
macro(add_pkg name expr)
file(GLOB ${name}_SRCS ${expr} ${ARGV})
SET(LIB_NAME SystemBase)
SET(LIB_SRC
	WSystem.cpp
	ServiceCreator.cpp
	FCGIServer.cpp
)

INCLUDE_DIRECTORIES(${LIB_INCS} ${CMAKE_CURRENT_SOURCE_DIR}) 
ADD_LIBRARY(${LIB_NAME} OBJECT ${LIB_SRC})


###  usage dump_flags(${CMAKE_CURRENT_LIST_DIR})
macro(dump_flags dir)
    get_property(includes DIRECTORY ${dir} PROPERTY INCLUDE_DIRECTORIES)
    MESSAGE("${dir}.includes: ${includes}")
    
    get_property(links DIRECTORY ${dir} PROPERTY LINK_DIRECTORIES)
    MESSAGE("${dir}.links: ${links}")
endmacro(dump_flags dir)

### usage configure_dir(${from} ${to})
macro(configure_dir srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_dir srcDir destDir)

##
#message(STATUS "cxx=${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
##

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-sign-compare -std=c++0x -Wall")
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# rpath
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

### clang
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	list(APPEND LIBLIBS pthread)
	set(NO_WARN "-Wno-unused-private-field -Wno-unused-const-variable -pthread")
endif()

### gcc
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_EXE_LINKER_FLAGS "-pthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")

	#if(CMAKE_CXX_COMPILER_VERSION LESS "4.7")
	#message(FATAL_ERROR "Insufficient gcc version")
	#endif()

	# gcc > 4.8 linux x64
	if((CMAKE_CXX_COMPILER_VERSION GREATER "4.7") AND (UNIX) AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
		SET(GCC_SANITIZE_THREAD "OFF" CACHE BOOL "use thread sanitize check")
		IF(GCC_SANITIZE_THREAD)
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -O1 -fPIE -pie")
		ENDIF(GCC_SANITIZE_THREAD)
		SET(GCC_SANITIZE_ADDRESS "OFF" CACHE BOOL "use address sanitize check")
		IF(GCC_SANITIZE_ADDRESS)
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
		ENDIF(GCC_SANITIZE_ADDRESS)
	endif()

	# GCC_BOUND_CHECK
	SET(GCC_BOUND_CHECK "OFF" CACHE BOOL "use stl bound check")
	IF(GCC_BOUND_CHECK)
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
	ENDIF(GCC_BOUND_CHECK)

	# STATIC_BUILD
	SET(STATIC_BUILD "ON" CACHE BOOL "link gcc static")
	IF(STATIC_BUILD)
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a )
		list(APPEND LIB_LIBS ${LIB_STATIC})

		if(CMAKE_CXX_COMPILER_VERSION GREATER "4.7")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
		endif()
	ELSE(STATIC_BUILD)
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.a  OUTPUT_VARIABLE GCC_LIB_PATH)
		execute_process(COMMAND dirname ${GCC_LIB_PATH} OUTPUT_VARIABLE GCC_LIB_PATH)
		string(STRIP ${GCC_LIB_PATH} GCC_LIB_PATH)
		list(APPEND LIBPATH ${GCC_LIB_PATH})
	ENDIF(STATIC_BUILD)
endif()

#################################
#CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#PROJECT(wrm-search CXX)

#set(APP_NAME "wrmsearch")
#set(APP_VERSION_MAJOR 2)
#set(APP_VERSION_MINOR 7)
#set(APP_VERSION_PATCH 0)
#string(TIMESTAMP APP_VERSION_BUILD  "%Y%m%d")
#set(APP_VERSION_STRING "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")
#set(APP_VERSION_ABTEST "${APP_NAME}-v${APP_VERSION_STRING}-B${APP_VERSION_BUILD}")

#set(SRC_ROOT    ${PROJECT_SOURCE_DIR}/src)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} cmake)
#include(Halo)
#################################

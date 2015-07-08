##################################################
### add_pkg libzmq3 zmq
macro(add_pkg name libs)
	set(ROOTS /opt/halo /opt /data0/opt/halo /data0)
	
	unset(root CACHE)  
    foreach(root ${ROOTS})
		if(EXISTS ${root}/${name})
			set(${name}_ROOT "${root}/${name}" CACHE PATH "${name}")
    		break()
   		endif()
    endforeach()
    unset(root CACHE)
    
    if(NOT EXISTS ${${name}_ROOT})
    	message(FATAL_ERROR "package ${name} not found")
    else()
    	message(STATUS "add pkg ${name} ${${name}_ROOT}")
	endif()
    
    list(APPEND LIB_INCS ${${name}_ROOT}/include)
    
    if(STATIC_BUILD)
        list(APPEND LIB_LIBS ${${name}_ROOT}/lib/lib${libs}.a)
    else(STATIC_BUILD)
        list(APPEND LIB_PATH ${${name}_ROOT}/lib)
        list(APPEND LIB_LIBS ${libs})
    endif(STATIC_BUILD)
    
    if(${ARGC} EQUAL 0)
    	return()
    endif()
    
    foreach(libs ${ARGN})
    	if(STATIC_BUILD)
    		list(APPEND LIB_LIBS ${${name}_ROOT}/lib/lib${libs}.a)
    	else()
    		list(APPEND LIB_LIBS ${libs})
    	endif()
    endforeach()
endmacro(add_pkg name libs)

##################################################
### add_cxx_flags [DEBUG|MINSIZE|RELEASE|RELWITH|GNU|Clang] ...
### add_cxx_flags(DEBUG RELWITH GNU "-pthread")
macro(add_cxx_flags)
    set(BUILD_TYPES DEBUG MINSIZE RELEASE RELWITH)
    set(TOOL_TYPES GNU Clang)
    set(build )
    set(tools )
    set(flags )

    # classify
    foreach(f ${ARGN})
    	list(FIND BUILD_TYPES ${f} is_build_type)
    	list(FIND TOOL_TYPES ${f} is_tool_type)
    	
    	if(NOT (${is_build_type} EQUAL -1))
    		list(APPEND build ${f})
    	elseif(NOT (${is_tool_type} EQUAL -1))
    		list(APPEND tools ${f})
    	else()
    		list(APPEND flags ${f})
    	endif()
    endforeach()

    # tool 
    list(FIND tools ${CMAKE_CXX_COMPILER_ID} is_tool_match)
    list(LENGTH tools tools_len)
    list(LENGTH build build_len)

    if(${tools_len} EQUAL 0 OR ${is_tool_match} GREATER -1)
    	if(${build_len} EQUAL 0)
    		message("list(APPEND CMAKE_CXX_FLAGS ${flags})")
    		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}")
    	else()
    		foreach(b ${build})
    			message("list(APPEND CMAKE_CXX_FLAGS_${b} ${flags})")
    			set(CMAKE_CXX_FLAGS_${b} "${CMAKE_CXX_FLAGS_${b}} ${flags}")
    		endforeach()
    	endif()
    endif()
endmacro()
##################################################
### add_flags

include(CheckCXXCompilerFlag)

macro(add_flags flags)
    unset(add_flags_support CACHE)
    check_cxx_compiler_flag(${flags} add_flags_support)
	if(${add_flags_support})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}")
	elseif(${ARGC} GREATER 0)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
	endif()
endmacro()

##################################################
### add_objs base src/base/*.cpp
macro(add_objs name expr)
    file(GLOB ${name}_SRCS ${expr} ${ARGN})
    add_library(${name} OBJECT ${${name}_SRCS})
    set(${name}_OBJS $<TARGET_OBJECTS:${name}>) 
endmacro()

##################################################
### add_incs
macro(add_incs)
	list(APPEND LIB_INCS ${ARGN})
endmacro()

##################################################
### add_libs
macro(add_libs)
	list(APPEND LIB_LIBS ${ARGN})
endmacro()

##################################################
### add_static_libs
macro(add_libs)
	list(APPEND LIB_LIBS ${ARGN})
endmacro()

macro(add_static_libs)
	list(APPEND LIB_STATIC ${ARGN})
endmacro()

# pthread
macro(add_pthread)
	check_cxx_compiler_flag(-pthread support_pthread)
	if(${support_pthread})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	else()
		set(LIB_LIBS "${LIB_LIBS} pthread")
	endif()
endmacro()
##################################################
###
macro(begin_build)
	include_directories(${LIB_INCS}) 
	link_directories(${LIB_PATH})
endmacro()

macro(end_build)
	foreach(exe ${ARGN})
		target_link_libraries(${exe} ${LIB_LIBS})
	endforeach()
endmacro()

macro(build_exe exe)
	begin_build(${exe})
	add_executable(${exe} ${ARGN})
	end_build(${exe})
endmacro()
##################################################
### dump_flags(${CMAKE_CURRENT_LIST_DIR})
macro(dump_flags dir)
    get_property(includes DIRECTORY ${dir} PROPERTY INCLUDE_DIRECTORIES)
    MESSAGE("${dir}.includes: ${includes}")
    
    get_property(links DIRECTORY ${dir} PROPERTY LINK_DIRECTORIES)
    MESSAGE("${dir}.links: ${links}")
endmacro(dump_flags dir)

##################################################
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


##################################################
##################################################
set(LIB_INCS "")
set(LIB_LIBS "")
set(LIB_OBJS "")
set(LIB_PATH "")
set(LIB_STATIC c m z rt dl)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-sign-compare -Wall")

# c++11
add_flags(-std=c++11 -std=c++0x)
add_flags(-Wno-unused-const-variable)
add_flags(-Wno-unused-but-set-variable)
add_flags(-Wno-unused-private-field)
add_flags(-Wno-deprecated-declarations)
add_flags(-Wno-unused-function)

add_cxx_flags(Clang -stdlib=libc++)

# rpath
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

### gcc
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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

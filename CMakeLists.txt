cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)


if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    endif()
endif()

message( STATUS "Using CMake ${CMAKE_VERSION}" )

if( "${Engine}" STREQUAL "Godot" )
	add_compile_definitions(GODOT)
elseif( "${Engine}" STREQUAL "Unity" )
	add_compile_definitions(UNITY)
elseif( "${Engine}" STREQUAL "Unreal" )
	add_compile_definitions(UNREAL)
else()
	add_compile_definitions(NATIVE)
endif()

# Require out-of-source builds
file( TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH )

if ( EXISTS "${LOC_PATH}" )
    message( FATAL_ERROR "You cannot build in the source directory. Please use a build subdirectory." )
endif()

# Add paths to modules
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" )

# Enable some option for compiler
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Ignore PCL errors in Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-gnu-anonymous-struct -Wno-nested-anon-types)
endif()

# Turn on link time optimization for everything
set( CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON )

# Output compile commands to compile_commands.json (for debugging CMake issues)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build universal lib on macOS
# Note that CMAKE_OSX_ARCHITECTURES must be set before project().
if ( APPLE )
    set( CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" )
endif()

# Main project information
if( "${Engine}" STREQUAL "Godot" )
    project( GDExtensionTemplate
        LANGUAGES
            CXX
        VERSION
            0.1.0
    )
elseif( "${Engine}" STREQUAL "Unity" )
    project( UnityExtensionTemplate
        LANGUAGES
            CXX
        VERSION
            0.1.0
    )
elseif( "${Engine}" STREQUAL "Unreal" )
    project( UnrealExtensionTemplate
        LANGUAGES
            CXX
        VERSION
            0.1.0
    )
elseif( "${Engine}" STREQUAL "Native" )
    project( NativeExtensionTemplate
        LANGUAGES
            CXX
        VERSION
            0.1.0
    )
endif()

# Create our library
add_library( ${PROJECT_NAME} SHARED )

# LIB_ARCH is the architecture being built. It is set to the build system's architecture.
# For macOS, we build a universal library (both arm64 and x86_64).
set( LIB_ARCH ${CMAKE_SYSTEM_PROCESSOR} )
if ( APPLE )
    set( LIB_ARCH "universal" )
endif()

# LIB_DIR is where the actual library ends up. This is used in both the build directory and the
# install directory and needs to be consistent with the paths in the gdextension file.
#   e.g. linux.release.x86_64 = "lib/Linux-x86_64/libGDExtensionTemplate.so"
set( LIB_DIR "lib/${CMAKE_SYSTEM_NAME}-${LIB_ARCH}" )

message( STATUS "Building ${PROJECT_NAME} for ${LIB_ARCH} on ${CMAKE_SYSTEM_NAME}")

# BUILD_OUTPUT_DIR is where we put the resulting library (in the build directory)
set( BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/" )

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN true
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
)

if( NOT DEFINED CMAKE_DEBUG_POSTFIX )
    set_target_properties( ${PROJECT_NAME}
        PROPERTIES
            DEBUG_POSTFIX "-d"
    )
endif()

# Warnings
# include( CompilerWarnings )

# Create and include version info file from git
include( GitVersionInfo )

add_subdirectory( src )

# Install library and extension file in ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}
set( INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/" )

message( STATUS "Install directory: ${INSTALL_DIR}")

if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()



# ccache
# Turns on ccache if found
include( ccache )

# Formatting
# Adds a custom target to format all the code at once
include( ClangFormat )

# godot-cpp
if( "${Engine}" STREQUAL "Godot" )
    add_subdirectory( templates )
	# From here: https://github.com/godotengine/godot-cpp
	if ( NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/godot-cpp/Makefile" )
		message(
			FATAL_ERROR
				"[${PROJECT_NAME}] The godot-cpp submodule was not downloaded. Please update submodules: git submodule update --init --recursive."
		)
	endif()

	set( GODOT_CPP_SYSTEM_HEADERS ON CACHE BOOL "" FORCE )

	add_subdirectory( godot-cpp )

	set_target_properties( godot-cpp
		PROPERTIES
			CXX_VISIBILITY_PRESET hidden    # visibility needs to be the same as the main library
	)

	target_link_libraries( ${PROJECT_NAME}
		PRIVATE
			godot-cpp
	)
endif()
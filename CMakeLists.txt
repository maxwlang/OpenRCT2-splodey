# CMAKE project for openrct2
cmake_minimum_required(VERSION 3.24)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
	message(FATAL_ERROR "GCC 12+ required. Older GCC are known to fail compilation")
endif()

# Note: Searching for CCache must be before project() so project() can use CCache too
# if it is available
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(OPENRCT2_USE_CCACHE "Use CCache to improve recompilation speed (optional)" ON)

if (OPENRCT2_USE_CCACHE)
    find_package(CCache)

    if (CCache_FOUND)
         # Use e.g. "ccache clang++" instead of "clang++"
         set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCache_EXECUTABLE}")
         set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    "${CCache_EXECUTABLE}")
    else()
        message("Usage of CCache was enabled, but CCache was not found, so CCache is not being enabled.")
    endif()
endif (OPENRCT2_USE_CCACHE)

if (APPLE)
    execute_process(COMMAND /usr/bin/uname -m OUTPUT_VARIABLE SYSTEM_MACOS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT DEFINED ARCH)
        set(ARCH "${SYSTEM_MACOS_ARCH}")
    endif ()

    set(CMAKE_OSX_ARCHITECTURES "${ARCH}" CACHE STRING "")

    if("${ARCH}" MATCHES "^x86_64")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "")
    elseif("${ARCH}" MATCHES "^arm64")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
    else()
        message("Unknown macOS architecture: ${ARCH}. Behavior may be unexpected.")
    endif()
endif ()

if (APPLE)
    set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_OSX_ARCHITECTURES}" CACHE STRING "")
endif ()

project(openrct2 CXX)

include(cmake/platform.cmake)
include(CMakeDependentOption)

# vcpkg includes its own copy of duktapeConfig.cmake; only include ours as needed.
if (NOT MSVC)
    include(FindPkgConfig)
    set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
endif ()
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(TITLE_SEQUENCE_VERSION "0.4.14")
set(TITLE_SEQUENCE_URL  "https://github.com/OpenRCT2/title-sequences/releases/download/v${TITLE_SEQUENCE_VERSION}/title-sequences.zip")
set(TITLE_SEQUENCE_SHA1 "6c04781b959b468e1f65ec2d2f21f5aaa5e5724d")

set(OBJECTS_VERSION "1.7.1")
set(OBJECTS_URL  "https://github.com/OpenRCT2/objects/releases/download/v${OBJECTS_VERSION}/objects.zip")
set(OBJECTS_SHA1 "6bfcb5f8dbd87b049071088baba029d8215f969c")

set(OPENSFX_VERSION "1.0.5")
set(OPENSFX_URL  "https://github.com/OpenRCT2/OpenSoundEffects/releases/download/v${OPENSFX_VERSION}/opensound.zip")
set(OPENSFX_SHA1 "b1b1f1b241d2cbff63a1889c4dc5a09bdf769bfb")

set(OPENMSX_VERSION "1.6")
set(OPENMSX_URL  "https://github.com/OpenRCT2/OpenMusic/releases/download/v${OPENMSX_VERSION}/openmusic.zip")
set(OPENMSX_SHA1 "ba170fa6d777b309c15420f4b6eb3fa25082a9d1")

set(REPLAYS_VERSION "0.0.87")
set(REPLAYS_URL  "https://github.com/OpenRCT2/replays/releases/download/v${REPLAYS_VERSION}/replays.zip")
set(REPLAYS_SHA1 "6061B53DE346BD853BB997E635AC7374B1A7D2F0")

option(FORCE32 "Force 32-bit build. It will add `-m32` to compiler flags.")
option(WITH_TESTS "Build tests")
option(PORTABLE "Create a portable build (-rpath=$ORIGIN)" OFF)
option(APPIMAGE "Create an appimage build (-rpath=$ORIGIN/../lib)" OFF)
option(DOWNLOAD_TITLE_SEQUENCES "Download title sequences during installation." ON)
option(DOWNLOAD_OBJECTS "Download objects during installation." ON)
option(DOWNLOAD_OPENSFX "Download OpenSoundEffects during installation." ON)
option(DOWNLOAD_OPENMSX "Download OpenMusic during installation." ON)
CMAKE_DEPENDENT_OPTION(DOWNLOAD_REPLAYS "Download replays during installation." ON
    "WITH_TESTS" OFF)
CMAKE_DEPENDENT_OPTION(MACOS_USE_DEPENDENCIES "Use OpenRCT2 dependencies instead of system libraries" ON
    "APPLE" OFF)
CMAKE_DEPENDENT_OPTION(MACOS_BUNDLE "Build macOS application bundle (OpenRCT2.app)" ON
     "MACOS_USE_DEPENDENCIES; NOT DISABLE_GUI" OFF)

# Options
option(STATIC "Create a static build.")
option(USE_MMAP "Use mmap to try loading rct2's data segment into memory.")

option(DISABLE_DISCORD_RPC "Disable Discord-RPC support." OFF)
# Currently unused, disable by default.
option(DISABLE_GOOGLE_BENCHMARK "Disable Google Benchmarks support." ON)
option(DISABLE_HTTP "Disable HTTP support.")
option(DISABLE_NETWORK "Disable multiplayer functionality. Mainly for testing.")
option(DISABLE_TTF "Disable support for TTF provided by freetype2.")
option(DISABLE_VERSION_CHECKER "Disable the check for newer versions.")
option(DISABLE_FLAC     "Disable FLAC support.")
option(DISABLE_VORBIS   "Disable OGG/VORBIS support.")
option(DISABLE_OPENGL   "Disable OpenGL support.")
option(ENABLE_SCRIPTING "Enable script / plugin support." ON)
option(ENABLE_ASAN "Enable the AddressSanitizer.")
option(ENABLE_UBSAN "Enable the UndefinedBehaviourSanitizer.")
option(ENABLE_HEADERS_CHECK "Check if include directives in header files are correct. Only works with clang" OFF)
option(DISABLE_GUI "Don't build GUI. (Headless only.)")


if (FORCE32)
    set(TARGET_M "-m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TARGET_M}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${TARGET_M}")
endif ()

if (PORTABLE OR WIN32)
    set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_BINDIR}")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif ()

if (APPIMAGE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif ()

if (MACOS_USE_DEPENDENCIES)
    message("Using OpenRCT2 dependencies instead of system libraries")
    # if we're building on macOS, then we need the dependencies
    include(cmake/download.cmake)

    set(MACOS_DYLIBS_VERSION "37")
    set(MACOS_DYLIBS_ZIPFILE "openrct2-libs-v${MACOS_DYLIBS_VERSION}-universal-macos-dylibs.zip")
    set(MACOS_DYLIBS_SHA1 "6d633a881fc243e960e29d9c371915a7cdeeb880")
    set(MACOS_DYLIBS_DIR "${ROOT_DIR}/lib/macos")
    set(MACOS_DYLIBS_URL "https://github.com/OpenRCT2/Dependencies/releases/download/v${MACOS_DYLIBS_VERSION}/${MACOS_DYLIBS_ZIPFILE}")

    download_openrct2_zip(
        ZIP_VERSION ${MACOS_DYLIBS_VERSION}
        DOWNLOAD_DIR ${MACOS_DYLIBS_DIR}
        ZIP_URL ${MACOS_DYLIBS_URL}
        SHA1 ${MACOS_DYLIBS_SHA1}
    )

    set(CMAKE_MACOSX_RPATH 1)
    list(APPEND CMAKE_PREFIX_PATH "${MACOS_DYLIBS_DIR}")

    # if we're making the OpenRCT2.app bundle, rpath will be handled by fixup_bundle
    # if we're building the CLI executable, we need to do it ourselves
    if (NOT MACOS_BUNDLE)
        message("Setting rpath for macOS dylibs")
        # the RPATH to be used when installing, but only if it's not a system directory
        list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
        if("${isSystemDir}" STREQUAL "-1")
            set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        endif("${isSystemDir}" STREQUAL "-1")

        # if the DESTDIR env var is defined, use it in the install RPATH
        if(DEFINED ENV{DESTDIR})
            get_filename_component(destdirRealPath "$ENV{DESTDIR}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
            set(CMAKE_INSTALL_RPATH "${destdirRealPath}${CMAKE_INSTALL_PREFIX}/lib")
        endif()
    endif ()
endif ()

# LIST of supported flags, use SET_CHECK_CXX_FLAGS() to apply to target.
# Use ADD_CHECK_CXX_COMPILER_FLAG() to add to list.
set(SUPPORTED_CHECK_CXX_COMPILER_FLAGS "")

include(cmake/ipo.cmake)
list(APPEND IPO_ENABLED_BUILDS RELEASE RELWITHDEBINFO MINSIZEREL)
ipo_enable("${IPO_ENABLED_BUILDS}")

# Describe current version in terms of closest tag
execute_process(
    COMMAND git describe HEAD
    COMMAND sed -E "s/-g.+$//"
    WORKING_DIRECTORY ${ROOT_DIR}
    OUTPUT_VARIABLE OPENRCT2_VERSION_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Define current git branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${ROOT_DIR}
    OUTPUT_VARIABLE OPENRCT2_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Define short commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${ROOT_DIR}
    OUTPUT_VARIABLE OPENRCT2_COMMIT_SHA1_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)


if (NOT DISABLE_DISCORD_RPC)
    if (UNIX AND NOT APPLE)
        find_package(DiscordRPC)
        if(${DISCORDRPC_FOUND})
            set(HAVE_DISCORD_RPC TRUE)
            add_definitions(-D__ENABLE_DISCORD__)
            include_directories(DISCORDRPC_PROCESS_INCLUDES)
        endif()
    elseif (MACOS_USE_DEPENDENCIES)
        find_package(discordrpc CONFIG REQUIRED)
        if(${DISCORDRPC_FOUND})
            add_definitions(-D__ENABLE_DISCORD__)
            set(HAVE_DISCORD_RPC TRUE)
            message("Building with libdiscord-rpc.dylib")
        endif()
    elseif(NOT HAVE_DISCORD_RPC AND EXISTS "${ROOT_DIR}/discord-rpc")
        # Don't build discord's examples, some of which are in C and do not honour
        # the flags we set for C++. Also we don't use the provided examples.
        set(BUILD_EXAMPLES OFF CACHE BOOL "Build example apps")
        add_subdirectory("${ROOT_DIR}/discord-rpc")
        add_definitions(-D__ENABLE_DISCORD__)
        include_directories("${ROOT_DIR}/discord-rpc/include")
        set(HAVE_DISCORD_RPC TRUE)
        message("Building with discord-rpc support")
    else()
        message("No discord-rpc detected, to enable clone discord-rpc to root directory: ${ROOT_DIR}")
    endif()
endif()

# ensure X86 and X86_64 are always initialized
# may be overwritten below
set(X86 0)
set(X86_64 0)
# Copied from https://github.com/opencv/opencv/blob/dcdd6af5a856826fe62c95322145731e702e54c5/cmake/OpenCVDetectCXXCompiler.cmake#L63-L70
if(MSVC64 OR MINGW64)
    set(X86_64 1)
elseif(MINGW OR (MSVC AND NOT CMAKE_CROSSCOMPILING))
    set(X86 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(X86_64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
    set(X86 1)
elseif(CMAKE_OSX_ARCHITECTURES MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(X86_64 1)
endif()

# Check if a flag exists and add it to the list of compiler (so, not linker) options
function (ADD_CHECK_CXX_COMPILER_FLAG _CXXFLAGS _CACHE_VAR _FLAG)
    CHECK_CXX_COMPILER_FLAG("${_FLAG}" "${_CACHE_VAR}")
    if (${_CACHE_VAR})
        set(SUPPORTED_CHECK_CXX_COMPILER_FLAGS ${SUPPORTED_CHECK_CXX_COMPILER_FLAGS} ${_FLAG} PARENT_SCOPE)
    else ()
        message(STATUS "Unsupported CXXFLAG: ${_FLAG}")
    endif ()
endfunction ()

# Add check flags to a compile TARGET
function (SET_CHECK_CXX_FLAGS _TARGET)
    target_compile_options("${_TARGET}" PRIVATE "${SUPPORTED_CHECK_CXX_COMPILER_FLAGS}")
endfunction()

# Check if a flag exists and add it to the compiler and the linker options
function (ADD_CHECK_CXX_FLAG _CXXFLAGS _CACHE_VAR _FLAG)
    CHECK_CXX_COMPILER_FLAG("${_FLAG}" "${_CACHE_VAR}")
    if (${_CACHE_VAR})
        set(${_CXXFLAGS} "${${_CXXFLAGS}} ${_FLAG}" PARENT_SCOPE)
    else ()
        message(STATUS "Unsupported CXXFLAG: ${_FLAG}")
    endif ()
endfunction ()

if (MSVC)
    # CMAKE does not have a built-in option for setting the CRT, so override the default flags.
    # NOTE: doing it this way avoids a linker warning about one directive overriding another
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /permissive- /Zc:externConstexpr /EHsc /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # C4244: 'conversion_type': conversion from 'type1' to 'type2', possible loss of data
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4068") # C4068: unknown pragma

    # Enable char8_t<->char conversion
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:char8_t-")

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
        add_definitions(-D__SSE4_1__)
        add_definitions(-D__AVX2__)
    endif ()
    add_definitions(-DNOMINMAX)
else ()
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_OVERRIDE -Wsuggest-override)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_DUPLICATED_COND -Wduplicated-cond)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_NON_VIRTUAL_DTOR -Wnon-virtual-dtor)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_MISSING_VARIABLE_DECLARATIONS -Wmissing-variable-declarations)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_DUPLICATED_BRANCHES -Wduplicated-branches)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_RESTRICT -Wrestrict)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_MISSING_FIELD_INITIALIZERS -Wmissing-field-initializers)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_UNREACHABLE_CODE_BREAK -Wunreachable-code-break)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_LOGICAL_OP -Wlogical-op)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_RANGE_LOOP_ANALYSIS -Wrange-loop-analysis)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_TAUTOLOGICAL_ZERO_COMPARE -Wtautological-unsigned-zero-compare)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_OLD_STYLE_CAST -Wold-style-cast)
    # unused-const-variable=1 only checks main compilation unit
    # unused-const-variable[=2] would check for unused values from headers as well
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_UNUSED_CONST_VARIABLE -Wunused-const-variable=1)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WNO_CLOBBERED -Wno-clobbered)
    # Disabled due to problems compiling OpenSSL on macOS.
    # ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_DOCUMENTATION -Wdocumentation)

    # Items below are not supported by ICC
    if (NOT MINGW)
        # Do not enable for MinGW, as its headers contain redundant declarations of builtin functions
        ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_REDUNDANT_DECLS -Wredundant-decls)

        # Currently used MinGW w/GCC 7.2 doesn't provide sane error messages for this
        ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_NULL_DEREFERENCE -Wnull-dereference)
    endif ()
    # These have no workarounds available when building with LTO
    if (NOT IPO_BUILD_ENABLED)
        ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_FINAL_TYPES -Wsuggest-final-types)
        ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_FINAL_METHODS -Wsuggest-final-methods)
    endif ()
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_IGNORED_QUALIFIERS -Wignored-qualifiers)

    # -Wstrict-overflow is only active when -fstrict-overflow is enabled, but -fstrict-overflow
    # is enabled on -O2, -O3, -Os. This should help catch bugs locally before they reach Travis
    # As of 2a435bf -Wstrict-overflow=1 passes, but higher values do not.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-overflow")
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_STRICT_OVERFLOW -Wstrict-overflow=1)

    # Compiler flags
    set(DEBUG_LEVEL 0 CACHE STRING "Select debug level for compilation. Use value in range 0–3.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-aliasing -Werror -Wundef -Wmissing-declarations -Winit-self -Wall -Wextra -Wshadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-missing-braces -Wno-comment -Wnonnull -Wno-unused-parameter -Wno-attributes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=${DEBUG_LEVEL}")

    # Enable char8_t<->char conversion
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-char8_t -Wno-deprecated-declarations")

    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=objc-method-access")
    endif()

    # On mingw all code is already PIC, this will avoid compiler error on redefining this option
    if (NOT MINGW)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif ()

    if (APPLE AND NOT USE_MMAP)
        set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
    else ()
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif ()
endif ()

if (CXX_WARN_SUGGEST_FINAL_TYPES)
    # Disable -Wsuggest-final-types via pragmas where due.
   add_definitions(-D__WARN_SUGGEST_FINAL_TYPES__)
endif ()

if (CXX_WARN_SUGGEST_FINAL_METHODS)
    # Disable -Wsuggest-final-methods via pragmas where due.
   add_definitions(-D__WARN_SUGGEST_FINAL_METHODS__)
endif ()

if (MINGW)
   add_definitions(-fstack-protector-strong)
endif ()

# Include sub-projects
include("${ROOT_DIR}/src/openrct2/CMakeLists.txt" NO_POLICY_SCOPE)
include("${ROOT_DIR}/src/openrct2-cli/CMakeLists.txt" NO_POLICY_SCOPE)
if(NOT DISABLE_GUI)
    include("${ROOT_DIR}/src/openrct2-ui/CMakeLists.txt" NO_POLICY_SCOPE)
endif()

# graphics files (g2.dat and font.dat) 
if (NOT CMAKE_CROSSCOMPILING)
    set(graphics_files "g2" "fonts" "tracks")
    
    foreach(graphics_file ${graphics_files})
        set(output_file "${graphics_file}.dat")
        set(json_file "${ROOT_DIR}/resources/${graphics_file}/sprites.json")
        file(GLOB_RECURSE dependent_files CONFIGURE_DEPENDS "${ROOT_DIR}/resources/${graphics_file}/*")
        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ./openrct2-cli sprite build "${CMAKE_BINARY_DIR}/${output_file}" "${json_file}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            DEPENDS ${dependent_files}
        )
        list(APPEND output_files "${output_file}")
    endforeach()

    add_custom_target(graphics DEPENDS OpenRCT2::openrct2-cli ${output_files})

    if(NOT DISABLE_GUI)
        add_dependencies(openrct2 graphics)
    endif()
    
else ()
    message("Skipping graphics generation in cross-compile")
endif ()

# Include tests
if (WITH_TESTS)
    enable_testing()
    add_subdirectory("test/tests")
endif ()

# macOS bundle "install" is handled in src/openrct2-ui/CMakeLists.txt
# This is because the openrct2 target is modified (and that is where that target is defined)
if (NOT MACOS_BUNDLE OR (MACOS_BUNDLE AND WITH_TESTS))
    # Install
    # Don't recurse, grab all *.txt and *.md files
    add_definitions(-DDOCDIR="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}")
    file(GLOB DOC_FILES "${ROOT_DIR}/distribution/*.txt")
    list(APPEND DOC_FILES "${ROOT_DIR}/contributors.md"
                        "${ROOT_DIR}/PRIVACY.md"
                        "${ROOT_DIR}/licence.txt"
                        "${ROOT_DIR}/distribution/scripting.md"
                        "${ROOT_DIR}/distribution/openrct2.d.ts")

    if (DOWNLOAD_TITLE_SEQUENCES)
        # Checks if this version of the title sequences are already installed, updates if necessary
        # if /data/sequence exists, it will overwrite this anyways (see `install(DIRECTORY "data/" ...)` below)
        # therefore, the existence of that directory will skip this download
        install(CODE "
            include(${ROOT_DIR}/cmake/download.cmake)
            download_openrct2_zip(
                ZIP_VERSION ${TITLE_SEQUENCE_VERSION}
                DOWNLOAD_DIR \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATADIR}/openrct2/sequence/
                SKIP_IF_EXISTS ${CMAKE_SOURCE_DIR}/data/sequence/
                ZIP_URL ${TITLE_SEQUENCE_URL}
                SHA1 ${TITLE_SEQUENCE_SHA1}
            )")
    endif ()
    if (DOWNLOAD_OBJECTS)
        # Checks if this version of the objects are already installed, updates if necessary
        # if /data/object exists, it will overwrite this anyways (see `install(DIRECTORY "data/" ...)` below)
        # therefore, the existence of that directory will skip this download
        install(CODE "
            include(${ROOT_DIR}/cmake/download.cmake)
            download_openrct2_zip(
                ZIP_VERSION ${OBJECTS_VERSION}
                DOWNLOAD_DIR \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATADIR}/openrct2/object/
                SKIP_IF_EXISTS ${CMAKE_SOURCE_DIR}/data/object/
                ZIP_URL ${OBJECTS_URL}
                SHA1 ${OBJECTS_SHA1}
            )")
    endif ()
    if (DOWNLOAD_OPENSFX)
        install(CODE "
            include(${ROOT_DIR}/cmake/download.cmake)
            download_openrct2_zip(
                ZIP_VERSION ${OPENSFX_VERSION}
                SKIP_IF_EXISTS ${CMAKE_SOURCE_DIR}/data/assetpack/openrct2.sound.parkap
                DOWNLOAD_DIR \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATADIR}/openrct2/
                ZIP_URL ${OPENSFX_URL}
                SHA1 ${OPENSFX_SHA1}
            )")
    endif ()
    if (DOWNLOAD_OPENMSX)
        install(CODE "
            include(${ROOT_DIR}/cmake/download.cmake)
            download_openrct2_zip(
                ZIP_VERSION ${OPENMSX_VERSION}
                SKIP_IF_EXISTS ${CMAKE_SOURCE_DIR}/data/assetpack/openrct2.music.alternative.parkap
                DOWNLOAD_DIR \$ENV{DESTDIR}${CMAKE_INSTALL_FULL_DATADIR}/openrct2/
                ZIP_URL ${OPENMSX_URL}
                SHA1 ${OPENMSX_SHA1}
            )")
    endif ()
    if (DOWNLOAD_REPLAYS)
        # Checks if this version of the replays are already installed, updates if necessary
        install(CODE "
            include(${ROOT_DIR}/cmake/download.cmake)
            download_openrct2_zip(
                ZIP_VERSION ${REPLAYS_VERSION}
                DOWNLOAD_DIR \${CMAKE_CURRENT_BINARY_DIR}/testdata/replays/
                ZIP_URL ${REPLAYS_URL}
                SHA1 ${REPLAYS_SHA1}
            )")
    endif ()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/g2.dat" DESTINATION "${CMAKE_INSTALL_DATADIR}/openrct2")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fonts.dat" DESTINATION "${CMAKE_INSTALL_DATADIR}/openrct2")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tracks.dat" DESTINATION "${CMAKE_INSTALL_DATADIR}/openrct2")
    install(DIRECTORY "data/" DESTINATION "${CMAKE_INSTALL_DATADIR}/openrct2")

    # even when building WITH_TESTS, none of the below install steps are required for OpenRCT2.app
    if (NOT MACOS_BUNDLE)
        install(TARGETS "libopenrct2" LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                                    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        if(NOT DISABLE_GUI)
            install(TARGETS "openrct2" RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        endif()
        install(TARGETS "openrct2-cli" OPTIONAL RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        install(FILES ${DOC_FILES} DESTINATION "${CMAKE_INSTALL_DOCDIR}")
        install(FILES "distribution/linux/openrct2.appdata.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
        if (NOT DISABLE_GUI)
            install(FILES "resources/logo/icon_x16.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x24.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/24x24/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x32.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x48.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x96.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/96x96/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x128.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_x256.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps" RENAME "openrct2.png")
            install(FILES "resources/logo/icon_flag.svg" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps" RENAME "openrct2.svg")
            install(FILES "distribution/linux/openrct2.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
            install(FILES "distribution/linux/openrct2-savegame.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
            install(FILES "distribution/linux/openrct2-scenario.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
            install(FILES "distribution/linux/openrct2-uri.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
        endif()
        install(FILES "distribution/linux/openrct2-mimeinfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages/" RENAME "openrct2.xml")
        install(DIRECTORY "distribution/man/" DESTINATION "${CMAKE_INSTALL_MANDIR}/man6" FILES_MATCHING PATTERN "*.6")

        if (MACOS_USE_DEPENDENCIES)
            # Note: dependencies may have the same names as system installed libraries
            # (via homebrew). A local CMAKE_INSTALL_PREFIX is recommended to avoid issues
            file(GLOB DYLIB_FILES "${MACOS_DYLIBS_DIR}/lib/*.dylib")
            install(FILES ${DYLIB_FILES} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        endif()
    endif()
endif()

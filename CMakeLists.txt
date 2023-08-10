cmake_minimum_required (VERSION 3.13 FATAL_ERROR)

set (CMAKE_POLICY_DEFAULT_CMP0074 NEW)
set (CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project ("DnsResolver" LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)


# sockpp library
set (SOCKPP_BUILD_STATIC ON)
set (SOCKPP_BUILD_SHARED OFF)

add_subdirectory ("lib/sockpp")


# Tins library
set (LIBTINS_BUILD_EXAMPLES OFF)
set (LIBTINS_BUILD_TESTS OFF)
set (LIBTINS_BUILD_SHARED OFF)
set (LIBTINS_ENABLE_PCAP OFF)
set (LIBTINS_ENABLE_CXX11 ON)
set (LIBTINS_ENABLE_DOT11 OFF)
set (LIBTINS_ENABLE_WPA2 OFF)
set (LIBTINS_ENABLE_TCPIP OFF)
set (LIBTINS_ENABLE_ACK_TRACKER OFF)
set (LIBTINS_ENABLE_TCP_STREAM_CUSTOM_DATA OFF)
set (LIBTINS_ENABLE_WPA2_CALLBACKS OFF)

add_subdirectory ("lib/libtins")
target_include_directories (tins INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/libtins/include>  
)


# magic_enum library
add_subdirectory ("lib/magic_enum")


# executables
add_subdirectory ("src")

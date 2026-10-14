cmake_minimum_required(VERSION 3.25)
project(HoriSpotConnect LANGUAGES C CXX)
if(NOT APPLE OR NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  message(FATAL_ERROR "This Preview build targets macOS Apple Silicon")
endif()
set(HOST macos CACHE STRING "Upstream dependency platform")
set(PLATFORM arm64 CACHE STRING "Upstream dependency architecture")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
option(HORI_SANITIZE "Instrument project source with address and UB sanitizers" OFF)
if(HORI_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
enable_testing()
add_subdirectory(../spotraop bridge)
add_subdirectory(../common/cspot/tests/audio audio)
add_executable(hori-discover tools/discover.cpp)
# macOS exposes DNSService APIs through libSystem's dnssd re-export.
target_compile_options(hori-discover PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_executable(hori-native-tests ../common/cspot/tests/audio/cdn-tests.cpp)
target_include_directories(hori-native-tests PRIVATE
  ../common/libcodecs/targets/include/vorbis
  ../common/libcodecs/targets/include/vorbisenc
  ../common/libcodecs/targets/include/ogg)
target_link_libraries(hori-native-tests PRIVATE cspot)
foreach(case queue_auto queue_lossless queue_320 queue_missing_format queue_restriction
             key_invalid cdn_read cdn_eof cdn_seek cdn_small cdn_tiny cdn_short_header
             cdn_oversized vorbis96 vorbis160 vorbis320)
  add_test(NAME ${case} COMMAND hori-native-tests ${case})
  set_tests_properties(${case} PROPERTIES TIMEOUT 20)
endforeach()
file(GLOB HORI_CROSS_SOURCES ../common/crosstools/src/*.c)
add_executable(hori-raop-probe tools/raop-probe.cpp ${HORI_CROSS_SOURCES})
target_include_directories(hori-raop-probe PRIVATE ../common/crosstools/src
  ../spotraop/libraop/targets/include ../spotraop/libopenssl/targets/macos/arm64/include
  ../common/libpupnp/targets/macos/arm64/include/upnp
  ../common/libpupnp/targets/macos/arm64/include/ixml)
target_compile_definitions(hori-raop-probe PRIVATE USE_SSL SSL_STATIC_LIB UPNP_STATIC_LIB _GNU_SOURCE)
target_link_libraries(hori-raop-probe PRIVATE hori_audio
  ${CMAKE_CURRENT_SOURCE_DIR}/../spotraop/libraop/targets/macos/arm64/libraop.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/libcodecs/targets/macos/arm64/libcodecs.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/libpupnp/targets/macos/arm64/libpupnp.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../spotraop/libopenssl/targets/macos/arm64/libopenssl.a)

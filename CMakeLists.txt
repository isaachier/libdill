cmake_minimum_required(VERSION 3.1)
project(libdill VERSION 1.6 LANGUAGES C)

include(CheckSymbolExists)
include(CheckFunctionExists)

set(sources
     bsock.c
     chan.c
     cr.c
     crlf.c
     ctx.c
     fd.c
     handle.c
     iol.c
     ipaddr.c
     ipc.c
     libdill.c
     msock.c
     now.c
     pfx.c
     pollset.c
     rbtree.c
     stack.c
     tcp.c
     utils.c
     dns/dns.c)
include_directories(${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/dns")
set_source_files_properties(dns/dns.c PROPERTIES COMPILE_FLAGS -std=c99)
add_library(dill ${sources})

find_package(Threads REQUIRED)
target_link_libraries(dill PUBLIC Threads::Threads)

# check and enable rt if available
list(APPEND CMAKE_REQUIRED_LIBRARIES rt)
check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
  target_link_libraries(dill PUBLIC rt)
endif()

# Installation (https://github.com/forexample/package-example)

# Layout. This works for all platforms:
#   * <prefix>/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Targets:
#   * <prefix>/lib/libdill.a
#   * header location after install: <prefix>/include/libdill.h
install(
    TARGETS dill
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
)

# Headers:
#   * libdill.h -> <prefix>/include/libdill.h
install(
    FILES libdill.h
    DESTINATION "${include_install_dir}"
)

# Config
#   * <prefix>/lib/cmake/libdill/libdillConfig.cmake
#   * <prefix>/lib/cmake/libdill/libdillConfigVersion.cmake
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

# Config
#   * <prefix>/lib/cmake/libdill/libdillTargets.cmake
install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

set(CMAKE_REQUIRED_LIBRARIES )

# check and enable stack guard and dns if available
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

set(CMAKE_REQUIRED_LIBRARIES )
set(CMAKE_REQUIRED_DEFINITIONS )

check_function_exists(mprotect HAVE_MPROTECT)
if(HAVE_MPROTECT)
  target_compile_definitions(dill PUBLIC HAVE_MPROTECT)
endif()

check_function_exists(posix_memalign HAVE_POSIX_MEMALIGN)
if(HAVE_POSIX_MEMALIGN)
  target_compile_definitions(dill PUBLIC HAVE_POSIX_MEMALIGN)
endif()

# tests
include(CTest)
if(BUILD_TESTING)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
    set(test_files
        tests/chan.c
        tests/choose.c
        tests/crlf.c
        tests/example.c
        tests/fd.c
        tests/go.c
        tests/handle.c
        tests/iol.c
        tests/ipaddr.c
        tests/ipc.c
        tests/overload.c
        tests/pfx.c
        tests/rbtree.c
        tests/signals.c
        tests/sleep.c
        tests/tcp.c
        tests/threads.c
        tests/threads2.c)
    foreach(test_file IN LISTS test_files)
      get_filename_component(test_name ${test_file} NAME_WE)
      add_executable(test_${test_name} ${test_file})
      set_target_properties(test_${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        OUTPUT_NAME ${test_name})
      target_link_libraries(test_${test_name} dill)
      add_test(test_${test_name} tests/${test_name})
    endforeach()
endif()

# perf
if(BUILD_PERF)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/perf)
    set(perf_files
        perf/chan.c
        perf/choose.c
        perf/ctxswitch.c
        perf/go.c
        perf/hdone.c
        perf/timer.c
        perf/whispers.c)
    foreach(perf_file IN LISTS perf_files)
      get_filename_component(perf_name ${perf_file} NAME_WE)
      add_executable(perf_${perf_name} ${perf_file})
      set_target_properties(perf_${perf_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/perf
        OUTPUT_NAME ${perf_name})
      target_link_libraries(perf_${perf_name} dill)
    endforeach()
endif()

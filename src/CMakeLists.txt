if(MIC)
    if(NOT ${CMAKE_C_COMPILER} MATCHES "icc.*$")
        message(FATAL_ERROR "Require Intel Compiler to build for Intel MIC.")
    endif()
    if("$ENV{MPSS_SYSROOTS}" STREQUAL "")
        message(FATAL_ERROR "MPSS not found!")
    endif()

    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR k1om)
    set(CMAKE_SYSTEM_VERSION 1)
    set(_CMAKE_TOOLCHAIN_PREFIX  x86_64-k1om-linux-)
    set(CMAKE_FIND_ROOT_PATH $ENV{MPSS_SYSROOTS}/k1om-mpss-linux)
    set(CMAKE_C_FLAGS "-mmic ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-mmic ${CMAKE_CXX_FLAGS}")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)
    set(CMAKE_EXECUTABLE_SUFFIX ".mic")
endif()

set(CMAKE_CXX_FLAGS "-Wall -O3 -std=c++11 ${CMAKE_CXX_FLAGS}")

if(PROFILING)
    add_definitions(-DPROFILING)
endif()
if(HINT)
    add_definitions(-DHINT)
endif()

set(ALIGNMENT 256)

if("${THREAD}" STREQUAL "")
    message(FATAL_ERROR "No thread given!")
elseif(${THREAD} STREQUAL "OpenMP")
    find_package(OpenMP)
    if(NOT OPENMP_FOUND)
        message(FATAL_ERROR "OpenMP not found!")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    add_subdirectory(threads/openmp)
elseif(${THREAD} STREQUAL "None")
    add_subdirectory(threads/none)
else()
    message(FATAL_ERROR "Unknown thread given!")
endif()

if("${BACKEND}" STREQUAL "")
    message(FATAL_ERROR "No backend given!")
elseif(${BACKEND} STREQUAL "MPI")
    find_package(MPI)
    if(NOT MPI_C_FOUND)
        message(FATAL_ERROR "MPI not found!")
    endif()
    #Get rid of MPICXX bindings
    add_definitions(-DMPICH_SKIP_MPICXX)
    add_definitions(-DOMPI_SKIP_MPICXX)
    include_directories(${MPI_C_INCLUDE_PATH})
    list(APPEND LIBRARIES ${MPI_C_LIBRARIES})
    add_subdirectory(backends/mpi) 
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/backends/mpi)
else()
    message(FATAL_ERROR "Unknown backend given!")
endif()

if("${CLUSTER}" STREQUAL "")
    message(FATAL_ERROR "No target cluster given!")
elseif(${CLUSTER} STREQUAL "QPACE2")
    set(PROVIDER_NULL ON)
    set(PROVIDER_SELF ON)
    set(PROVIDER_LOOP ON)
    set(PROVIDER_VERBS ON)
    set(PROVIDER_SCIF ON)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/clusters/qpaceb)
    add_subdirectory(clusters/qpace2)
elseif(${CLUSTER} STREQUAL "QPACEB")
    set(PROVIDER_NULL ON)
    set(PROVIDER_SELF ON)
    set(PROVIDER_LOOP ON)
    set(PROVIDER_VERBS ON)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/clusters/qpaceb)
    add_subdirectory(clusters/qpaceb)
elseif(${CLUSTER} STREQUAL "iDataCool")
    set(PROVIDER_NULL ON)
    set(PROVIDER_SELF ON)
    set(PROVIDER_LOOP ON)
    set(PROVIDER_CMA ON)
    set(PROVIDER_VERBS ON)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/clusters/idatacool)
    add_subdirectory(clusters/idatacool)
else()
    message(FATAL_ERROR "Unknown target cluster given!")
endif()

if(PROVIDER_NULL)
    add_subdirectory(providers/null)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_NULL>)
endif()
if(PROVIDER_SELF)
    add_subdirectory(providers/self)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_SELF>)
endif()
if(PROVIDER_LOOP)
    add_subdirectory(providers/loop)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_LOOP>)
endif()
if(PROVIDER_CMA)
    add_subdirectory(providers/cma)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_CMA>)
endif()
if(PROVIDER_VERBS)
    find_library(IBVERBS NAMES ibverbs PATHS ENV LIBRARY_PATH)
    if(NOT IBVERBS)
        message(FATAL_ERROR "Library ibverbs not found!")
    endif()
    add_subdirectory(providers/verbs)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_VERBS>)
    list(APPEND LIBRARIES ${IBVERBS})
endif()
if(PROVIDER_SCIF)
    find_library(SCIF NAMES scif PATHS ENV LIBRARY_PATH)
    if(NOT SCIF)
        message(FATAL_ERROR "Library scif not found!")
    endif()
    add_subdirectory(providers/scif)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_SCIF>)
    list(APPEND LIBRARIES ${SCIF})
endif()
if(PROVIDER_NUMA)
    find_library(NUMA NAMES numa PATHS ENV LIBRARY_PATH)
    if(NOT NUMA)
        message(FATAL_ERROR "Library NUMA not found!")
    endif()
    add_subdirectory(providers/numa)
    list(APPEND PROVIDERS $<TARGET_OBJECTS:PROVIDER_NUMA>)
    list(APPEND LIBRARIES ${NUMA})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.hpp @ONLY) 
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.hpp DESTINATION include/pmr)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(FILES recvwindow.hpp sendwindow.hpp window.hpp DESTINATION include/pmr)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CAPI)
    if(NOT ${BACKEND} STREQUAL "MPI")
        message(FATAL_ERROR "C API only available for backend MPI!")
    endif()
    add_subdirectory(api/c)
    list(APPEND API_SRC $<TARGET_OBJECTS:API_C>)
    install(FILES api/c/pmr.h DESTINATION include/pmr)
endif()

add_subdirectory(misc)

add_library(pmr STATIC
    $<TARGET_OBJECTS:MISC>
    $<TARGET_OBJECTS:THREAD>
    $<TARGET_OBJECTS:BACKEND>
    $<TARGET_OBJECTS:CLUSTER>
    ${PROVIDERS}
    ${API_SRC})

target_link_libraries(pmr ${LIBRARIES})
target_include_directories(pmr PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

install(TARGETS pmr
    EXPORT pmr-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
install(EXPORT pmr-targets FILE pmr-config.cmake DESTINATION cmake)
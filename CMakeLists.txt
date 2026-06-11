cmake_minimum_required(VERSION 3.10)
project(jshook VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(DOBBY_USE_PREBUILT "Use prebuilt Dobby static library from deps/Dobby/lib" OFF)

if(DOBBY_USE_PREBUILT)
    message(STATUS "Using prebuilt Dobby from deps/Dobby")
    set(DOBBY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/include")
    set(DOBBY_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/lib/libdobby.a")
else()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby")
        message(STATUS "Fetching Dobby from source...")
        include(FetchContent)
        FetchContent_Declare(
            Dobby
            GIT_REPOSITORY https://github.com/jmpews/Dobby.git
            GIT_TAG master
            GIT_SHALLOW TRUE
        )
        set(DOBBY_GENERATE_SHARED On CACHE BOOL "" FORCE)
        set(DOBBY_GENERATE_OBJECT Off CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(Dobby)
        set(DOBBY_INCLUDE_DIR "${dobby_SOURCE_DIR}/include")
        set(DOBBY_LIBRARY "${dobby_BINARY_DIR}/source/libdobby.a")
    else()
        message(STATUS "Using local Dobby at deps/Dobby")
        set(DOBBY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/include")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/build/lib/libdobby.a")
            set(DOBBY_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/build/lib/libdobby.a")
        else()
            add_subdirectory(deps/Dobby)
            set(DOBBY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/include")
            set(DOBBY_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/build/libdobby.a")
        endif()
    endif()
endif()

if(NOT DOBBY_INCLUDE_DIR)
    set(DOBBY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/Dobby/include")
endif()

set(JSHOOK_SOURCES
    src/main.c
    src/xxtea_hook.c
    src/utils.c
)

add_library(jshook SHARED ${JSHOOK_SOURCES})

target_include_directories(jshook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${DOBBY_INCLUDE_DIR}
)

if(DOBBY_LIBRARY AND EXISTS "${DOBBY_LIBRARY}")
    target_link_libraries(jshook ${DOBBY_LIBRARY} dl)
endif()

target_link_libraries(jshook dl log)

if(ANDROID)
    set_target_properties(jshook PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

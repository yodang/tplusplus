cmake_minimum_required(VERSION 3.10)

project(t++)

set(CMAKE_CXX_STANDARD 17)

set(CXX_CLANG_TIDY) # to avoid appending ad nauseam
find_program(CXX_CLANG_TIDY NAMES clang-tidy)
if (CXX_CLANG_TIDY)
    list(APPEND CXX_CLANG_TIDY
        -format-style=file
    )
    # set(C_CLANG_TIDY ${CXX_CLANG_TIDY})

    if(NOT COMMAND disable_clangtidy)
      function(disable_clangtidy DIRECTORY)
        if(NOT DIRECTORY)
            message(FATAL_ERROR "Error: Cannot disable clangtidy without a path")
        endif()
        configure_file(${CMAKE_SOURCE_DIR}/CMake/toolchain/.clang-tidy.in ${DIRECTORY}/.clang-tidy)
      endfunction(disable_clangtidy)
    endif()
else()
    message(FATAL_ERROR "Cannot find clang-tidy executable : aborting checks")
endif()

set(CURL_LIBRARY "-lcurl")
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(test_bencode src/test_bencode.cpp)
target_include_directories(test_bencode PRIVATE include)

add_executable(test_sha src/test_sha.cpp)
target_include_directories(test_sha PRIVATE include)
target_link_libraries(test_sha OpenSSL::Crypto)

add_executable(test_torrent src/test_torrent.cpp)
target_include_directories(test_torrent PRIVATE include)
target_link_libraries(test_torrent OpenSSL::Crypto ${CURL_LIBRARIES})

add_executable(test_http src/test_http.cpp)
target_include_directories(test_http PRIVATE include)
target_link_libraries(test_http ${CURL_LIBRARIES})
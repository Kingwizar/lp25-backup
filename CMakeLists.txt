cmake_minimum_required(VERSION 3.24)
project(LP25 C)

set(CMAKE_C_STANDARD 99)

add_executable(LP25 main.c configuration.c configuration.h defines.h file-properties.c file-properties.h files-list.c files-list.h messages.c messages.h processes.c sync.c sync.h utility.c utility.h)

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries( LP25 ${OPENSSL_LIBRARIES})
endif()
cmake_minimum_required(VERSION 3.20)
project(ReverseSQ CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# 强制使用静态链接的 OpenSSL 库
set(OPENSSL_INCLUDE_DIR "D:\\OpenSSL-Win64\\include")
set(OPENSSL_CRYPTO_LIBRARY "D:\\OpenSSL-Win64\\lib\\VC\\x64\\MT\\libcrypto_static.lib")
set(OPENSSL_SSL_LIBRARY "D:\\OpenSSL-Win64\\lib\\VC\\x64\\MT\\libssl_static.lib")

aux_source_directory(include SOURCE_FILES)
include_directories(include ${OPENSSL_INCLUDE_DIR})


# 编译为动态链接库（DLL）
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# 链接 OpenSSL 和 eTPSS 库
target_link_libraries(${PROJECT_NAME}  ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} ws2_32 Crypt32)
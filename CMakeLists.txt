cmake_minimum_required(VERSION 3.10)
project(http_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# 头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/HttpServer/include
    ${PROJECT_SOURCE_DIR}/AIApps/ChatServer/include
    /usr/include/mysql-cppconn-8
    /usr/include/mysql
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    /usr/include/opencv4
    /usr/local/include
)

# 查找库
find_library(MYSQLCPPCONN_LIBRARY
    NAMES mysqlcppconn mysqlcppconn8
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS /usr/lib /usr/lib64 /usr/local/lib
)

# 收集源文件
file(GLOB_RECURSE HTTP_SERVER_SRC
    "${PROJECT_SOURCE_DIR}/HttpServer/src/*.cpp"
)

file(GLOB_RECURSE CHAT_SERVER_SRC
    "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/*.cpp"
)

# 主程序入口
set(MAIN_SRC "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/main.cpp")

# 从 ChatServer 源中移除 main.cpp (避免重复定义)
list(REMOVE_ITEM CHAT_SERVER_SRC "${MAIN_SRC}")

# 核心静态库 (供 http_server 和 test 共用)
add_library(http_server_lib STATIC
    ${HTTP_SERVER_SRC}
    ${CHAT_SERVER_SRC}
)

target_link_libraries(http_server_lib
    pthread
    muduo_net
    muduo_base
    mysqlcppconn
    mysqlclient
    ssl
    crypto
    ${CURL_LIBRARIES}
    ${OPENCV4_LIBRARIES}
    /usr/lib/x86_64-linux-gnu/libopencv_core.so
    /usr/lib/x86_64-linux-gnu/libopencv_imgproc.so
    /usr/lib/x86_64-linux-gnu/libopencv_highgui.so
    /usr/lib/x86_64-linux-gnu/libopencv_imgcodecs.so
    /usr/lib/x86_64-linux-gnu/libopencv_videoio.so
    /usr/lib/x86_64-linux-gnu/libopencv_dnn.so
    ${ONNXRUNTIME_LIBRARY}
    SimpleAmqpClient
    rabbitmq
)

link_directories(/usr/local/lib)

# 主可执行文件
add_executable(http_server ${MAIN_SRC})
target_link_libraries(http_server http_server_lib)

# 测试 (可选 — 仅在安装了 GTest 时编译)
find_package(GTest QUIET)
if(GTest_FOUND OR GTEST_FOUND)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "GoogleTest found — test targets enabled.")
else()
    message(STATUS "GoogleTest not found — skipping test targets.")
endif()

# 调试信息
message(STATUS "Include directories:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "  ${dir}")
endforeach()

message(STATUS "ONNX Runtime library: ${ONNXRUNTIME_LIBRARY}")
message(STATUS "Main source file: ${MAIN_SRC}")
message(STATUS "HTTP Server sources: ${HTTP_SERVER_SRC}")
message(STATUS "Chat Server sources (lib): ${CHAT_SERVER_SRC}")

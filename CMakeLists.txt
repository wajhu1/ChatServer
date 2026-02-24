# CMakeLists.txt
cmake_minimum_required(VERSION 3.0)
# 项目
project(chat)
# 编译参数
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# 可执行文件输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
# 源文件目录
add_subdirectory(src)


# aux_source_directory(. SRC_LIST)

# add_executable(server ${SRC_LIST})


# target_link_libraries(server muduo_net muduo_base pthread)

# 设置最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(muduo-core)

# 设置全局的C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 定义需要链接的库
set(LIBS pthread)

# 收集源文件并生成可执行文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
add_executable(muduo-core main.cpp ${SRC_LIST})

# 链接必要的库
target_link_libraries(muduo-core ${LIBS})

target_include_directories(muduo-core
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/lib/include
)
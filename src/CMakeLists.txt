cmake_minimum_required(VERSION 3.10)
project(stream_handle VERSION 0.1.0)


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "*.h")
list(FILTER SOURCES EXCLUDE REGEX ".*/main\\.cpp$")


#Создаем библиотеку из файлов проекта (т.к. у нас только .h то используем INTERFACE)
add_library(stream_logic INTERFACE)
target_include_directories(stream_logic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(stream_logic INTERFACE project_options)


#Создаем исполняемый файл
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE stream_logic)



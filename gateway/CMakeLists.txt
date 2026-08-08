cmake_minimum_required(VERSION 3.12...3.31)
project(gateway CXX)

find_package(userver REQUIRED COMPONENTS core)

userver_setup_environment()

include_directories(src)

add_library(${PROJECT_NAME}_objs OBJECT
    src/handlers/register_proxy.hpp
    src/handlers/register_proxy.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core)

# The service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Install
include(GNUInstallDirs)

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})

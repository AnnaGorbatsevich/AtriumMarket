cmake_minimum_required(VERSION 3.12...3.31)
project(stats_service CXX)

# Guarded: see gateway/CMakeLists.txt for why.
if(NOT TARGET userver::core)
    find_package(userver REQUIRED COMPONENTS core clickhouse kafka)
    userver_setup_environment()
endif()

include_directories(src)

add_library(${PROJECT_NAME}_objs OBJECT
    src/handlers/hello.hpp
    src/handlers/seller_stats.hpp
    src/handlers/seller_stats.cpp
    src/order_event_consumer.hpp
    src/order_event_consumer.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core userver::clickhouse userver::kafka)

# The service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Install
include(GNUInstallDirs)

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})

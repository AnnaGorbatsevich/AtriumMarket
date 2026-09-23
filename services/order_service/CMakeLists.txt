cmake_minimum_required(VERSION 3.12...3.31)
project(order_service CXX)

# Guarded: see gateway/CMakeLists.txt for why.
if(NOT TARGET userver::core)
    find_package(userver REQUIRED COMPONENTS core postgresql kafka)
    userver_setup_environment()
endif()

include_directories(src)

add_library(${PROJECT_NAME}_objs OBJECT
    src/db.hpp
    src/db.cpp
    src/events.hpp
    src/events.cpp
    src/handlers/add_basket.hpp
    src/handlers/add_basket.cpp
    src/handlers/get_basket.hpp
    src/handlers/get_basket.cpp
    src/handlers/get_orders.hpp
    src/handlers/get_orders.cpp
    src/handlers/update_basket.hpp
    src/handlers/update_basket.cpp
    src/handlers/checkout.hpp
    src/handlers/checkout.cpp
    src/handlers/update_order_status.hpp
    src/handlers/update_order_status.cpp
    src/handlers/reset_basket.hpp
    src/handlers/reset_basket.cpp
    src/order_status.hpp
    src/handlers/hello.hpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::core userver::postgresql userver::postgresql-dynamic-configs userver::kafka crypt)

# The service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

userver_testsuite_add_simple(
    PYTEST_ARGS --junitxml=${CMAKE_CURRENT_BINARY_DIR}/pytest-junit.xml
)

# Install
include(GNUInstallDirs)

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})

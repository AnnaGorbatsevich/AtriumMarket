cmake_minimum_required(VERSION 3.12...3.31)
project(AtriumMarket)

enable_testing()

find_package(userver REQUIRED COMPONENTS core postgresql)
userver_setup_environment()

add_subdirectory(gateway)
add_subdirectory(services/user_service)
add_subdirectory(services/listing_service)
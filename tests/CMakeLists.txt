cmake_minimum_required(VERSION 2.8)
project(ConfiguratorJson)
add_compile_options(-Wall -Werror)

set(CMAKE_CXX_STANDARD 11)

add_executable(TestConfig TestConfig.cpp ../Configurator/configurator.cpp)
add_executable(TestConfig2 TestConfig2.cpp ../Configurator/configurator.cpp)
add_executable(TestConfig3 TestConfig3.cpp ../Configurator/configurator.cpp)
add_executable(testOptional.cpp testOptional.cpp ../Configurator/configurator.cpp)


cmake_minimum_required(VERSION 2.8)
project(Configurator)
add_compile_options(-Wall -Werror)

set(CMAKE_CXX_STANDARD 11)

add_executable(TestConfig TestConfig.cpp ../Configurator/configurator.cpp)
add_executable(TestConfig2 TestConfig2.cpp ../Configurator/configurator.cpp)
add_executable(TestConfig3 TestConfig3.cpp ../Configurator/configurator.cpp)
add_executable(testOptional testOptional.cpp ../Configurator/configurator.cpp)

add_test("TestConfig" TestConfig)
add_test("TestConfig2" TestConfig2)
add_test("TestConfig3" TestConfig3)
add_test("testOptional" testOptional)

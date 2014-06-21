all: TestConfig testOptional

TestConfig: TestConfig.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	g++ TestConfig.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig

testOptional: testOptional.cpp ../Configurator/Optional.h
	g++ testOptional.cpp -std=c++0x -o testOptional

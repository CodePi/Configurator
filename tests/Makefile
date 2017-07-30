all: TestConfig TestConfig2 TestConfig3 testOptional

CC=g++

TestConfig: TestConfig.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	$(CC) TestConfig.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig

TestConfig2: TestConfig2.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	$(CC) TestConfig2.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig2

TestConfig3: TestConfig3.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	$(CC) TestConfig3.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig3

testOptional: testOptional.cpp ../Configurator/Optional.h
	$(CC) testOptional.cpp -std=c++0x -o testOptional

clean:
	rm TestConfig TestConfig2 TestConfig3 testOptional

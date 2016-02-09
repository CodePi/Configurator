all: TestConfig TestConfig2 TestConfig3 testOptional

TestConfig: TestConfig.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	g++ TestConfig.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig

TestConfig2: TestConfig2.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	g++ TestConfig2.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig2

TestConfig3: TestConfig3.cpp ../Configurator/configurator.cpp ../Configurator/configurator.h ../Configurator/Optional.h
	g++ TestConfig3.cpp ../Configurator/configurator.cpp -std=c++0x -o TestConfig3

testOptional: testOptional.cpp ../Configurator/Optional.h
	g++ testOptional.cpp -std=c++0x -o testOptional

clean:
	rm TestConfig TestConfig2 TestConfig3 testOptional

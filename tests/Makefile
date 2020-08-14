CC=g++
FLAGS=-std=c++0x
TARGETS := TestConfig TestConfig2 TestConfig3 testOptional

all : $(TARGETS)

% : %.cpp ../Configurator/configurator.h ../Configurator/Optional.h ../Configurator/configurator.cpp TestConfig.h
	$(CC) $< -o $@ $(FLAGS) ../Configurator/configurator.cpp

clean:
	rm -f $(TARGETS)

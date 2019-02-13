CC=g++
FLAGS=-std=c++0x
TARGETS := TestConfig TestConfig2 TestConfig3 testOptional

TARGETS := $(patsubst %,$(TAG)%,$(TARGETS))

all : $(TARGETS)

$(TAG)% : %.cpp ../Configurator/configurator.h ../Configurator/Optional.h ../Configurator/configurator.cpp
	$(CC) $< -o $@ $(FLAGS) ../Configurator/configurator.cpp

clean:
	rm -f $(TARGETS)

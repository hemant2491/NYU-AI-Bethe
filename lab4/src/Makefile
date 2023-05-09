CC=g++
CXXFLAGS = -g -std=c++17

CPPFLAGS +=

SOURCES := $(shell find . -name '*.cpp')

HEADERS := $(shell find . -name '*.h')

OUTPUT := learn

all: $(OUTPUT)

$(OUTPUT): $(SOURCES) $(HEADERS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(OUTPUT) $(SOURCES)

clean:
	$(RM) $(OUTPUT)

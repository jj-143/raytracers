SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS=$(SOURCES:.cpp=.d)
BINS=$(SOURCES:.cpp=)

CFLAGS+=-MMD
CXXFLAGS+=-MMD

TARGET=build/main

$(TARGET): $(OBJECTS)
	$(CXX) $(LXXFLAG) $(OBJECTS) -o $(TARGET)

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPS) $(TARGET)

-include $(DEPS)

CC = g++
CFLAGS = -std=c++17 -g

SOURCES = source.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = tagger

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

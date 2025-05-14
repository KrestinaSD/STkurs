CC=g++
CFLAGS=-c -g -Wall -Wextra
LDFLAGS= -lpthread -lcrypto++
SOURCES= main.cpp interface.cpp communicator.cpp userbase.cpp logger.cpp filelogger.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm $(OBJECTS) $(EXECUTABLE)

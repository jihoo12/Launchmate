CC = gcc
TARGET = launchmate
SOURCES = launchmate.c
LIBS = $(shell pkg-config --libs gtk4) ./libgtk4-layer-shell.so.1.0.3
CFLAGS = $(shell pkg-config --cflags gtk4)

$(TARGET): $(SOURCES)
	$(CC) -o $(TARGET) $(SOURCES) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)


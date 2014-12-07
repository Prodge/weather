CC=cc
CFLAGS= -std=c99 -pedantic

all: weather

weather: weather.o; \
    $(CC) weather.o -o weather

weather.o: weather.c; \
    $(CC) $(CFLAGS) weather.c

clean:; \
    rm -rf *.o weather

# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=cc
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS= -std=c99 -pedantic

all: weather

weather: weather.o
    $(CC) weather.o -o weather

weather.o: weather.c
    $(CC) $(CFLAGS) weather.c

clean:
    rm -rf *.o weather

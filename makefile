COMPILER = cc
FLAGS = -std=c99 -Wall -Werror -pedantic -c
EXE = weather

all: $(EXE)

weather: weather.o; \
     $(COMPILER) -o $(EXE) -L/usr/local/lib -lcurl -I/usr/local/include weather.o

weather.o: weather.c; \
      $(COMPILER) $(FLAGS) weather.c

clean:; \
    rm -rf *.o $(EXE)

run:; \
    ./$(EXE)

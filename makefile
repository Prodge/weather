COMPILER = cc
FLAGS = -std=c99 -Wall -Werror -pedantic -c
EXE = weather
BINDIR = /usr/local/bin
MANDIR = /usr/local/share/man/man1

all: $(EXE)

weather: weather.o; \
    $(COMPILER) -o $(EXE) -L/usr/local/lib -lcurl -I/usr/local/include weather.o

weather.o: weather.c; \
    $(COMPILER) $(FLAGS) weather.c

clean:; \
    rm -rf *.o $(EXE)

install: all; \
	install -D $(EXE) $(BINDIR)/$(EXE)

uninstall:; \
	rm -f $(BINDIR)/$(EXE)

installman:; \
	install -D docs/$(EXE).1 $(MANDIR)/$(EXE).1

uninstallman:; \
	rm -f $(MANDIR)/$(EXE).1

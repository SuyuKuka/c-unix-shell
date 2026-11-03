CC = gcc

SRC = src
BUILD = build
LIBS = -lreadline -lncurses

all: fsh

# Compilation de l'exécutable fsh
fsh:
	$(CC) -o fsh $(SRC)/*.c $(LIBS)

# Compiler et lancer le programme
c:
	$(CC) -o fsh $(SRC)/*.c $(LIBS)
	./fsh

# Compiler et lancer le programme avec le valgrind
valgrind:
	$(CC) -o fsh $(SRC)/*.c $(LIBS)
	valgrind ./fsh

# Nettoyage des fichiers générés
clean:
	rm -f fsh
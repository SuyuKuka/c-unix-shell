CC = gcc

SRC = src
BUILD = build
LIBS = -lreadline -lncurses

all: makedir fsh

# Création du répertoire build
makedir:
	mkdir -p $(BUILD)

# Compilation de l'exécutable fsh
fsh:
	$(CC) -o fsh $(SRC)/*.c $(LIBS)

# Lancer le test
test: 
	$(CC) -o fsh $(SRC)/*.c $(LIBS)
	./test.sh

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
	rm -f $(BUILD)/*.o fsh
	rm -d $(BUILD)
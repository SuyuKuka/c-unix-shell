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

test: 
	$(CC) -o fsh $(SRC)/*.c $(LIBS)
	./test.sh

c:
	$(CC) -o fsh $(SRC)/*.c $(LIBS)
	./fsh

# Valgrind
valgrind: all
    valgrind --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --show-reachable=no \
            --track-fds=yes \
            --quiet ./fsh

# Nettoyage des fichiers générés
clean:
	rm -f $(BUILD)/*.o fsh
	rm -d $(BUILD)
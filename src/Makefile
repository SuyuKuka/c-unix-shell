CC = gcc

SRC = src
BUILD = build

all: makedir fsh

makedir: @echo "Création d'un répertoire..."
    mkdir -p $(BUILD)

fsh: @echo "Lancement du FSH..."

clean:
    rm -f $(BUILD)/*.o fsh
    rm -d $(BUILD)
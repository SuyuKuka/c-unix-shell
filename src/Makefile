CC = gcc

SRC = src
BUILD = build

all: makedir fsh

makedir:    @echo "Création du répertoire $(BUILD)..."
    mkdir -p $(BUILD)

fsh:    @echo "Lancement du FSH..."
    $(CC) -o fsh src/*.c

clean:
    rm -f $(BUILD)/*.o fsh
    rm -d $(BUILD) # rm -rf $(BUILD) ?
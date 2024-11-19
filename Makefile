CC = gcc

SRC = src
BUILD = build

all: makedir fsh

makedir:
    mkdir -p $(BUILD)

fsh:
    $(CC) -o fsh src/*.c

clean:
    rm -f $(BUILD)/*.o fsh
    rm -d $(BUILD) # rm -rf $(BUILD) pour forcer effaçage ?
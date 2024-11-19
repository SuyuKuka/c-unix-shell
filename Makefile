CC = gcc

SRC = src
BUILD = build
LIBS = -lreadline -lncurses

all: makedir fsh

makedir:
    mkdir -p $(BUILD)

fsh:
    $(CC) -o fsh src/*.c $(LIBS)

clean:
    rm -f $(BUILD)/*.o fsh
    rm -d $(BUILD) # rm -rf $(BUILD) pour forcer effaçage au cas où ?
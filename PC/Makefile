#MAKEFILE POUR LE PROGRAMME PC
#AUTEURS : KNOCKAERT-CUNIN

CC=gcc
SOURCE=usbPC.c
EXE=usbPC.exe
OPTIONS= -Wall -Wextra 
LIBS= -lusb-1.0 -lncurses

all:  $(SOURCE)
	$(CC) $(SOURCE) $(LIBS) -o $(EXE) $(OPTIONS) 

clean:
	rm $(EXE)

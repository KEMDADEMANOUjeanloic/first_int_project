#variable
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main1.c src/enregistrement.c src/planification.c
OBJ = $(SRC:.c=.o)
EXEC = progr

#Règle par dfaut
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@

#nettoyage
clean:
	rm -f $(OBJ)
mrproper: clean
	rm -f $(EXEC)

#Rgle de test
test: all
	./$(EXEC)


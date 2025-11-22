#variable
LIBS = -lglut -lGLU -lGL 
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main1.c src/enregistrement.c src/planification.c
OBJ = $(SRC:.c=.o)
EXEC = prog

#Règle par dfaut
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

#nettoyage
clean:
	rm -f $(OBJ)
mrproper: clean
	rm -f $(EXEC)

#Rgle de test
test: all
	./$(EXEC)


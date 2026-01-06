#variable

# GTK flags (used only for GUI build)
GTK_CFLAGS := $(shell pkg-config --cflags gtk4)
GTK_LIBS := $(shell pkg-config --libs gtk4)

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Sources pour le binaire CLI
SRC_CLI = src/main1.c src/enregistrement.c src/planification.c
OBJ_CLI = $(SRC_CLI:.c=.o)
EXEC = prog

# Sources pour le binaire GUI
SRC_GUI = src/interface.c src/enregistrement.c src/planification.c
OBJ_GUI = $(SRC_GUI:.c=.o)
EXEC_GUI = prog_gui

#Règle par défaut construit le CLI
all: $(EXEC)

$(EXEC): $(OBJ_CLI)
	$(CC) $(OBJ_CLI) -o $@

# Règle pour construire la GUI
gui: $(EXEC_GUI)

$(EXEC_GUI): CFLAGS += $(GTK_CFLAGS)
$(EXEC_GUI): $(OBJ_GUI)
	$(CC) $(OBJ_GUI) -o $@ $(GTK_LIBS)

#nettoyage
clean:
	rm -f $(OBJ)
mrproper: clean
	rm -f $(EXEC)

#Rgle de test
test: all
	./$(EXEC)


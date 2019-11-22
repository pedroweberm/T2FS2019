#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
EX_DIR=./exemplo

all: teste1 t2shell
teste1: $(EX_DIR)/teste1.c libt2fs
	$(CC) -o teste1 $(EX_DIR)/teste1.c -L$(LIB_DIR) -lt2fs -Wall
t2shell: $(EX_DIR)/t2shell.c libt2fs
	$(CC) -o t2shell $(EX_DIR)/t2shell.c -L$(LIB_DIR) -lt2fs -Wall
libt2fs: t2fs linked
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o $(BIN_DIR)/linked.o
t2fs: $(SRC_DIR)/t2fs.c 
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o
linked: $(SRC_DIR)/linked.c
	$(CC) -c $(SRC_DIR)/linked.c -o $(BIN_DIR)/linked.o -Wall

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~



DIR_INC = .
DIR_SRC = .
DIR_OBJ = .
DIR_BIN = .

SRC = $(wildcard ${DIR_SRC}/*.c)  
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = app

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc
CFLAGS = -g -w -fmax-errors=10 -std=c99 -fsigned-char -I${DIR_INC} -D CODECODE_CI

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@
    
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@
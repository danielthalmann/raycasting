NAME=raycast
SRCS=src\map.c\
	src\graphics.c
OBJS=${SRCS:.c=.o}

ifeq (${OS}, Windows_NT)
	LINKED=-Llib\SDL2-2.0.14\x86_64-w64-mingw32\lib -m64 -lmingw32 -mwindows -mconsole -lSDL2main -lSDL2
	COMPILE=-Ilib\SDL2-2.0.14\x86_64-w64-mingw32\include
else
	LINKED=-m64 -lSDL2main -lSDL2 
	COMPILE=
endif

CC=gcc

%.o: %.c
	${CC} ${COMPILE} -c $< -o $@

${NAME}: ${OBJS}
	${CC} ${OBJS} -o $@ ${LINKED}

all: ${NAME}

clean: 
	del ${OBJS}

os: 
	@echo ${OS}
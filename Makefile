NAME=raycast
SRCS=src\map.c\
	src\graphics.c
OBJS=${SRCS:.c=.o}
COMPILE=-Ilib\SDL2-2.0.14\x86_64-w64-mingw32\include
LINKED=-Llib\SDL2-2.0.14\x86_64-w64-mingw32\lib -m64 -lmingw32 -mwindows -mconsole -lSDL2main -lSDL2 
CC="C:\Program Files\CodeBlocks\MinGW\bin\gcc.exe"

%.o: %.c
	${CC} ${COMPILE} -c $< -o $@

${NAME}: ${OBJS}
	${CC} ${OBJS} -o $@ ${LINKED}

all: ${NAME}

clean: 
	del ${OBJS}
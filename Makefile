CC = gcc
FLAGS = -std=c99 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat=2 -g -O3 -pedantic
IN = main.c
OUT = mini_paint.exe
CODES = ansi_codes.h
CFG = config.h

run: ${OUT}
	./${OUT}

build: ${OUT}

${OUT}: ${IN} ${CODES} ${CFG}
	${CC} ${FLAGS} -o ${OUT} ${IN}

clean:
	rm *.exe *.out *.o *.ppm *.jpg *.png *.ico

.PHONY: run build

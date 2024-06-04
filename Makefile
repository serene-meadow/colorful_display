NAME := program
VAR_DIR := native
BLD_DIR := build
SRC_DIR := source
ART_DIR := artifact

.DEFAULT_GOAL := ${ART_DIR}/${VAR_DIR}/${NAME}

COMPILER := c++

COMPILER_FLAG_AUXI_LIST := $(shell pkg-config --cflags sdl2 SDL2_net)
COMPILER_FLAG_LIST := -std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -MMD -MP ${COMPILER_FLAG_AUXI_LIST}

LINKER_FLAG_AUXI_LIST := $(shell pkg-config --libs sdl2 SDL2_net)
LINKER_FLAG_LIST := ${LINKER_FLAG_AUXI_LIST}

OBJ_LIST := $(patsubst ${SRC_DIR}/%.cpp,${BLD_DIR}/${VAR_DIR}/%.o,$(wildcard ${SRC_DIR}/*.cpp))
DEP_LIST := $(OBJ_LIST:.o=.d)

.PHONY: all clean list web

all: ${.DEFAULT_GOAL} web

${.DEFAULT_GOAL}: ${OBJ_LIST} | ${ART_DIR}/${VAR_DIR}
	${COMPILER} $^ ${LINKER_FLAG_LIST} -o $@

web:
	make \
		NAME:=${NAME}.html \
		VAR_DIR:=web \
		COMPILER:=em++ \
		COMPILER_FLAG_AUXI_LIST:='-sUSE_SDL=2 -sUSE_SDL_MIXER=2' \
		LINKER_FLAG_AUXI_LIST:='-sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sASYNCIFY'

${OBJ_LIST}: ${BLD_DIR}/${VAR_DIR}/%.o: ${SRC_DIR}/%.cpp | ${BLD_DIR}/${VAR_DIR}
	${COMPILER} ${COMPILER_FLAG_LIST} -c $< -o $@

${ART_DIR}/${VAR_DIR} ${BLD_DIR}/${VAR_DIR}:
	mkdir --parents $@

list:
	@printf '%s\n' 'object       files : ${OBJ_LIST}'
	@printf '%s\n' 'dependencies files : ${DEP_LIST}'

clean:
	rm --force --recursive ${ART_DIR}
	rm --force --recursive ${BLD_DIR}

-include $(DEP_LIST)

NAME := colorful_display
VAR_DIR := native
BLD_DIR := build
SRC_DIR := source
ART_DIR := artifact

.DEFAULT_GOAL := ${ART_DIR}/${VAR_DIR}/${NAME}

COMPILER := c++

COMPILER_FLAG_AUXI_LIST := -fsanitize=undefined $(shell pkg-config --cflags sdl2)
COMPILER_FLAG_LIST := -std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -MMD -MP ${COMPILER_FLAG_AUXI_LIST}

LINKER_FLAG_AUXI_LIST := -fsanitize=undefined $(shell pkg-config --libs sdl2)
LINKER_FLAG_LIST := ${LINKER_FLAG_AUXI_LIST}

OBJ_LIST := $(patsubst ${SRC_DIR}/%.cpp,${BLD_DIR}/${VAR_DIR}/%.o,$(wildcard ${SRC_DIR}/*.cpp))
DEP_LIST := $(OBJ_LIST:.o=.d)

.PHONY: all list_objects clean_all

WEB_ARTIFACTS := ${ART_DIR}/web/${NAME}.wasm ${ART_DIR}/web/${NAME}.js

all: ${.DEFAULT_GOAL} ${WEB_ARTIFACTS}

${.DEFAULT_GOAL}: ${OBJ_LIST} | ${ART_DIR}/${VAR_DIR}
	${COMPILER} $^ ${LINKER_FLAG_LIST} -o $@

${WEB_ARTIFACTS}:
	make \
		NAME:=${NAME}.js \
		VAR_DIR:=web \
		COMPILER:=em++ \
		COMPILER_FLAG_AUXI_LIST:='-sUSE_SDL=2' \
		LINKER_FLAG_AUXI_LIST:='-sUSE_SDL=2'

${OBJ_LIST}: ${BLD_DIR}/${VAR_DIR}/%.o: ${SRC_DIR}/%.cpp | ${BLD_DIR}/${VAR_DIR}
	${COMPILER} ${COMPILER_FLAG_LIST} -c $< -o $@

${ART_DIR}/${VAR_DIR} ${BLD_DIR}/${VAR_DIR}:
	mkdir --parents $@

list_objects:
	@printf '%s\n' '${OBJ_LIST}'

clean_all:
	rm --force --recursive ${ART_DIR}
	rm --force --recursive ${BLD_DIR}

-include $(DEP_LIST)

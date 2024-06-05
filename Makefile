NAME := colorful_display
SRC_DIR := source
BLD_DIR := build/native
ART_DIR := artifact/native

.DEFAULT_GOAL := ${ART_DIR}/${NAME}

COMPILER := c++

COMPILER_FLAG_AUXI_LIST := -fsanitize=undefined $(shell pkg-config --cflags sdl2)
COMPILER_FLAG_LIST := -std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -MMD -MP ${COMPILER_FLAG_AUXI_LIST}

LINKER_FLAG_AUXI_LIST := -fsanitize=undefined $(shell pkg-config --libs sdl2)
LINKER_FLAG_LIST := ${LINKER_FLAG_AUXI_LIST}

OBJ_LIST := $(patsubst ${SRC_DIR}/%.cpp,${BLD_DIR}/%.o,$(wildcard ${SRC_DIR}/*.cpp))
DEP_LIST := $(OBJ_LIST:.o=.d)

WEB_MAKER := make \
	NAME:=${NAME}.js \
	BLD_DIR:=build/web \
	ART_DIR:=artifact/web \
	COMPILER:=em++ \
	COMPILER_FLAG_AUXI_LIST:='-sUSE_SDL=2' \
	LINKER_FLAG_AUXI_LIST:='-sUSE_SDL=2'

.PHONY: all list_objects clean clean_all

WEB_ARTIFACTS := artifact/web/${NAME}.wasm artifact/web/${NAME}.js
WEBSITE_UPDATE := $(patsubst artifact/web/%,website/generated/%,${WEB_ARTIFACTS})

${WEBSITE_UPDATE}: ${WEB_ARTIFACTS} | website/generated/
	cp ${WEB_ARTIFACTS} website/generated/

all: ${.DEFAULT_GOAL} ${WEBSITE_UPDATE}

${.DEFAULT_GOAL}: ${OBJ_LIST} | ${ART_DIR}
	${COMPILER} $^ ${LINKER_FLAG_LIST} -o $@

${WEB_ARTIFACTS}:
	${WEB_MAKER}

${OBJ_LIST}: ${BLD_DIR}/%.o: ${SRC_DIR}/%.cpp | ${BLD_DIR}
	${COMPILER} ${COMPILER_FLAG_LIST} -c $< -o $@

${ART_DIR} ${BLD_DIR} website/generated/: %:
	mkdir --parents $@

list_objects:
	@printf '%s\n' '${OBJ_LIST}'

clean:
	rm --force --recursive ${ART_DIR}
	rm --force --recursive ${BLD_DIR}

clean_all: clean
	${WEB_MAKER} clean

-include $(DEP_LIST)

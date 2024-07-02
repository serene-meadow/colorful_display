# The target defines what is built. \
	`native`: Native executable program. \
	`web`: WebAssembly program with JavaScipt script to load it. 
target := native

# base name of artifact
name := colorful_display

# source directory
SRC_DIR := source

# build directory
BLD_DIR := build/${target}

# artifact directory
ART_DIR := artifact/${target}

# directory for website content made by Emscripten
WEB_DIR := website/compiled

# object files
OBJ_LIST := $(patsubst ${SRC_DIR}/%.cpp,${BLD_DIR}/%.o,$(wildcard ${SRC_DIR}/*.cpp))

# dependency files for `make`
DEP_LIST := $(OBJ_LIST:.o=.d)

COMPILER_FLAG_LIST := -std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -MMD -MP
LINKER_FLAG_LIST := #(empty string)

ifeq (${target}, native)
.DEFAULT_GOAL := ${ART_DIR}/${name}
compiler := c++
COMPILER_FLAG_LIST += -fsanitize=undefined -D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_SANITIZE_VECTOR $(shell pkg-config --cflags sdl2)
LINKER_FLAG_LIST += -fsanitize=undefined $(shell pkg-config --libs sdl2)
ARTIFACT := ${ART_DIR}/${name}
else ifeq (${target}, web)
.DEFAULT_GOAL := ${ART_DIR}/${name}.js
compiler := em++
COMPILER_FLAG_LIST += -sUSE_SDL=2
LINKER_FLAG_LIST += -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH
ARTIFACT := ${ART_DIR}/${name}.js ${ART_DIR}/${name}.wasm
else
$(error Unsupported target "${target}".)
endif

.PHONY: all website info clean
.POSIX: #(More portable?)

# Update website. (If not in `web` target, switch to `web` target to update the website.)
ifeq (${target}, web)
WEBSITE_CONTENT := $(patsubst ${ART_DIR}/%,${WEB_DIR}/%,${ARTIFACT})
${WEBSITE_CONTENT}: ${ARTIFACT} | ${WEB_DIR}
	cp ${ARTIFACT} ${WEB_DIR}/
website: ${WEBSITE_CONTENT}
else
website:
	make target=web website
endif

# Make all targets.
ifeq (${target}, web)
all: website
	make target=native
else
all:
	make target=web all
endif

info:
	@printf '%s\n' 'Program Name: ${name}'
	@printf '%s\n' 'Default Goal: ${.DEFAULT_GOAL}'
	@printf '%s\n' 'Artifact: ${ARTIFACT}'
	@printf '%s\n' 'Object Files: ${OBJ_LIST}'
	@printf '%s\n' 'Artifact Directory: ${ART_DIR}'
	@printf '%s\n' 'Source Directory: ${SRC_DIR}'
	@printf '%s\n' 'Website Content: ${WEBSITE_CONTENT}'
	@printf '%s\n' 'Website Content Directory: ${WEB_DIR}'

# Build program.
ifeq (${target}, web)
${ARTIFACT}: ${OBJ_LIST} | ${ART_DIR}
	${compiler} $^ ${LINKER_FLAG_LIST} -o ${ART_DIR}/${name}.js
else
${ARTIFACT}: ${OBJ_LIST} | ${ART_DIR}
	${compiler} $^ ${LINKER_FLAG_LIST} -o $@
endif

# Build object files.
${OBJ_LIST}: ${BLD_DIR}/%.o: ${SRC_DIR}/%.cpp | ${BLD_DIR}
	${compiler} ${COMPILER_FLAG_LIST} -c $< -o $@

# Create directories.
${ART_DIR} ${BLD_DIR}: %:
	mkdir --parents $@
ifeq (${target}, web)
${WEB_DIR}:
	mkdir --parents $@
endif

# Remove built artifacts.
clean:
	rm --force --recursive ${ART_DIR}
	rm --force --recursive ${BLD_DIR}

-include $(DEP_LIST)

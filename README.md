# Colorful Display in a [Webpage](https://adrian-javid.github.io/colorful_display/)

This program displays animated color gradients.

![A color gradient.](website/preview_image.png)

The purpose of this repository is to host a [webpage](https://adrian-javid.github.io/colorful_display/) that displays the graphical output of the program.

## Project File Structure
* [`source/`](source/) contains the C++ code for the graphical program.
* [`website/`](website/) is the root of which to host the webpage.

## Building

This project uses a [`Makefile`](https://www.gnu.org/software/make/manual/make.html) to build.

This project builds for Linux and [WebAssembly (WASM)](https://webassembly.org/).

When building either for the native target or the website target, an `artifact` directory and a `build` directory are created. The `build` directory is for object files and dependency files. The compiled program is put into the `artifact` directory.


When compiling for the web, another additional directory [`website/compiled`](website/compiled) is created if it doesn't already exist. The WebAssembly program is copied to here so that the webpage can use it.

```sh
# Compiles both the native target and the website target.
make all
```

### Building Natively

The default target of the `make` command is the native program.
```sh
# Compiles natively.
make
```
Once compiled, the program can be found at `artifact/native/colorful_display`.

By default, the `Makefile` will use whatever C++ compiler the `c++` command refers to. The C++ compiler can be specified as an argument to the `make` command.
```sh
# Compiles natively and uses the Clang C++ compiler.
make compiler=clang++
```

This following command removes the build files for the native target created by the `make` command.
```sh
make clean
```

### Building for the Web

```sh
# Compiles the WebAssembly program into the website's directory.
make website
```

## Dependencies

Linux [`make`](https://www.gnu.org/software/make/) is used to build this program.

The [Emscripten](https://emscripten.org/) compiler is used to compile the C++ code to WebAssembly for the webpage. To compile natively, another compiler is needed.

[Simple DirectMedia Layer (SDL)](https://www.libsdl.org/) is used for graphics and input.

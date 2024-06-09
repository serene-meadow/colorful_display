# Colorful Display

This is a program that displays animated color gradients.

The purpose of this repository is to host a webpage that displays the graphical output of the program.

## Project File Structure
* `source/` contains the C++ code for the graphical program.
* `website/` is the root of which to host the webpage.

## Building

This project uses a [`Makefile`](https://www.gnu.org/software/make/manual/make.html) to build.

Since this repository is mainly to build a single webpage, I didn't feel the need to add options for building on other platforms besides Linux and [WebAssembly](https://webassembly.org/).

When building either for the native target or the web target, an `artifact` directory and a `build` directory are created. The `build` directory is for object files and dependency files. The compiled program is put into the `artifact` directory.


When compiling for the web, another additional directory `website/compiled` is created if it doesn't already exist. The WebAssembly program is copied to here so that the webpage can use it.

```sh
# This command compiles both the native target and the web target.
make all
```

### Building Natively

The default target of the `make` command is the native program.
```sh
# Compile natively.
make
```
The program can be found at `artifact/native/colorful_display`.

By default, the `Makefile` will use whatever C++ compiler the `c++` command refers to. The C++ compiler can be specified as an argument to the `make` command.
```sh
# Compile natively and use the Clang C++ compiler.
make compiler=clang++
```

This following command removes the build files for the native target created by the `make` command.
```sh
make clean
```

### Building for the Web

```sh
# Compile the WebAssembly program into the website's directory.
make website
```

## Dependencies

Linux [`make`](https://www.gnu.org/software/make/) is used to build the program.

The [Emscripten](https://emscripten.org/) compiler is used to compile the C++ code to WebAssembly for the webpage. To compile natively, another compiler is needed.

[Simple DirectMedia Layer (SDL)](https://www.libsdl.org/) is used for the graphics.

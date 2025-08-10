# Project Overview

This project is a C application that uses the `raylib` library to create an interactive 3D visualization of musical dissonance. The application generates a "dissonance surface" where the height of the surface at any point represents the calculated dissonance between a set of complex tones.

The core of the project is in `main.c`, which sets up the `raylib` window, camera, and shader uniforms. It also defines the musical voices and their harmonic partials. The actual rendering of the dissonance surface is handled by a GLSL fragment shader, `dissonance.fs`, which uses raymarching to draw the complex 3D shape.

The project also includes a simple `Makefile` for easy compilation.

## Building and Running

To build the project, you can use the `make` command. This will compile the `main.c` file and link it with the `raylib` library, creating an executable named `hello`.

```bash
make
```

To run the application, execute the following command:

```bash
./hello
```

## Development Conventions

The code is written in C and follows a clear structure. The `main.c` file is well-commented, with sections for initialization, camera setup, shader and uniform setup, voice data setup, and the main game loop.

The project uses a custom shader for rendering, which is a common practice in `raylib` for more advanced graphics. The shader code is located in `dissonance.fs`.

The project relies on the `raylib` library, so any further development should be done with the `raylib` API in mind. The `raylib.h` header file is included in `main.c`, and the `libraylib.a` static library is linked during compilation.

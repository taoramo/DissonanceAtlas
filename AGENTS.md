# Agent Instructions for raylib_helloworld

## Build Commands

### Build the project
```bash
make
```

### Clean build artifacts
```bash
make clean
```

### Run the application
```bash
./hello
```

### Format code
```bash
clang-format -i *.c *.h
```

### Format all source files recursively
```bash
find . -name "*.c" -o -name "*.h" | xargs clang-format -i
```

## Formatting Configuration
- **Tool**: clang-format with project-specific `.clang-format` configuration
- **Key settings**:
  - Indent width: 2 spaces
  - Column limit: 120 characters
  - Pointer alignment: Right
  - Brace style: Attach (opening brace on same line)
  - No tabs, only spaces

## Code Style Guidelines

### Language and Framework
- **Language**: C programming language
- **Framework**: raylib game library
- **Compiler**: clang with flags `-Wextra -Wall`

### Naming Conventions
- **Defines**: `ALL_CAPS` (e.g., `#define MAX_PARTIALS 6`)
- **Macros**: `ALL_CAPS`
- **Variables**: `lowerCase` (e.g., `screenWidth`, `cameraMesh`)
- **Constants**: `lowerCase` (e.g., `const int maxValue = 8`)
- **Pointers**: `MyType *pointer` (e.g., `Texture2D *array = NULL`)
- **Enums**: `TitleCase` (e.g., `enum TextureFormat`)
- **Enum members**: `ALL_CAPS` (e.g., `PIXELFORMAT_UNCOMPRESSED_R8G8B8`)
- **Structs**: `TitleCase` (e.g., `struct Texture2D`)
- **Struct members**: `lowerCase` (e.g., `texture.width`)
- **Functions**: `TitleCase` (e.g., `InitWindow()`, `LoadRenderTextureFloat()`)
- **Function parameters**: `lowerCase` (e.g., `width`, `height`)

### Formatting Rules
- **Indentation**: 2 spaces (no tabs)
- **Column limit**: 120 characters
- **Float values**: Always use `x.xf` suffix (e.g., `10.0f`)
- **Operators**: No spaces around `*` and `/`, single space around `+` and `-`
- **Braces**: Opening brace on same line, closing brace aligned
- **Control statements**: Followed by space (e.g., `if (condition)`)
- **Comments**: Start with space + capital letter, no trailing period
- **Includes**: System headers first, then local headers with quotes

### Code Structure
- **Header guards**: `#ifndef HEADER_H` / `#define HEADER_H` / `#endif`
- **Function organization**: Related functions grouped together
- **Variable initialization**: Always initialize variables
- **Error handling**: Use raylib's logging system (`TraceLog`, `LOG_ERROR`)
- **Resource management**: Properly unload resources (`UnloadTexture`, `UnloadShader`, etc.)

### File Organization
- **Headers**: `.h` files with function declarations and struct definitions
- **Sources**: `.c` files with implementations
- **Shaders**: `.vs`, `.fs` files for vertex and fragment shaders
- **Naming**: Use `snake_case` for files (e.g., `dissonance.c`, `terrain.vs`)

### Dependencies
- **raylib**: Main game library (linked via `libraylib.a`)
- **OpenGL**: For graphics rendering
- **Standard libraries**: `stdio.h`, `stdlib.h`, `math.h`

### Platform Specific
- **macOS**: Requires framework flags (`-framework OpenGL -framework Cocoa`, etc.)
- **Audio**: CoreAudio framework
- **Input**: IOKit framework

## Testing
No formal testing framework is currently set up. Manual testing by running the application and verifying functionality is recommended.

## Code Quality
- Always check for shader loading errors with `IsShaderValid()`
- Verify texture creation success
- Handle window initialization failures
- Use raylib's logging system for debugging

## Known Issues and Warnings

### OpenGL Deprecation Warning
- **Issue**: `glBindTexture` is deprecated on macOS 10.14+
- **Solution**: Define `GL_SILENCE_DEPRECATION` in compiler flags to suppress warnings
- **Alternative**: Consider migrating to raylib's higher-level texture functions

### C++ Standard Requirement
- **Issue**: raylib.h requires C++11 or later
- **Solution**: Add `-std=c++11` (or later) to compiler flags
- **Note**: This project uses C, but raylib headers may require C++ standard flags

### Compiler Flags
Current flags in use:
```bash
CFLAGS = -Wextra -Wall
```

Recommended flags to address issues:
```bash
CFLAGS = -Wextra -Wall -std=c++11 -DGL_SILENCE_DEPRECATION
```
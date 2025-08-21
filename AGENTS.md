# Agent Instructions for raylib_helloworld

## Build Commands
- **Build**: `make`
- **Clean**: `make clean`
- **Run**: `./hello`
- **Format**: `clang-format -i *.c *.h`
- **Format all**: `find . -name "*.c" -o -name "*.h" | xargs clang-format -i`

## Code Style Guidelines
- **Language**: C with raylib framework
- **Compiler**: `clang -Wextra -Wall -std=c99 -DGL_SILENCE_DEPRECATION`
- **Formatting**: clang-format with 2-space indentation, 120 char limit, right-aligned pointers
- **Naming**: `lowerCase` vars, `TitleCase` functions/structs, `ALL_CAPS` defines/macros
- **Files**: `snake_case` for .c/.h, .vs/.fs for shaders
- **Includes**: System headers first, then local with quotes
- **Comments**: Start with space + capital, no trailing period
- **Error handling**: Use raylib's `TraceLog(LOG_ERROR, ...)`
- **Resource management**: Always unload resources (`UnloadTexture`, `UnloadShader`, etc.)

## Dependencies
- **raylib**: Main game library (libraylib.a)
- **OpenGL**: Graphics rendering
- **macOS frameworks**: OpenGL, Cocoa, IOKit, CoreAudio

## Testing
No formal testing framework. Manual testing: run `./hello` and verify functionality.

## Code Quality
- Check shader loading with `IsShaderValid()`
- Verify texture/render target creation
- Handle window initialization failures
- Use raylib logging for debugging

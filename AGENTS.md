# Agent Instructions for raylib_helloworld

## Build Commands
- **Build**: `make`
- **Clean**: `make clean`
- **Run**: `./atlas`
- **Format**: `clang-format -i *.c *.h`
- **Format all**: `find . -name "*.c" -o -name "*.h" | xargs clang-format -i`


## Code Style Guidelines
- **Language**: C99 with raylib framework
- **Compiler**: `clang -Wextra -Wall -std=c99 -DGL_SILENCE_DEPRECATION`
- **Formatting**: clang-format (2-space indent, 120 char limit, right-aligned pointers)
- **Naming**: `lowerCase` vars, `TitleCase` functions/structs, `ALL_CAPS` defines/macros
- **Files**: `snake_case` for .c/.h, .vs/.fs for shaders
- **Includes**: System headers first (<>), then local with quotes ("")
- **Comments**: Start with space + capital, no trailing period
- **Error handling**: Use raylib's `TraceLog(LOG_ERROR, ...)` for errors
- **Resource management**: Always unload resources (`UnloadTexture`, `UnloadShader`, etc.)

## Testing
- **Manual testing**: Run `./atlas` and verify 3D terrain visualization
- **Audio testing**: Press 'T' to toggle sine wave playback
- **Input testing**: Use WASD/RF keys for camera movement, right-click for terrain sampling

## Dependencies
- **raylib**: Game library (libraylib.a)
- **miniaudio**: Audio processing (miniaudio.h)
- **OpenGL**: Graphics rendering via GL3
- **macOS**: Frameworks - OpenGL, Cocoa, IOKit, CoreAudio, CoreVideo

## Code Quality
- Validate shaders with `IsShaderValid()`
- Check texture/render target creation
- Handle window/audio initialization failures
- Use raylib logging for debugging
- Free all allocated resources before exit
- **Note**: raylib.h requires C++11+ but project uses C99 - use build system, not direct linting

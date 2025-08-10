NAME = hello
MACOS_FLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo -framework QuartzCore
LIBFLAGS = -lraylib -L. -lvulkan -Wl,-rpath,/usr/local/lib
INCLUDES = -I~/VulkanSDK/1.4.321.0/macOS/include/
CFLAGS = -Wextra -Wall -std=c99
SRC = main.c vulkan_backend.m
SHADERS = dissonance.fs

all: $(NAME)

# Compile GLSL to SPIR-V
dissonance.spv: dissonance.fs
	glslc -fshader-stage=fragment $(SHADERS) -o dissonance.spv

# Main target
$(NAME): $(SRC) dissonance.h vulkan_backend.h dissonance.spv libraylib.a
	clang $(SRC) -o $(NAME) $(CFLAGS) $(MACOS_FLAGS) $(LIBFLAGS) $(INCLUDES)

clean:
	rm -f $(NAME) dissonance.spv

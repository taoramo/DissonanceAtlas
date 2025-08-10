NAME = hello
MACOS_FLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo -framework QuartzCore
LIBFLAGS = -lraylib -L. -lvulkan -Wl,-rpath,/usr/local/lib
INCLUDES = -I~/VulkanSDK/1.4.321.0/macOS/include/
CFLAGS = -Wextra -Wall -std=c99
SRC = main.c vulkan_backend.m
SHADERS = dissonance.fs

all: $(NAME)

# Compile GLSL to SPIR-V
dissonance.frag.spv: dissonance.fs
	glslc -fshader-stage=fragment dissonance.fs -o dissonance.frag.spv

dissonance.vert.spv: dissonance.vert
	glslc -fshader-stage=vertex dissonance.vert -o dissonance.vert.spv

# Main target
$(NAME): $(SRC) dissonance.h vulkan_backend.h dissonance.frag.spv dissonance.vert.spv libraylib.a
	clang $(SRC) -o $(NAME) $(CFLAGS) $(MACOS_FLAGS) $(LIBFLAGS) $(INCLUDES)

clean:
	rm -f $(NAME) dissonance.frag.spv dissonance.vert.spv

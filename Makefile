NAME = atlas
MACOS_FLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
LIBFLAGS = -lraylib -L.
CFLAGS = -Wextra -Wall -std=c99 -O3 -DGL_SILENCE_DEPRECATION
SRC = main.c dissonance.c

all: $(NAME)

$(NAME): $(SRC) dissonance.h dissonance.fs dissonance.vs libraylib.a
	clang $(SRC) -o $(NAME) $(CFLAGS) $(MACOS_FLAGS) $(LIBFLAGS)

clean:
	rm -f $(NAME)

.PHONY: all clean

NAME = hello
MACOS_FLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
LIBFLAGS = -lraylib -L.
CFLAGS = -Wextra -Wall
SRC = main.c dissonance.c


all: $(NAME)

$(NAME): $(SRC) dissonance.h dissonance.fs dissonance.vs libraylib.a
	clang $(SRC) -o $(NAME) $(CFLAGS) $(MACOS_FLAGS) $(LIBFLAGS)

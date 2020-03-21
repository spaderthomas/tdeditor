#include <unistd.h>

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define CONSOLE_BLUE 1
#define CONSOLE_GREEN 2
#define CONSOLE_RED 4
#define CONSOLE_PURPLE 5
#define CONSOLE_YELLOW 6

void print_color(int color, const char* message) {
	printf(message);
}

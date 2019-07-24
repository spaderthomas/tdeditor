#include <windows.h>   // WinApi header

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

static const char* conf = "C:/Programming/tdeditor/src/tded.conf";

#define CONSOLE_BLUE 1
#define CONSOLE_GREEN 2
#define CONSOLE_RED 4
#define CONSOLE_PURPLE 5
#define CONSOLE_YELLOW 6

void print_color(int color, const char* message) {
	HANDLE win_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO old_color;
	GetConsoleScreenBufferInfo(win_stdout, &old_color);

	SetConsoleTextAttribute(win_stdout, color);
	printf("%s", message);
	SetConsoleTextAttribute(win_stdout, old_color.wAttributes);
}

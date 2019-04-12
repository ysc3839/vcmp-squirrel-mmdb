void output_console_message(const char *msg)
{
#ifdef _WIN32
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbBefore;
	GetConsoleScreenBufferInfo(hstdout, &csbBefore);
	SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN);
	printf("[SqMMDB] ");

	SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
	printf("%s\n", msg);

	SetConsoleTextAttribute(hstdout, csbBefore.wAttributes);
#else
	printf("\033[0;32m[SqMMDB]\033[0;37m %s\n", msg);
#endif
}

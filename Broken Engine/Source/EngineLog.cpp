#include "EngineLog.h"
#include <Windows.h>

BE_BEGIN_NAMESPACE
void ExLogCompilationFirstMessage() {
	BrokenEngine::LogCompilationFirstMessage();
}

void ExEngineConsoleLog(const char file[], int line, const char* format, ...) {
	va_list args;
	va_start(args, format);
	BrokenEngine::EngineConsoleLog(file, line, format, args);
	va_end(args);
}

void ExSystemConsoleLog(const char file[], int line, const char* format, ...) {
	static char tmp_string[MAX_BUF_SIZE];
	static char tmp_string2[MAX_BUF_SIZE];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, MAX_BUF_SIZE, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, MAX_BUF_SIZE, "\n%s(%d) : %s", file, line, tmp_string);
	OutputDebugString(tmp_string2);
	OutputDebugStringA(tmp_string2);

	std::cout << tmp_string2 << std::endl;
}
BE_END_NAMESPACE
#ifndef __ENGINELOG_H__
#define __ENGINELOG_H__
#pragma once

#include "BrokenCore.h"
//
////Undefine internal logs for external logging
//#undef ENGINE_CONSOLE_LOG
//#undef SYSTEM_CONSOLE_LOG
//#undef ENGINE_AND_SYSTEM_CONSOLE_LOG
//#undef COMPILATIONLOGINFO


//Redefining
#define EX_ENGINE_CONSOLE_LOG(format, ...) Broken::ExEngineConsoleLog(__FILE__, __LINE__, format, __VA_ARGS__)
/// Print only in System Console
#define EX_SYSTEM_CONSOLE_LOG(format, ...) Broken::ExSystemConsoleLog(__FILE__, __LINE__, format, __VA_ARGS__)
/// Print in both Consoles
#define EX_ENGINE_AND_SYSTEM_CONSOLE_LOG(format, ...) Broken::ExEngineConsoleLog(__FILE__, __LINE__, format, __VA_ARGS__); Broken::ExSystemConsoleLog(__FILE__, __LINE__, format, __VA_ARGS__)
/// First info at Compilation
#define EX_COMPILATIONLOGINFO Broken::ExLogCompilationFirstMessage()

BE_BEGIN_NAMESPACE
void BROKEN_API ExLogCompilationFirstMessage();
void BROKEN_API ExEngineConsoleLog(const char file[], int line, const char* format, ...);
void BROKEN_API ExSystemConsoleLog(const char file[], int line, const char* format, ...);
BE_END_NAMESPACE



#endif


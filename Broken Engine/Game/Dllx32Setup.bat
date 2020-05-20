@echo off
del /q *.dll
xcopy "%CD%\dlls\x86" "%CD%" /i /q

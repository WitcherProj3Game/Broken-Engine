@echo off
del /q *.dll
xcopy "%CD%\dlls\x64" "%CD%" /i /q

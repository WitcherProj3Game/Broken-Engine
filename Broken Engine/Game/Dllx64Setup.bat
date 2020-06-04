@echo off
if not EXIST SDL2.dll (
del /q *.dll
xcopy "%CD%\dlls\x64" "%CD%" /i /q
)

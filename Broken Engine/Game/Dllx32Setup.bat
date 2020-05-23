@echo off
if not EXIST SDL2_x86.dll (
del /q *.dll
xcopy "%CD%\dlls\x86" "%CD%" /i /q
)

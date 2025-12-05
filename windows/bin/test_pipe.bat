@echo off
chcp 65001 > nul

echo Введите числа через пробел
set /p userInput=

echo %userInput% | M.exe | A.exe | P.exe | S.exe

pause
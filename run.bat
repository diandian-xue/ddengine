@echo off

cd /d %DDENGINE_PROJ%

:: bin\\Debug\\simple-httpd.exe
:: bin\\Debug\\ddengine_test.exe

bin\\Debug\\lua.exe %DDENGINE_ROOT%\\example\\simple-httpd\\main.lua


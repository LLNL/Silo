@echo off
:: /**** usage: silodiff.bat file1 file2
:: file1 becomes %1
:: file2 becomes %2

:: make sure we have two files on the command line 
:: well actually, this simply checks for the existence of two args on the 
:: command line, no error-checking, but browser will error out if
:: the args aren't correct

if "%1"=="" goto Usage
if "%2"=="" goto Usage
goto RunBrowser

:Usage
echo usage: silodiff.bat file1 file2
goto end

:RunBrowser
:: %~dp0 contains the path to this batch file, and should also be
:: the location where browser.exe exists.

%~dp0\browser.exe -e diff %1 %2

:end

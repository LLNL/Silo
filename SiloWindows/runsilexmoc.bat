@echo off
set MOC=%QTDIR%\bin\moc.exe
set SILEX=..\..\tools\silex


echo ************************************************************************
echo Preprocessing silex source using %MOC%
echo ...

echo Running moc on the silex source

%MOC% %SILEX%\Explorer.h        -o %SILEX%\Explorer_moc.cpp
%MOC% %SILEX%\SiloArrayView.h   -o %SILEX%\SiloArrayView_moc.cpp
%MOC% %SILEX%\SiloDirTreeView.h -o %SILEX%\SiloDirTreeView_moc.cpp
%MOC% %SILEX%\SiloDirView.h     -o %SILEX%\SiloDirView_moc.cpp
%MOC% %SILEX%\SiloObjectView.h  -o %SILEX%\SiloObjectView_moc.cpp
%MOC% %SILEX%\SiloValueView.h   -o %SILEX%\SiloValueView_moc.cpp
%MOC% %SILEX%\SiloView.h        -o %SILEX%\SiloView_moc.cpp

echo ...
echo Done.
echo ************************************************************************

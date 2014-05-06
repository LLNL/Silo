@echo off
set HDF5_IN=%1\hdf5dll.dll
set ZLIB_IN=%2\zlib1.dll
set SZIP_IN=%3\szlibdll.dll
set SILO_IN=%4\silohdf5.dll

set HDF5_OUT=%5\hdf5dll.dll
set ZLIB_OUT=%5\zlib1.dll
set SZIP_OUT=%5\szlibdll.dll
set SILO_OUT=%5\silohdf5.dll

echo ************************************************************************
echo copying dependent libs to test directory
echo ...

if not exist %HDF5_OUT% (
 copy %HDF5_IN% %HDF5_OUT%
)

if not exist %ZLIB_OUT% (
 copy %ZLIB_IN% %ZLIB_OUT%
)

if not exist %SZIP_OUT% (
 copy %SZIP_IN% %SZIP_OUT%
)

if not exist %SILO_OUT% (
 copy %SILO_IN% %SILO_OUT%
)

echo ...
echo Done.
echo ************************************************************************

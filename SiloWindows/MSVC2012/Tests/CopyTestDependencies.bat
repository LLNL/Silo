@echo off
set HDF5_IN=%1\hdf5.dll
set ZLIB_IN=%2\zlib.dll
set SZIP_IN=%3\szlibdll.dll
set SILO_IN=%4\silohdf5.dll

set HDF5_OUT=%5\hdf5.dll
set ZLIB_OUT=%5\zlib.dll
set SZIP_OUT=%5\szlibdll.dll
set SILO_OUT=%5\silohdf5.dll

set PION_IN=%5\..\..\..\tests\pion0244.silo
set PION_OUT=%5\pion0244.silo

set Z1PLT_IN=%5\..\..\..\tests\z1plt.silo
set Z1PLT_OUT=%5\z1plt

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

echo ************************************************************************
echo copying test data files to test directory
echo ... from

if not exist %PION_OUT% (
 copy %PION_IN% %PION_OUT%
)

if not exist %Z1PLT_OUT% (
 copy %Z1PLT_IN% %Z1PLT_OUT%
)


echo ...
echo Done.
echo ************************************************************************

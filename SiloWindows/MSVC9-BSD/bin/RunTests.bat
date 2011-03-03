@echo off

set errorfile=.\error.txt 
set resultsfile=.\results.txt 
set drivers=(DB_PDB DB_HDF5)
set buildDir=..\Win32\DLLwithHDF5_Release\


set gp=(version.exe namescheme.exe point.exe extface.exe testall.exe TesReadMask.exe array.exe multi_test.exe partial_io.exe simple.exe ucd.exe ucdsamp3.exe obj.exe onehex.exe oneprism.exe onepyramid.exe onetet.exe subhex.exe twohex.exe multispec.exe sami.exe specmix.exe spec.exe alltypes.exe wave.exe polyzl.exe csg.exe rocket.exe mmadjacency.exe mat3d_3across.exe ucd1d.exe dirtest.exe quad.exe namescheme.exe)


:: copy necessary dlls to this executable directory
if not exist .\silohdf5.dll  copy %buildDir%\silohdf5.dll .
if not exist .\hdf5dll.dll   copy %HDF5_LIB_DIR%\hdf5dll.dll .
if not exist .\zlib1.dll     copy %ZLIB_LIB_DIR%\zlib1.dll .
if not exist .\szlibdll.dll  copy %SZIP_LIB_DIR%\szlibdll.dll .

if exist %resultsfile% (del %resultsfile%)


for %%z in %drivers% do (
    for %%v in %gp% do (
        if exist %%v (
            echo Running %%v %%z 
            echo Running %%v %%z >> %resultsfile%
            %%v %%z > %errorfile% 2>&1
            type %errorfile% >> %resultsfile% 
            echo exit code: %errorlevel% >> %resultsfile%
            echo ============================================== >> %resultsfile%
            echo ============================================== >> %resultsfile%
        )
    )

    REM tests that require extra arguments
    if exist testall.exe (
        for %%i in (small medium large fortran c) do (
            echo Running testall.exe -%%i %%z 
            echo Running testall.exe -%%i %%z >> %resultsfile%
            testall.exe -%%i %%z > %errorfile% 2>&1
            type %errorfile% >> %resultsfile% 
            echo exit code: %errorlevel% >> %resultsfile%
            echo ============================================== >> %resultsfile%
            echo ============================================== >> %resultsfile%
        )
    )

    if exist group_test.exe (
        echo Running group_test.exe 0 0 0 %%z 
        echo Running group_test.exe 0 0 0 %%z >> %resultsfile%
        group_test.exe 0 0 0 %%z > %errorfile% 2>&1
        type %errorfile% >> %resultsfile% 
        echo exit code: %errorlevel% >> %resultsfile%
        echo ============================================== >> %resultsfile%
        echo ============================================== >> %resultsfile%
    )

)


if exist listtypes.exe (
    for %%i in (ucd.h5 ucd.pdb) do (
        if exist %%i (
            echo Running listtypes.exe %%i 
            echo Running listtypes.exe %%i >> %resultsfile%
            listtypes.exe %%i > %errorfile% 2>&1
            type %errorfile% >> %resultsfile% 
            echo exit code: %errorlevel% >> %resultsfile%
            echo ============================================== >> %resultsfile%
            echo ============================================== >> %resultsfile%
        )
    )
)



:: PDB Specific
if exist testpdb.exe (
    echo Running testpdb.exe  
    echo Running testpdb.exe  >> %resultsfile%
    testpdb.exe > %errorfile% 2>&1
    type %errorfile% >> %resultsfile% 
    echo exit code: %errorlevel% >> %resultsfile%
    echo ============================================== >> %resultsfile%
    echo ============================================== >> %resultsfile%
)

:: HDF5 Specific
if exist compression.exe (
    for %%i in (gzip szip fpzip lossy3 minratio1000) do (
        echo Running compression.exe %%i 
        echo Running compression.exe %%i >> %resultsfile%
        compression.exe %%i > %errorfile% 2>&1
        type %errorfile% >> %resultsfile% 
        echo exit code: %errorlevel% >> %resultsfile%
        echo ============================================== >> %resultsfile%
        echo ============================================== >> %resultsfile%
    )
)

if exist grab.exe (
    echo Running grab.exe  
    echo Running grab.exe  >> %resultsfile%
    grab.exe > %errorfile% 2>&1
    type %errorfile% >> %resultsfile% 
    echo exit code: %errorlevel% >> %resultsfile%
    echo ============================================== >> %resultsfile%
    echo ============================================== >> %resultsfile%
)

REM if exist largefile.exe (
REM    echo Running largefile.exe  >> %resultsfile%
REM    largefile.exe > %errorfile% 
REM    type %errorfile% >> %resultsfile% 
REM    echo exit code: %errorlevel% >> %resultsfile%
REM    echo ============================================== >> %resultsfile%
REM    echo ============================================== >> %resultsfile%
REM )

REM Fortran tests
set ft=(array_f.exe curve_f.exe mat_f.exe point_f.exe quad_f.exe testall_f.exe ucd_f.exe)
for %%v in %ft% do (
    if exist %%v (
        echo Running %%v
        echo Running %%v >> %resultsfile%
        %%v > %errorfile% 2>&1
        type %errorfile% >> %resultsfile% 
        echo exit code: %errorlevel% >> %resultsfile%
        echo ============================================== >> %resultsfile%
        echo ============================================== >> %resultsfile%
    )
)


::print results
::type %resultsfile%

echo all output has been directed to the file results.txt

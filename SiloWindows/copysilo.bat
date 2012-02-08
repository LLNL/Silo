@echo off

set SiloIn=.\\..\\src\\silo\\silo.h.in
set SiloOut=.\\include\\silo.h
set VersionOut=.\\include\\siloversion.h
set major=0
set minor=0
set patch=
set pre=
set versionfile=.\\..\\VERSION
set dotpatch=
set dotpre=
set underpatch=
set underpre=

REM Retrieve and parse the version tokens
findstr ".*\..*\..*-pre.*" %versionfile% > Nul
if %errorlevel%==0 (
    REM "found Major.Minor.Patch-Pre: major.minor.patch-pre"
    for /F "tokens=1,2,3* delims=.,-pre" %%i in (%versionfile%) do (
        Set major=%%i
        Set minor=%%j
        Set patch=%%k
        Set pre=%%l
        set dotpatch=.%%k
        set dotpre=-pre%%l
        set underpatch=_%%k
        set underpre=_pre%%l
    ) 
    goto parsedversion
) 

findstr  ".*\..*\..*" %versionfile% > Nul
if %errorlevel%==0 (
    REM "found Major.Minor.Patch: major.minor.patch"
    for /F "tokens=1,2,3* delims=.,-pre" %%i in (%versionfile%) do (
        Set major=%%i
        Set minor=%%j
        Set patch=%%k
        set dotpatch=.%%k
        set underpatch=_%%k
    ) 
    goto parsedversion
)

findstr ".*\..*-pre.*" %versionfile% > Nul
if %errorlevel%==0 (
    REM "found Major.Minor-Pre: major.minor-pre"
    for /F "tokens=1,2,3* delims=.,-pre" %%i in (%versionfile%) do (
        Set major=%%i
        Set minor=%%j
        Set pre=%%k
        set dotpre=-pre%%k
        set underpre=_pre%%k
    ) 
    goto parsedversion
)

findstr  ".*\..*" %versionfile% > Nul
if %errorlevel%==0 (
    REM "found Major.Minor: major.minor"
    for /F "tokens=1,2,3* delims=.,-pre" %%i in (%versionfile%) do (
    Set major=%%i
    Set minor=%%j
    ) 
    goto parsedversion
)

:parsedversion

if exist %SiloOut% (
  del %SiloOut%
)

if exist %VersionOut% (
  del %VersionOut%
)

REM Read silo.h.in, parsing for VERS info, and substituting in appropriate values
for /F "tokens=1* delims=]" %%i in ('find /v /n "" ^.\..\src\silo\silo.h.in') do (
  REM preserve blank lines
  if "%%j"=="" (
    @echo.>>%SiloOut%
  ) else (
    REM search the input line for special tokens
    for /F "tokens=1,2,3* delims=@" %%A in ("%%j") do (
      :: @echo %%A%%B%%C%%D>>mytest.txt 
      if "%%B"=="SILO_VERS_MAJ" (
        @echo %%A%major%>> %SiloOut% 
      ) else if "%%B"=="SILO_VERS_MIN" (
        @echo %%A%minor%>> %SiloOut% 
      ) else if "%%B"=="SILO_VERS_PAT" (
        @echo %%A%patch%>> %SiloOut% 
      ) else if "%%B"=="SILO_VERS_PRE" (
        @echo %%A%pre%>> %SiloOut% 
      ) else if "%%B"=="SILO_VERS_TAG" (
        @echo %%A Silo_version_%major%_%minor%%underpatch%%underpre%>> %SiloOut%
      ) else if "%%B"=="SILO_DTYPPTR" (
          @echo %%Avoid%%C>> %SiloOut%
      ) else if "%%B"=="SILO_DTYPPTR1" (
          @echo %%Avoid*%%C>> %SiloOut%
      ) else if "%%B"=="SILO_DTYPPTR2" (
          @echo %%Avoid*%%C>> %SiloOut%
      ) else (
        if not "%%C"=="" (
          @echo %%A%%B%%C>> %SiloOut%
        ) else if not "%%B"=="" (
          @echo %%A%%B>> %SiloOut%
        ) else (
          @echo %%A>> %SiloOut%
        )
      )
    )
  )
)


@echo #define PACKAGE_STRING "silo %major%.%minor%%dotpatch%%dotpre%" >> %VersionOut% 
@echo #define PACKAGE_VERSION "%major%.%minor%%dotpatch%%dotpre%" >> %VersionOut% 
@echo #define SILO_VERSION "%major%.%minor%%dotpatch%%dotpre%" >> %VersionOut% 


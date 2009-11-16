@echo off

set SiloIn=..\\..\\src\\silo\\silo.h.in
set SiloOut=..\\..\\src\\silo\\silo.h
set major=0
set minor=0
set patch=0
set pre=

REM Retrieve and parse the version tokens
for /F "tokens=1,2,3* delims=.,-pre" %%i in (..\\..\\VERSION) do (
  Set major=%%i
  Set minor=%%j
  Set patch=%%k
  Set pre=%%l
)

REM Read silo.h.in, parsing for VERS info, and substituting in appropriate values
for /F "tokens=1* delims=]" %%i in ('find /v /n "" ^..\..\src\silo\silo.h.in') do (
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
        if "%pre%"=="" (
          @echo %%A Silo_version_%major%_%minor%_%patch%>> %SiloOut% 
        ) else (
          @echo %%A Silo_version_%major%_%minor%_%patch%_pre%pre%>> %SiloOut% 
        )
      ) else if "%%B"=="SILO_DTYPPTR" (
          @echo %%A@%%B@%%C>> %SiloOut%
      ) else if "%%B"=="SILO_DTYPPTR1" (
          @echo %%A@%%B@%%C>> %SiloOut%
      ) else if "%%B"=="SILO_DTYPPTR2" (
          @echo %%A@%%B@%%C>> %SiloOut%
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

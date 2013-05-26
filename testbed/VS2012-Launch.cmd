@ECHO OFF
SETLOCAL

IF "%PROCESSOR_ARCHITECTURE%"=="x86" (
	REG QUERY HKLM\Software\WOW6432Node\Microsoft\VisualStudio\10.0 /v InstallDir >nul 2>&1 && (
		FOR /F "tokens=2,*" %%I IN ('REG QUERY HKLM\Software\WOW6432Node\Microsoft\VisualStudio\11.0 /v InstallDir ^|FINDSTR InstallDir') DO (
			SET VSPATH=%%J
		)
		GOTO :FoundDevStudio
	)
) ELSE (
	REG QUERY HKLM\Software\WOW6432Node\Microsoft\VisualStudio\10.0 /v InstallDir >nul 2>&1 && (
		FOR /F "tokens=2,*" %%I IN ('REG QUERY HKLM\Software\WOW6432Node\Microsoft\VisualStudio\11.0 /v InstallDir ^|FINDSTR InstallDir') DO (
			SET VSPATH=%%J
		)
		GOTO :FoundDevStudio
	)
)

ECHO Unable to find Visual Studio 2012 in your registry.
PAUSE
EXIT

:FoundDevStudio

SET VSEXE="%VSPATH%devenv.exe"
SET VSVARS="%VSPATH%..\..\VC\bin\x86_amd64\vcvarsx86_amd64.bat

CALL %VSVARS%

REM ======= Start VS2010 asynchronously =========
START "" %VSEXE% tundra-output\tundra-generated.sln

REM ======= Wait 3 seconds, hopefully the VS window has appeared by then =========
REM Without this delay, the VS window will be opened in the background
ping 127.0.0.1 -n 6 -w 1000 >NUL


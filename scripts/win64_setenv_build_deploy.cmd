@echo off
CALL scripts\vcvarsx86_amd64.bat
bin\win32\tundra2 -c win64-msvc-release
bin\win32\tundra2 win64-msvc-release
python scripts\package_windows_build.py --file c:\users\emoon\Dropbox\Public\prodbg_windows.zip

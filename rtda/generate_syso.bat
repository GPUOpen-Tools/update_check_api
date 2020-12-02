@ECHO OFF

REM SET PATH=C:\TDM-GCC-64\bin;%PATH%
SET PATH=C:\MinGW\bin;%PATH%

windres -i rtda.rc -O coff -o versioninfo.syso

PAUSE
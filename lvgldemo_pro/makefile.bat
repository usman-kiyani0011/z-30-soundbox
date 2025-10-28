@echo off
set "lj=%~p0"
set "lj=%lj:~0,-1%"
for %%a in ("%lj%") do set wjj=%%~nxa
echo app:%wjj%
cd ..\build

call mr_et389pro_app.bat %wjj%

pause


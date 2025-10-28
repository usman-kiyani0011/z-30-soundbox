@echo off
set "lj=%~p0"
set "lj=%lj:\= %"
for %%a in (%lj%) do set wjj=%%a
echo app:%wjj%
cd ..\build

call mr_et389pro_app.bat %wjj%

pause


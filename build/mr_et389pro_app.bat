set lib=et389pro_lib
call SetPathEC100Y.bat

echo off
rem echo %1

set name1=%1
set appname=%1
echo %appname%


SET APPPARM=-i APP2IMG -a 0x60370000 -s 0xF0000



echo on

cd ..\..\
SET ENV_PATH=apps/build
SET DRIVER_PATH=sdk/%lib%
SET APP_NAME=%appname%
SET ELF_PATH=apps/%appname%/dev.elf
SET IMG_PATH=apps/%appname%/dev.img
SET IMG_PATH_SIG=apps/%appname%/dev.img.sig
SET APP_PATH=apps/%appname%/app.bin
SET FOTA_PATH=apps/%appname%/fota.bin
SET GCC_PATH=tools/compile_tools_ec600u


make -f apps\build\makefile_app  PROJECT_NAME=%appname% || pause
tools\compile_tools_ec600u\tools\win32\dtools.exe mkappimg %ELF_PATH% %IMG_PATH%
tools\compile_tools_ec600u\tools\win32\vlrsign.exe --pw 12345678 --pn test --ha Blake2 --img  %IMG_PATH% --out %IMG_PATH_SIG%                         
tools\compile_tools_ec600u\python3\python3.exe tools\compile_tools_ec600u\tools\pacgen.py cfg-init --pname UIX8910_MODEM --palias 8915DM_cat1_open --pversion "8910 MODULE" --version BP_R1.0.0 --flashtype 1 cfg-host-fdl -a 0x8000c0 -s 0xff40 -p sdk\%lib%\fdl1.sign.img cfg-fdl2 -a 0x810000 -s 0x30000 -p sdk\%lib%\fdl2.sign.img cfg-image %APPPARM% -p %IMG_PATH_SIG% pac-gen %APP_PATH%"
tools\compile_tools_ec600u\tools\win32\dtools.exe fotacreate2 --single-pac %APP_PATH%,sdk\%lib%\fota8910.xml %FOTA_PATH% -d v


cd apps\%appname%
..\build\mz01a_ota.exe fota.bin YQ06B- 2
echo off
del /f/q Release
md Release
copy app.bin Release\
copy fota.bin Release\ 
copy ota.bin Release\
rar a -r -inul -df Release Release
rename Release.rar Release

del /f/q app.bin dev.elf dev.img dev.img.sig ec600u.bin
rename fota.bin app.bin
@rem ..\build\mz01a_sig.exe ota.bin




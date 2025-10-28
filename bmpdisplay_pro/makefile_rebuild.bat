@echo off
del /f/q ..\..\sdk\et389pro_lib\dev.map
rd /s/q ..\..\sdk\et389pro_lib\dep
rd /s/q ..\..\sdk\et389pro_lib\obj
echo del succ

call makefile.bat
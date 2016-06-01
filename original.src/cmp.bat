@echo off

cls
del *.obj
del err.txt

del D:\sphinxC\testC--\c--.exe

SET PATH=D:\WATCOM\BINW;%PATH%
SET PATH=D:\WATCOM\BINNT;%PATH%
SET INCLUDE=D:\WATCOM\H\NT;%INCLUDE%
SET INCLUDE=D:\WATCOM\H\NT;%INCLUDE%
SET INCLUDE=%INCLUDE%;D:\WATCOM\H\NT\DIRECTX
SET INCLUDE=%INCLUDE%;D:\WATCOM\H\NT\DDK
SET INCLUDE=D:\WATCOM\H;%INCLUDE%
SET WATCOM=D:\WATCOM
SET EDPATH=D:\WATCOM\EDDAT
SET WHTMLHELP=D:\WATCOM\BINNT\HELP
SET WIPFC=D:\WATCOM\WIPFC



wmake -f makefile.nt  

copy D:\sphinxC\vcSrc\c--.exe  D:\sphinxC\testC--\*.*
del D:\sphinxC\vcSrc\c--.exe
pause

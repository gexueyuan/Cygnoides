fromelf.exe --bin -o ..\bin\app.bin .\build\Cygnoides.axf
::C:\Keil\ARM\ARMCC\bin\fromelf.exe -c -s -o ..\bin\app.lst build\Cygnoides.axf

copy ..\tools\convertforboot.exe  ..\bin\convertforboot.exe
cd ..\bin
::               source file  targer file  platform sgn  file type load address exec address 
convertforboot.exe   app.bin app  "CV1"  "FW"  08020200  08020200
del convertforboot.exe



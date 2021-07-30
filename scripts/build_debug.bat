@echo off
set /p file= "Please enter a file to assemble: "
echo "Assembling %file%..."
cd ..\.build
qcpu-d.exe ..\asm\%file%.asm ..\programs\%file%
echo "Program assembled at: ..\programs\%file%"
pause
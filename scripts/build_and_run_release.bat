@echo off
set /p file= "Please enter a file to assemble: "
echo "Assembling %file%..."
cd ..\.build
qcpu.exe ..\asm\%file%.asm ..\programs\%file%
echo "Running Program at: ..\programs\%file%"
qcpu.exe ..\programs\%file%
pause
@echo off
cls
:start
CraftServices.exe --ports com20 --baud 9600 --refresh 50 --phantomwingman com20,180,50,10 --loglevel info --exitgpsloss
echo re-running CraftServices..
goto start
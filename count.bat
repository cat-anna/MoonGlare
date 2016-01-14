@echo off
echo. >> lines.txt
echo. >> lines.txt
echo date | bash >> lines.txt
cloc --quiet . >> lines.txt

npp lines.txt
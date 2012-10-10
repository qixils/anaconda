@ECHO OFF
pushd "%CD%"
IF "%HASVCVARSALL%" == "1" GOTO NOVCVARSALL
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set HASVCVARSALL=1
:NOVCVARSALL
python -m chowdren.run "Knytt Underground.exe" test2
if %errorlevel% neq 0 popd && exit /b %errorlevel%
cd test2\build
devenv Chowdren.sln /Build MinSizeRel
if %errorlevel% neq 0 popd && exit /b %errorlevel%
cd ..
build\MinSizeRel\Chowdren.exe
popd
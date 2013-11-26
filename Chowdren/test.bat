@ECHO OFF
pushd "%CD%"
IF "%HASVCVARSALL%" == "1" GOTO NOVCVARSALL
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set HASVCVARSALL=1
:NOVCVARSALL
python -m chowdren.run "KUSteam.exe" knyttsource --noimages
if %errorlevel% neq 0 popd && exit /b %errorlevel%
cd knyttsource\build
devenv Chowdren.sln /Build Release
if %errorlevel% neq 0 popd && exit /b %errorlevel%
cd ..
build\Release\Chowdren.exe
popd
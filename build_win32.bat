set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community"
set QT_PATH=C:\Qt\5.12.8\msvc2017
set SEVENZIP_PATH="C:\Program Files\7-Zip"

set BUILD_NAME=xelfviewer_win32_portable
set SOURCE_PATH=%~dp0
mkdir %SOURCE_PATH%\build
mkdir %SOURCE_PATH%\build\loader
mkdir %SOURCE_PATH%\release
set /p RELEASE_VERSION=<%SOURCE_PATH%\release_version.txt

set QT_PATH=%QT_PATH%
call %VS_PATH%\VC\Auxiliary\Build\vcvars32.bat
set GUIEXE=xelfviewer.exe
set ZIP_NAME=%BUILD_NAME%_%RELEASE_VERSION%
set RES_FILE=rsrc

cd gui_source
%QT_PATH%\bin\qmake.exe gui_source.pro -r -spec win32-msvc "CONFIG+=release"

nmake Makefile.Release clean
nmake
del Makefile
del Makefile.Release
del Makefile.Debug

cd ..

cd winloader_source

cl.exe /c winloader.cpp /D_USING_V110_SDK71_ /GS- /Oi-
Rc.exe /v %RES_FILE%.rc
link.exe winloader.obj %RES_FILE%.res user32.lib kernel32.lib shell32.lib /NODEFAULTLIB /SAFESEH:NO /SUBSYSTEM:WINDOWS,5.01 /ENTRY:entry /OUT:%SOURCE_PATH%\build\loader\%GUIEXE%
del /s winloader.obj
del /s %RES_FILE%.res

cd ..

mkdir %SOURCE_PATH%\release\%BUILD_NAME%
mkdir %SOURCE_PATH%\release\%BUILD_NAME%\base
mkdir %SOURCE_PATH%\release\%BUILD_NAME%\base\platforms

copy %SOURCE_PATH%\build\loader\%GUIEXE% %SOURCE_PATH%\release\%BUILD_NAME%\
copy %SOURCE_PATH%\build\release\%GUIEXE% %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\bin\Qt5Widgets.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\bin\Qt5Gui.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\bin\Qt5Core.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\bin\Qt5OpenGL.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\bin\Qt5Svg.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %QT_PATH%\plugins\platforms\qwindows.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\platforms\

copy %VS_PATH%\VC\Redist\MSVC\14.16.27012\x86\Microsoft.VC141.CRT\msvcp140.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\
copy %VS_PATH%\VC\Redist\MSVC\14.16.27012\x86\Microsoft.VC141.CRT\vcruntime140.dll %SOURCE_PATH%\release\%BUILD_NAME%\base\

cd %SOURCE_PATH%\release
if exist %ZIP_NAME%.zip del %ZIP_NAME%.zip
%SEVENZIP_PATH%\7z.exe a %ZIP_NAME%.zip %BUILD_NAME%\*
rmdir /s /q %SOURCE_PATH%\release\%BUILD_NAME%
cd ..
pushd \
call C:\Qt\Qt5.3.0\5.3\mingw482_32\bin\qtenv2.bat
popd

cd ..
cd plugins

cd BitOp
cd source
qmake bitop.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd LuaSQLite3
cd source
qmake lsqlite3.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd LuaSocket
cd source
qmake luasocket.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd LPeg
cd source
qmake lpeg.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd LuaFileSystem
cd source
qmake lfs.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd Microphone
cd source
cd Desktop
qmake microphone.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..\..

cd JSON
cd source
qmake json.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..\..

cd ..
cd scripts

@echo off
echo Building game client...
cd gameclient
make -f Build.make building

if errorlevel 1 (
    echo Failed to build game client
    exit /b 1
)
cd ..

echo Building game server...

cd gameserver

make -f Build.make building

if errorlevel 1 (
    echo Failed to build game server
    exit /b 1
)

cd ..

echo Build completed successfully!
echo Client executable: gameclient/build/game.exe
echo Server executable: gameserver/build/server.exe 

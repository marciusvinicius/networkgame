echo Building game client...
cd gameclient

make -f Build.make building

cd ..

echo Building game server...

cd gameserver

make -f Build.make building

cd ..

echo Build completed successfully!

echo Client executable: gameclient/build/game
echo Server executable: gameserver/build/server

./gameclient/build/game && ./gameserver/build/server

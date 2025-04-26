building:
	build.bat
run: building
	./gameserver/build/server.exe && ./gameclient/build/game.exe

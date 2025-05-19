building:
	./build.sh
run: building
	gameserver/build/server && gameclient/build/game

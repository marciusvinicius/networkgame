building:
	zig cc -o build/game.exe src/network.c src/main.c -target x86_64-windows -I"C:\raylib\include" -L"C:\raylib\lib"  -LW:\enet -IW:\enet\include -lws2_32 -lwinmm -lenet -lenet64 -lraylib
	@echo Building done

run:building
	./build/game.exe

clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done

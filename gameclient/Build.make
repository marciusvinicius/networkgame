building:
	zig cc -o build/game src/network.c src/main.c  -I"/home/marcius/Workspace/opensource/raylib/include" -L"/home/marcius/Workspace/opensource/raylib/lib"  -L/home/marcius/Workspace/opensource/enet -I/home/marcius/Workspace/opensource/enet/include -lenet -lraylib
	@echo Building done

run:building
	./build/game

clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done

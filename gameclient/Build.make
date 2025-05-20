building:
	gcc -o build/game src/network.c src/main.c  -I"/home/marcius/Workspace/opensource/raylib/include"  -L/home/marcius/Workspace/opensource/enet -I/home/marcius/Workspace/opensource/enet/include -I../../common/src -lenet -lraylib -lm -g
	@echo Building done

run:building
	./build/game

clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done

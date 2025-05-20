building:
	gcc -o build/server src/server.c src/main.c -L/home/marcius/Workspace/opensource/enet -I/home/marcius/Workspace/opensource/enet/include -I../common/src -lenet -g
	@echo Building done
run:building
	./build/server
clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done

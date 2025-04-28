building:
	zig cc -o build/server.exe src/server.c src/main.c -target x86_64-windows -LW:\enet -IW:\enet\include -lws2_32 -lwinmm -lenet -lenet64
	@echo Building done

run:building
	./build/server.exe

clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done

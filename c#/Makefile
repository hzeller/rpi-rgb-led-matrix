all:
	mkdir -p bin
	cp ../lib/librgbmatrix.so.1 ./bin/librgbmatrix.so
	mcs -target:library -out:bin/RGBLedMatrix.dll *.cs
	mcs -r:./bin/RGBLedMatrix.dll -out:bin/testapp.exe ./testapp/Program.cs
	
clean: @ -rm -f ./bin

remake: @ $(MAKE) clean
	@ $(MAKE)
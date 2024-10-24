start: EditorBMP.o
	g++ EditorBMP.o -o start.app

LabaAi.o: EditorBMP.cpp
	g++ -c Editor.cpp

clean:
	rm *.o start.app


#target: dependencies
#	action

# From: https://www.youtube.com/watch?v=r7i5XorXJk

output: Main.o SFApp.o SFAsset.o SFBoundingBox.o SFEvent.o SFWindow.o
	g++ Main.o SFApp.o SFAsset.o SFBoundingBox.o SFEvent.o SFWindow.o -o output
	
Main.o: Main.cpp
	g++ -c -std=c++11 Main.cpp
	
SFApp.o: SFApp.cpp SFApp.h
	g++ -c -std=c++11 SFApp.cpp 
	
SFAsset.o: SFAsset.cpp SFAsset.h
	g++ -c -std=c++11 SFAsset.cpp
	
SFBoundingBox.o: SFBoundingBox.cpp SFBoundingBox.h
	g++ -c -std=c++11 SFBoundingBox.cpp
	
SFEvent.o: SFEvent.cpp SFEvent.h
	g++ -c -std=c++11 SFEvent.cpp
	
SFWindow.o: SFWindow.cpp SFWindow.h
	g++ -c -std=c++11 SFWindow.cpp	
	
library: 
	g++ -o StarshipFontana *.o -lSDL2 -lSDL2_image
	
clean:
	rm *.o output

	


build:
	g++ -o tema1 main.cpp -lpthread       \
		SharedVariables.cpp WordList.cpp
clean:
	rm -rf ./tema1 [a-z].txt word-list.txt

archive:
	rm -f "$(HOME)/Downloads/APD-Tema1.zip"
	zip "$(HOME)/Downloads/APD-Tema1.zip" *.cpp *.h README.md Makefile

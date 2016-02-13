all:
	g++ bolgefuck.cpp sha256.cpp -std=c++11 -o bf
	./bf examples
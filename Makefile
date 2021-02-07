build: compile
	g++ ovr2shp.o hfaclasses.o ./hfa/*.o ./port/*.o -lm -lpthread -o ovr2shp

compile: ovr2shp.cpp hfaclasses.cpp
	g++ -c -O -I./hfa -I./port hfaclasses.cpp
	g++ -c -O -I./hfa -I./port ovr2shp.cpp

build_dep:
	./build_dep.sh

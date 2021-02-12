build: compile
	g++ ovr2shp.o hfaclasses.o ./hfa/*.o ./port/*.o -lm -lpthread -L/usr/local/lib -lboost_iostreams -lboost_system -lboost_filesystem -o ovr2shp

compile: ovr2shp.cpp hfaclasses.cpp
	g++ -c -O -I./hfa -I./port hfaclasses.cpp
	g++ -c -O -I./hfa -I./port -std=c++17 -L/usr/local/lib -lboost_iostreams -lboost_system -lboost_filesystem ovr2shp.cpp

build_dep:
	./build_dep.sh

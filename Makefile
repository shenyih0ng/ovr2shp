GDAL_ROOT = $(HOME)/Desktop/gdal
BOOST_LIB = /usr/local/lib

build: compile
	g++ ovr2shp.o hfaclasses.o ./hfa/*.o -L$(BOOST_LIB) -L$(GDAL_ROOT)/lib -lm -lpthread -lboost_iostreams -lboost_system -lboost_filesystem -lgdal -o ovr2shp

compile: ovr2shp.cpp hfaclasses.cpp
	g++ -c -O -I./hfa -I$(GDAL_ROOT)/include hfaclasses.cpp
	g++ -c -O -I./hfa -I$(GDAL_ROOT)/include -std=c++17 ovr2shp.cpp

build_dep:
	./build_dep.sh

build: compile
	g++ ovr2shp.o hfaclasses.o hfasrs.o ./hfa/*.o -lm -lpthread -lgdal -o ovr2shp

build-gnuplot: compile-gplot
	g++ ovr2shp.o hfaclasses.o hfasrs.o ./hfa/*.o -lm -lpthread -lboost_iostreams -lboost_system -boost_filesystem -lgdal -o ovr2shp

compile: ovr2shp.cpp hfaclasses.cpp hfasrs.cpp build-dep
	g++ -c -O -I./hfa ovr2shp.cpp hfaclasses.cpp hfasrs.cpp

compile-gplot: ovr2shp.cpp hfaclasses.cpp hfasrs.cpp build-dep
	g++ -c -O -I./hfa -DGPLOT ovr2shp.cpp hfaclasses.cpp hfasrs.cpp

build-dep:
	./build_dep.sh

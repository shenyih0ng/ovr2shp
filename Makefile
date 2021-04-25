CXX := g++
CXXFLAGS := --std=c++17 -lm -lpthread -lgdal 
CXXFLAGS_GNUPLOT := ${CXXFLAGS} -lboost_iostreams -lboost_system -lboost_filesystem
INCLUDES := -I./hfa

OBJECTS := ./hfa/*.o ovr2shp.cpp hfaclasses.cpp hfasrs.cpp 

build: ${OBJECTS} 
	${CXX} ${OBJECTS} ${INCLUDES} ${CXXFLAGS} -o ovr2shp

build-gnuplot: ${OBJECTS}
	${CXX} ${OBJECTS} ${INCLUDES} ${CXXFLAGS_GNUPLOT} -DGPLOT -o ovr2shp

CXX := g++
CXXFLAGS := --std=c++17 -lm -lpthread -lgdal 
CXXFLAGS_GNUPLOT := ${CXXFLAGS} -lboost_iostreams -lboost_system -lboost_filesystem
INCLUDES := -I./hfa

OBJECTS := ./hfa/*.o ovr2shp.cpp hfaclasses.cpp hfasrs.cpp 

build: ${OBJECTS} 
	${CXX} ${INCLUDES} ${CXXFLAGS} ${OBJECTS} -o ovr2shp

build-gnuplot: ${OBJECTS}
	${CXX} ${INCLUDES} ${CXXFLAGS_GNUPLOT} -DGPLOT ${OBJECTS} -o ovr2shp

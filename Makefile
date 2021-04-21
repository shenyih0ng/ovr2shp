CXX := g++
CXXFLAGS := -lm -lpthread -lgdal 
CXXFLAGS_GNUPLOT := --std=c++17 ${CXXFLAGS} -lboost_iostreams -lboost_system -lboost_filesystem
INCLUDES := -I./hfa

OBJECTS := ./hfa/*.o ovr2shp.cpp hfaclasses.cpp hfasrs.cpp 

build: ${OBJECTS} 
	${CXX} ${INCLUDES} ${CXXFLAGS} ${OBJECTS} -o ovr2shp

build-gnuplot: ${OBJECTS}
	${CXX} ${INCLUDES} ${CXXFLAGS_GNUPLOT} -DGPLOT ${OBJECTS} -o ovr2shp

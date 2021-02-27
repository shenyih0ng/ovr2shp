FROM osgeo/gdal:ubuntu-small-latest

RUN apt-get update && apt-get -y install g++ make vim

WORKDIR /ovr2shp

COPY ./hfa ./hfa
COPY ./hfaclasses.cpp ./hfasrs.cpp ./ovr2shp.cpp ./ovr2shp.h ./Makefile ./build_dep.sh ./

ENTRYPOINT ["/bin/bash"]

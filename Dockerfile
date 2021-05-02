FROM osgeo/gdal:ubuntu-small-latest

RUN apt-get update && apt-get -y install g++ make vim

WORKDIR /ovr2shp

COPY ./hfa ./hfa
COPY ./hfaclasses.cpp ./hfasrs.cpp ./ovr2shp.cpp ./ovr2shp.h ./logging.h ./Makefile ./build_dep.sh ./

RUN /ovr2shp/build_dep.sh
RUN make -f /ovr2shp/Makefile build

ENTRYPOINT ["/bin/bash"]
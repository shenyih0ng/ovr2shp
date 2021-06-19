# ovr2shp

A project that helps convert ERDAS HFA Annotation Layer (`.ovr`) to Shapefile(`.shp`)/GeoJSON(`.geojson`). To understand how ERDAS HFA file format works, I looked into how GDAL implements the conversion from ERDAS HFA Raster Format (`.img`) to `.tif`.

ERDAS also provided a detailed documentation explaining the internals of HFA structure. Although it details mainly on the raster `.img` format, `.ovr` follows the same file structure described in the documentation.


## Development

**Details of setting up `ovr2shp` for development can be found [here](https://shenyih0ng.github.io/ovr2shp/)**

[gnuplot](http://www.gnuplot.info/) is used in the linux build to visualize the geometries/shape extracted from  `.ovr` files. [Gnuplot-Iostream Interface](https://github.com/dstahlke/gnuplot-iostream) is used to interface with the `gnuplot` binary.

## Prebuilt binaries

- [`v0.1.0`](https://github.com/shenyih0ng/ovr2shp/releases/tag/v0.1.0)
	- `win-x86`
	- `win-x64`
### References

[img2tif](http://web.archive.org/web/20130730133056/http://home.gdal.org/projects/imagine/hfa_index.html)

[Detail Documentation](/docs/hfa.pdf)

[Sample Data](https://download.hexagongeospatial.com/en/downloads/imagine/erdas-imagine-remote-sensing-example-data)

# ovr2shp

A project that helps convert ERDAS HFA Annotation Layer (`.ovr`) to Shapefile(`.shp`)/GeoJSON(`.geojson`). To understand how ERDAS HFA file format works, I looked into how GDAL implements the conversion from ERDAS HFA Raster Format (`.img`) to `.tif`.

ERDAS also provided a detailed documentation explaining the internals of HFA structure. Although it details mainly on the raster `.img` format, `.ovr` follows the same file structure described in the documentation.

## TODOS

- [ ] Generalize `get_ellipse` & `get_rectangle` functions

- [ ] Extract coordinate reference system if exist

- [ ] Shapefile+GeoJSON creation

### References

[img2tif]: http://web.archive.org/web/20130730133056/http://home.gdal.org/projects/imagine/hfa_index.html

[Detail Documentation]: /docs/hfa.pdf 

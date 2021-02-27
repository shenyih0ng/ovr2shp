(window.webpackJsonp=window.webpackJsonp||[]).push([[16],{137:function(e,t,a){"use strict";a.r(t),t.default=a.p+"assets/images/hfastructure-39ee10b5cb1230b9081faba4e8fab110.png"},138:function(e,t,a){"use strict";a.r(t),t.default=a.p+"assets/images/mif-736e263da9a70595be977e805350e79d.png"},84:function(e,t,a){"use strict";a.r(t),a.d(t,"frontMatter",(function(){return r})),a.d(t,"metadata",(function(){return o})),a.d(t,"toc",(function(){return l})),a.d(t,"default",(function(){return c}));var n=a(3),i=(a(0),a(95));const r={slug:"what-is-hfa",title:"Parsing .ovr",author:"shenyihong",author_title:"Maintainer of ovr2shp",author_url:"https://github.com/shenyih0ng",tags:["hfa","ovr"]},o={permalink:"/ovr2shp/what-is-hfa",editUrl:"https://github.com/shenyih0ng/ovr2shp/tree/gh-pages/blog/what-is-hfa.md",source:"@site/blog\\what-is-hfa.md",description:'Hierarchical File Architecture(HFA) is Hexagon Geospatial\'s proprietary file format. It is the file format behind ERDAS Imagine .img files and ERDAS Imagine Annotation Layer .ovr files. It is not exactly the most "open" file standard out there, but luckily GDAL implements a raster driver for.img files, which offers an glimpse into the internals of HFA.',date:"2021-02-22T14:49:17.326Z",tags:[{label:"hfa",permalink:"/ovr2shp/tags/hfa"},{label:"ovr",permalink:"/ovr2shp/tags/ovr"}],title:"Parsing .ovr",readingTime:5.585,truncated:!0},l=[{value:"img2tif",id:"img2tif",children:[]},{value:"Hierarchical File Architecture (HFA)",id:"hierarchical-file-architecture-hfa",children:[{value:"Header Tag",id:"header-tag",children:[]},{value:"Header Record",id:"header-record",children:[]},{value:"MIF Dictionary",id:"mif-dictionary",children:[]},{value:"Root Node",id:"root-node",children:[]},{value:"Ehfa_Entry",id:"ehfa_entry",children:[]}]}],p={toc:l};function c({components:e,...t}){return Object(i.b)("wrapper",Object(n.a)({},p,t,{components:e,mdxType:"MDXLayout"}),Object(i.b)("p",null,"Hierarchical File Architecture(HFA) is ",Object(i.b)("a",{parentName:"p",href:"https://www.hexagongeospatial.com/"},"Hexagon Geospatial's")," proprietary file format. It is the file format behind ERDAS Imagine ",Object(i.b)("inlineCode",{parentName:"p"},".img")," files and ERDAS Imagine Annotation Layer ",Object(i.b)("inlineCode",{parentName:"p"},".ovr"),' files. It is not exactly the most "open" file standard out there, but luckily ',Object(i.b)("a",{parentName:"p",href:"https://github.com/OSGeo/gdal"},"GDAL")," implements a raster driver for",Object(i.b)("inlineCode",{parentName:"p"},".img")," files, which offers an glimpse into the internals of HFA. "),Object(i.b)("p",null,"Now that we know that a open-sourced driver that can parse a file with the HFA format exists, we can use that as our starting point."),Object(i.b)("blockquote",null,Object(i.b)("p",{parentName:"blockquote"},"My Goal: ",Object(i.b)("strong",{parentName:"p"},"Convert Erdas Imagine Annotation Layer(.ovr) to ShapeFile(.shp)"))),Object(i.b)("p",null,"The problem, however, is that GDAL is bulky and I have limited experience dealing with big c++ projects. Since we would only need specific files to parse HFA compliant files, I went digging for something more lightweight. I managed to find a web archive page that contained a early version of a ",Object(i.b)("inlineCode",{parentName:"p"},".img")," to ",Object(i.b)("inlineCode",{parentName:"p"},".tif")," converter. You can find the page ",Object(i.b)("a",{parentName:"p",href:"http://web.archive.org/web/20130730133056/http://home.gdal.org/projects/imagine/hfa_index.html"},"here"),"."),Object(i.b)("p",null,"This page also turns out to be the holy grail as it also referenced a ",Object(i.b)("a",{parentName:"p",href:"https://github.com/shenyih0ng/ovr2shp/blob/dev/docs/hfa.pdf"},"detailed documentation")," of ",Object(i.b)("inlineCode",{parentName:"p"},".img")," file format and the internals of HFA"),Object(i.b)("h2",{id:"img2tif"},"img2tif"),Object(i.b)("p",null,"The source code included drivers for HFA and GeoTIFF, but there is just one caveat: The HFA driver included is not a general implementation but rather one that is specifically made for ",Object(i.b)("inlineCode",{parentName:"p"},".img"),". It is the same case for the HFA driver in GDAL. "),Object(i.b)("p",null,"Fortunately, it has all the low-level implementations included to parse raw bytes into a generic higher level HFA data classes."),Object(i.b)("pre",null,Object(i.b)("code",{parentName:"pre"},"+---hfa\n|       hfa.h\n|       hfaband.cpp         # class to represent a raster band in a .img\n|       hfacompress.cpp\n|       hfadictionary.cpp\n|       hfaentry.cpp\n|       hfafield.cpp\n|       hfaopen.cpp\n|       hfatype.cpp\n|       hfa_p.h\n")),Object(i.b)("p",null,"What's left for me is to fill in the gaps by implementing high level representations of data types found in a ",Object(i.b)("inlineCode",{parentName:"p"},".ovr")," HFA structure. With a few changes, I was able to integrate this driver to successfully parse a ",Object(i.b)("inlineCode",{parentName:"p"},".ovr")," file."),Object(i.b)("h2",{id:"hierarchical-file-architecture-hfa"},"Hierarchical File Architecture (HFA)"),Object(i.b)("p",null,"Hidden within the detailed documentation lies the secrets to HFA. "),Object(i.b)("blockquote",null,Object(i.b)("p",{parentName:"blockquote"},"The hierarchical file architecture maintains an object\u2212oriented representation of data in an ERDAS IMAGINE\ndisk file through use of a tree structure. Each object is called an entry and occupies one node in the tree. Each\nobject has a name and a type. The type refers to a description of the data contained by that object. Additionally\neach object may contain a pointer to a subtree of more nodes.")),Object(i.b)("p",null,Object(i.b)("img",{alt:"HFA File Structure",src:a(137).default})),Object(i.b)("h3",{id:"header-tag"},"Header Tag"),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},"dtype: ",Object(i.b)("inlineCode",{parentName:"li"},"Ehfa_HeaderTag")),Object(i.b)("li",{parentName:"ul"},"size: ",Object(i.b)("inlineCode",{parentName:"li"},"20b"),Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},"First ",Object(i.b)("inlineCode",{parentName:"li"},"16b")," contains the unique signature of a ERDAS IMAGE HFA File: ",Object(i.b)("strong",{parentName:"li"},"EHFA_HEADER_TAG")),Object(i.b)("li",{parentName:"ul"},"Remaining ",Object(i.b)("inlineCode",{parentName:"li"},"4b")," contains the file pointer to the header record")))),Object(i.b)("p",null,Object(i.b)("strong",{parentName:"p"},"The header tag does not correspond to the header node shown in the diagram above! This is just a tag that contains a reference to the header node")),Object(i.b)("blockquote",null,Object(i.b)("p",{parentName:"blockquote"},"The value of a file pointer is simply the number of bytes from the start of the file")),Object(i.b)("pre",null,Object(i.b)("code",{parentName:"pre",className:"language-cpp"},'\n/* Referenced from HFA Driver */\n\n// Parsing the first 16 bytes to verify if it is a HFA file\n\nif( VSIFReadL( szHeader, 16, 1, fp ) < 1 )\n{\nCPLError( CE_Failure, CPLE_AppDefined,\n      "Attempt to read 16 byte header failed for\\n%s.",\n      pszFilename );\n\nreturn NULL;\n}\n\nif( !EQUALN(szHeader,"EHFA_HEADER_TAG",15) )\n{\nCPLError( CE_Failure, CPLE_AppDefined,\n      "File %s is not an Imagine HFA file ... header wrong.",\n      pszFilename );\n\nreturn NULL;\n}\n\n// Parsing the subsequent 4 bytes to get header record/node\n\nVSIFReadL( &nHeaderPos, sizeof(GInt32), 1, fp );\nHFAStandard( 4, &nHeaderPos );\n\nVSIFSeekL( fp, nHeaderPos, SEEK_SET );\n')),Object(i.b)("h3",{id:"header-record"},"Header Record"),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},"dtype: ",Object(i.b)("inlineCode",{parentName:"li"},"Ehfa_File")),Object(i.b)("li",{parentName:"ul"},"The header record contains file pointers to the ",Object(i.b)("strong",{parentName:"li"},"Root node")," of the HFA Tree and the ",Object(i.b)("strong",{parentName:"li"},"MIF (Machine Independent Format) Dictionary"),".",Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},"MIF Dictionary stores all the type information for each kind of node in the HFA Tree.")))),Object(i.b)("h3",{id:"mif-dictionary"},"MIF Dictionary"),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},"dtype: ",Object(i.b)("inlineCode",{parentName:"li"},"char*")),Object(i.b)("li",{parentName:"ul"},"The MIF Dictionary contains different type information for different nodes in the HFA Tree. Hence, ",Object(i.b)("strong",{parentName:"li"},"the dictionary must be read and decoded before any of the other objects in the file can be decoded"))),Object(i.b)("pre",null,Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"# Sample dtypes seen in MIF Dictionary\n\n{1:lversion,1:LfreeList,1:LrootEntryPtr,1:sentryHeaderLength,1:LdictionaryPtr,}Ehfa_File,\n\n{1:Lnext,1:Lprev,1:Lparent,1:Lchild,1:Ldata,1:ldataSize,64:cname,32:ctype,1:tmodTime,}Ehfa_Entry,\n\n{16:clabel,1:LheaderPtr,}Ehfa_HeaderTag,\n\n")),Object(i.b)("p",null,"Type information is encoded like such: "),Object(i.b)("pre",null,Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"{<num_element>:<dtype_char><attr_name>}<dtype_identifier>\n")),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"num_element"),": Number of ",Object(i.b)("inlineCode",{parentName:"p"},"dtype_char")," element "),Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},"Example: To encode the Header Tag in the ",Object(i.b)("inlineCode",{parentName:"li"},"Ehfa_HeaderTag")," data type, each of the first ",Object(i.b)("inlineCode",{parentName:"li"},"16b")," has a ",Object(i.b)("inlineCode",{parentName:"li"},"char")," dtype occupying ",Object(i.b)("inlineCode",{parentName:"li"},"1b")," of space. To encode the entire tag 16 ",Object(i.b)("inlineCode",{parentName:"li"},"char")," is needed, hence ",Object(i.b)("inlineCode",{parentName:"li"},"{16:clabel}")))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"dtype_char"),": MIF data type identifier"),Object(i.b)("p",{parentName:"li"},Object(i.b)("img",{alt:"MIF ItemType",src:a(138).default}))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"attr_name"),": Name of attribute under type")),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"dtype_identifier"),": data type of node"))),Object(i.b)("h3",{id:"root-node"},"Root Node"),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},"dtype: ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry"))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},'The root node is the entry point into the main HFA tree structure. By traversing down the tree from the root, data nodes (where the "gold" is at) can extracted')),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},"A ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry")," dtype contains file pointers to the ",Object(i.b)("em",{parentName:"p"},"next node")," and ",Object(i.b)("em",{parentName:"p"},"child node"),". We use these pointers to traverse down the HFA Tree."),Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},Object(i.b)("strong",{parentName:"li"},"Next Node"),': The node "right" of the current node on the same level'),Object(i.b)("li",{parentName:"ul"},Object(i.b)("strong",{parentName:"li"},"Child Node"),': "Left-Most" child node')))),Object(i.b)("h3",{id:"ehfa_entry"},"Ehfa_Entry"),Object(i.b)("p",null,Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry")," is data type representing the nodes of the HFA tree structure. A single ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry"),' contains "tree" level attributes (file ptr to child node, name of node etc.) and  a file pointer reference to the data block.'),Object(i.b)("p",null,"To be put it simply, there are 2 different sections to a ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry")),Object(i.b)("p",null,"To dive deeper into the 2 sections, let's look at the type definition of ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry")," found in the ",Object(i.b)("strong",{parentName:"p"},"MIF Dictionary")),Object(i.b)("p",null,Object(i.b)("inlineCode",{parentName:"p"},"{1:Lnext,1:Lprev,1:Lparent,1:Lchild,1:Ldata,1:ldataSize,64:cname,32:ctype,1:tmodTime,}Ehfa_Entry,")),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("strong",{parentName:"p"},"Base section")),Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},"Tree level attributes/metadata (attributes in the type definition above are all tree level attributes)\t",Object(i.b)("blockquote",{parentName:"li"},Object(i.b)("p",{parentName:"blockquote"},"Tree level attributes can be interpreted as meta attributes that allow traversal of the tree structure without ever requiring to touch the data residing in it"))))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("strong",{parentName:"p"},"Data section")),Object(i.b)("ul",{parentName:"li"},Object(i.b)("li",{parentName:"ul"},"Data within the node (annotation name, coordinates etc.)"),Object(i.b)("li",{parentName:"ul"},"The data block can be accessed using the file pointer value found in the ",Object(i.b)("inlineCode",{parentName:"li"},"data")," attribute",Object(i.b)("blockquote",{parentName:"li"},Object(i.b)("p",{parentName:"blockquote"},Object(i.b)("inlineCode",{parentName:"p"},"{1:Ldata}")," contains the file pointer to the data node "))),Object(i.b)("li",{parentName:"ul"},"The data that resides in the node has a corresponding data type that can be found in the ",Object(i.b)("strong",{parentName:"li"},"MIF Dictionary"),". The data type is identifiable by the ",Object(i.b)("inlineCode",{parentName:"li"},"type")," attribute seen above.",Object(i.b)("blockquote",{parentName:"li"},Object(i.b)("p",{parentName:"blockquote"},Object(i.b)("inlineCode",{parentName:"p"},"{32:ctype}")," contains the data type identifier")))))),Object(i.b)("p",null,Object(i.b)("strong",{parentName:"p"},"In order to extract the data we are interested in, we will seek the specified file position in the ",Object(i.b)("inlineCode",{parentName:"strong"},"data")," attribute and parse the bytes into the higher level data structure specified in ",Object(i.b)("inlineCode",{parentName:"strong"},"type")," attr.")),Object(i.b)("p",null,"The following are examples of data type identifiers that can be found in a  ",Object(i.b)("inlineCode",{parentName:"p"},"Ehfa_Entry"),":"),Object(i.b)("ul",null,Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"Rectangle2"),": Contains geometric definitions of ERDAS Rectangle Annotations"),Object(i.b)("pre",{parentName:"li"},Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"{1:Lflags,1:lfillStyle,1:*oEevg_Coord,center,1:dwidth,1:dheight,1:dorientation,}Rectangle2\n"))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"Eprj_MapInfo"),": Contains image map coordinates"),Object(i.b)("pre",{parentName:"li"},Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"{0:pcproName,1:*oEprj_Coordinate,upperLeftCenter,1:*oEprj_Coordinate,lowerRightCenter,1:*oEprj_Size,pixelSize,0:pcunits,}Eprj_MapInfo\n"))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"Eprj_ProParameters"),": Contains projection parameters"),Object(i.b)("pre",{parentName:"li"},Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"{1:e2:EPRJ_INTERNAL,EPRJ_EXTERNAL,proType,1:lproNumber,0:pcproExeName,0:pcproName,1:lproZone,0:pdproParams,1:*oEprj_Spheroid,proSpheroid,}Eprj_ProParameters\n"))),Object(i.b)("li",{parentName:"ul"},Object(i.b)("p",{parentName:"li"},Object(i.b)("inlineCode",{parentName:"p"},"Eprj_Datum"),": Contains datum information"),Object(i.b)("pre",{parentName:"li"},Object(i.b)("code",{parentName:"pre",className:"language-cpp"},"{0:pcdatumname,1:e3:EPRJ_DATUM_PARAMETRIC,EPRJ_DATUM_GRID,EPRJ_DATUM_REGRESSION,type,0:pdparams,0:pcgridname,}Eprj_Datum\n")))))}c.isMDXComponent=!0}}]);
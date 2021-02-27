(window.webpackJsonp=window.webpackJsonp||[]).push([[9],{79:function(e,t,n){"use strict";n.r(t),n.d(t,"frontMatter",(function(){return l})),n.d(t,"metadata",(function(){return o})),n.d(t,"toc",(function(){return i})),n.d(t,"default",(function(){return p}));var a=n(3),r=n(7),b=(n(0),n(95)),l={id:"installation",title:"Installation",sidebar_label:"Installation"},o={unversionedId:"installation",id:"installation",isDocsHomePage:!1,title:"Installation",description:"Building from source",source:"@site/docs\\installation.md",slug:"/installation",permalink:"/ovr2shp/docs/installation",editUrl:"https://github.com/shenyihong/tree/gh-pages/docs/installation.md",version:"current",sidebar_label:"Installation",sidebar:"someSidebar",next:{title:"Basic Usage",permalink:"/ovr2shp/docs/"}},i=[{value:"Building from source",id:"building-from-source",children:[]},{value:"Docker",id:"docker",children:[]}],c={toc:i};function p(e){var t=e.components,n=Object(r.a)(e,["components"]);return Object(b.b)("wrapper",Object(a.a)({},c,n,{components:t,mdxType:"MDXLayout"}),Object(b.b)("h2",{id:"building-from-source"},"Building from source"),Object(b.b)("table",null,Object(b.b)("thead",{parentName:"table"},Object(b.b)("tr",{parentName:"thead"},Object(b.b)("th",{parentName:"tr",align:null},"Dependencies"),Object(b.b)("th",{parentName:"tr",align:null},"Version"))),Object(b.b)("tbody",{parentName:"table"},Object(b.b)("tr",{parentName:"tbody"},Object(b.b)("td",{parentName:"tr",align:null},"GDAL"),Object(b.b)("td",{parentName:"tr",align:null},Object(b.b)("inlineCode",{parentName:"td"},">=3.2.0"))),Object(b.b)("tr",{parentName:"tbody"},Object(b.b)("td",{parentName:"tr",align:null},"gnuplot (",Object(b.b)("em",{parentName:"td"},"optional"),")"),Object(b.b)("td",{parentName:"tr",align:null},Object(b.b)("inlineCode",{parentName:"td"},"5.2"))),Object(b.b)("tr",{parentName:"tbody"},Object(b.b)("td",{parentName:"tr",align:null},Object(b.b)("inlineCode",{parentName:"td"},"gnuplot-iostream.h")," (",Object(b.b)("em",{parentName:"td"},"optional"),")"),Object(b.b)("td",{parentName:"tr",align:null},Object(b.b)("a",{parentName:"td",href:"https://github.com/dstahlke/gnuplot-iostream"},"latest"))))),Object(b.b)("p",null,"Build ",Object(b.b)("strong",{parentName:"p"},"without")," ",Object(b.b)("inlineCode",{parentName:"p"},"gnuplot")," "),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre",className:"language-sh"},"make build\n")),Object(b.b)("p",null,"Build ",Object(b.b)("strong",{parentName:"p"},"with")," ",Object(b.b)("inlineCode",{parentName:"p"},"gnuplot")),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre",className:"language-sh"},"make build-gnuplot\n")),Object(b.b)("h2",{id:"docker"},"Docker"),Object(b.b)("p",null,Object(b.b)("em",{parentName:"p"},"This is a development image. Not recommended for production")),Object(b.b)("p",null,Object(b.b)("strong",{parentName:"p"},"Build")),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre"},"docker build -t ovr2shp .\n")),Object(b.b)("p",null,Object(b.b)("strong",{parentName:"p"},"Spin up a container")),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre"},"docker run --name ovr2shp -v <data_path>:/ovr2shp/data ovr2shp\n")),Object(b.b)("ul",null,Object(b.b)("li",{parentName:"ul"},Object(b.b)("inlineCode",{parentName:"li"},"data_path")," : path to data directory that contains ",Object(b.b)("inlineCode",{parentName:"li"},".ovr")," files to be converted")),Object(b.b)("p",null,Object(b.b)("strong",{parentName:"p"},"Compile")),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre"},":/ovr2shp# make build\n")),Object(b.b)("p",null,Object(b.b)("strong",{parentName:"p"},"Run")),Object(b.b)("pre",null,Object(b.b)("code",{parentName:"pre"},":/ovr2shp# ./ovr2shp -d -t -o ./annos.shp ./data/annos.ovr\n")))}p.isMDXComponent=!0}}]);
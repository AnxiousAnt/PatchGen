 PdFileFormat 

PD fileformat specification  
by [Simon Asselbergs](mailto:simon3@student-kmt.hku.nl) and [Tjeerd Sietsma](mailto:tjeerd@student-kmt.hku.nl)  


Introduction

> This file describes the fileformat of patchfiles (\*.pd) of Miller Puckette's PureData.
> 
> It is released because:

*   PureData's patchfile format is officially undocumented.
*   It may serve as a partial tutorial to help readers understand PureData better.

> This documents is currently unofficial, beware that there is no warranty whatsoever.  
> If you find this document incorrect or incomplete please send us an e-mail using the links at the top of this document.  
> There are a few things still unknown, we don't exclude the possibility those things are used for compatibility with Max.
> 


  
Records

> The PD fileformat is a genuine custom textfile format, not to be confused with XML.  
> It consists of one ore more records.  
> Each record may cover multiply lines but they all have thesame syntax:
> 
> **#**\[data\]**;**\\r\\n
> 
> where\[data\] holds the record data, \\r represents an ASCII code 13 carriage return character, and \\n represents an ASCII code 10 line-feed character.  
> Let us take a closer look on the records contents.
> 
> Each record consists of a chunk type, element type and optional parameters like this:
> 
> **#**\[chunk\_type\] \[element\_type\] \[p1\] \[p2\] \[p3\] \[...\]**;\\r\\n**
> 
> \[chunk\_type\] is a sinlge character with only three possible values: "X" for an object, "N" for a new window, and finally "A" for array data.
> 
> \[element\_type\] is a predefined PD element. This declaration is also used for wires. _Object_ elements are numbered in order of appearance in the file, all other elements are excluded from numbering. These numbers are pure virtual and can not be seen directly in the file.
> 
> \[p1\] \[p2\] \[p3\] \[...\] are the required parameters for each element, this differences per element.
> 


  
Wiring

> Almost all objects can be interconnected with wires in PureData. Each wire is stored in the file using the following syntax:
> 
> **#X connect** \[source\] \[outlet\_number\] \[sink\] \[inlet\_number\]**;\\r\\n**
> 
> \[source\] Is the number of the object the data is coming from. \[outlet-number\] Represents the number of the outlet of the source object where the wire starts. Sequentially, \[sink\] is the number of the target object, and finally \[inlet\_number\] specifies inlet of the target object to which the wired is connected.  
> Logically, the objects (again, **connect**s excluded) are numbered from 0 to the total number of objects in the file using the integer format. The inlets and outlets are numbered likewise.  
> Please keep this in mind, to prevent making often made off-by-one errors.
> 


  
Elements and objects

> There's a difference between elements and objects. Elements are the parts that together make up the entire layout of a patch, including windowsizes and position. Objects are the building blocks of PureData that contain functionality, gui-related or not.
> 
> All other **elements** are used to build actual PureData objects:

*   array - visual two dimensional array
*   canvas - specifications for the windowsizes and position
*   coords - used for graphs
*   floatatom - number
*   msg - message
*   obj - object, empty, subpatch or gui:
    *   bang
    *   toggle
    *   nbx
    *   vslider
    *   hslider
    *   vu
    *   canvas - gui element
    *   pd \[name\]
    *   \[name\]
*   restore - for exiting subwindows and graphs
*   symbolatom - symbol
*   text - comment



  
Common parameters

*   _Positions and size_: As PureData uses a graphical interface every gui-related element (object, message, number, symbol, comment, bang, toggle, number2, vslider, hslider, vradio, hradio, vu, canvas, graph, array) have a horizontal and vertical position in the window that holds the (sub)patch.  
    Records of gui elements that have adjustable sizes also contain the horizontal and/or vertical size.  
    Positions and sizes are stored in pixels.  
    Note that in graphs (element: coords) the pixel sizes of its canvas are relative of the coordinates of the graph, when the option "graph on parent" is selected.
*   _Color_: Some graphical elements have color attributes. Per color only one signed integer value is stored that contains the three 8-bit color components (RGB).  
    Formula to calculate color attribute values:  
      
    **color = (** \[red\] **\* -65536) + (** \[green\] **\* -256) + (** \[blue\] **\* -1)**Where \[red\], \[green\], \[blue\] obviously represent the three color components, their values range from 0 to 255.  
    They apply to the attributes \[background color\], \[front color\], \[label color\] of various elements.
*   _Fonts_: elements that have a unsigned integer \[font\] attribute (e.g. the element **text**) have a choice out of three different fonts:  
    0 = Courier, 1 = Helvetica, 2 = Times. Courier is the only available fixed width font.  
    The \[fontsize\] attribute contains the font size in pixels.
*   _Labels_: the many GUI-elements that have a \[label\] attribute can be named. In its label string no spaces ASCII code 32 is allowed. It will generally be recognized as a seperator character for the next parameter/attribute.
*   _Other parameters_: Because of the difference between element parameters there is no default syntax for all elements. They may or may not look much like other elements, depending on their visual and functional similarities.



  

* * *

Reference

*   [A](https://puredata.info/docs/developer/PdFileFormat#r1)
*   [N](https://puredata.info/docs/developer/PdFileFormat#r2)
    *   [canvas](https://puredata.info/docs/developer/PdFileFormat#r21)
*   [X](https://puredata.info/docs/developer/PdFileFormat#r3)
    *   [array](https://puredata.info/docs/developer/PdFileFormat#r31)
    *   [connect](https://puredata.info/docs/developer/PdFileFormat#r32)
    *   [coords](https://puredata.info/docs/developer/PdFileFormat#r33)
    *   [floatatom](https://puredata.info/docs/developer/PdFileFormat#r34)
    *   [msg](https://puredata.info/docs/developer/PdFileFormat#r35)
    *   [obj](https://puredata.info/docs/developer/PdFileFormat#r36)
        *   [bng](https://puredata.info/docs/developer/PdFileFormat#r361)
        *   [tgl](https://puredata.info/docs/developer/PdFileFormat#r362)
        *   [nbx](https://puredata.info/docs/developer/PdFileFormat#r363)
        *   [vsl](https://puredata.info/docs/developer/PdFileFormat#r364)
        *   [hsl](https://puredata.info/docs/developer/PdFileFormat#r365)
        *   [vradio](https://puredata.info/docs/developer/PdFileFormat#r366)
        *   [hradio](https://puredata.info/docs/developer/PdFileFormat#r367)
        *   [vu](https://puredata.info/docs/developer/PdFileFormat#r368)
        *   [cnv](https://puredata.info/docs/developer/PdFileFormat#r369)
    *   [\[name\]](https://puredata.info/docs/developer/PdFileFormat#r37)
    *   [pd \[name\]](https://puredata.info/docs/developer/PdFileFormat#r38)
    *   [restore](https://puredata.info/docs/developer/PdFileFormat#r39)
    *   [symbolatom](https://puredata.info/docs/developer/PdFileFormat#r3A)
    *   [text](https://puredata.info/docs/developer/PdFileFormat#r3B)



  

* * *

  

A

Description:

Announces array data

Syntax:

**#A** \[p1\] \[p2\] \[p3\] \[...\]**;\\r\\n**

Parameters:

\[p1\] \[p2\] \[p3\] \[...\] floating point variables representing array elements

Example:

See Array

Remarks:

Used only in combination of an array definition



N

Description:

Announces a frameset

Syntax:

**#N** \[new\_frame\]**;\\r\\n**

Parameters:

\[new\_frame\] new frameset, currently only a new patchwindow can be defined

Example:

see canvas

Remarks:

Currently used only for new canvas definitions



canvas

Description:

Defines window properties

Syntax:

**#N canvas** \[x\_pos\] \[y\_pos\] \[x\_size\] \[y\_size\] \[name\] \[open\_on\_load\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position offset of frameset (window)  
\[y\_pos\] - vertical position offset of frameset (window)  
\[x\_size\] - horizontal size of frameset (window)  
\[y\_size\] - vertical size of frameset (window)  
\[name\] - name / handle of frameset  
\[open\_on\_load\] - when flag is set the canvas is opened when the patch is loaded

Example:

#N canvas 0 0 452 302 12;

Example:

#N canvas 0 0 452 302 thiscanvas 0;  
#X restore 41 136 pd thiscanvas;

Remarks:

In this example patch there are two canvas definitions.  
The first example has a special syntax, the attributes \[name\] and \[open\_on\_load\] have been replaced by a \[font\_size\] attribute. This is only the case when the canvas definition is the first record in the patchfile, to define the position and size of the main patcher window, and set the default font size. When a first canvas definition is absent the default values of PureData are used. Note that in this case you can alter the default font size with a command line parameter.  
The second example shows a regular internal subpatch definition. Normally a canvas defintion is always postceeded with a restore element to define the position within the canvas, and the name of the subpatch.  
Presumably, the canvas name in the canvas defintion and restore definition should be thesame.



X

Description:

Announces regular elements

Syntax:

**#X** \[element\]**;\\r\\n**

Parameters:

\[element\] - element definition

Example:

#X obj 50 36;

Remarks:

Used with _every_ element definition except canvas definitions



array

Description:

Array

Syntax:

**#X array** \[array\_name\] \[array\_size\] **float** \[save\_flag\]**;\\r\\n**

Parameters:

\[array\_name\] - name / handle of the array  
\[array\_size\] - total number of elements of an array  
\[save\_flag\] - with this flag set the array data is stored in the patch

Example:

#N canvas 0 0 450 300 graph4 0;  
#X array array3 10 float 1;  
#A 0 0 0 0 0 0 0 0 0 0;  
#X coords 0 1 99 -1 200 140 1;  
#X restore 270 193 graph;

Remarks:

An array is a gui-form of a table, always visualised using a graph. Optionally the array data can be saved in the patch - which can be **very** space-consuming and CPU intensive when you want to store a large number of floating point values typed out in a textfile.



connect

Description:

Wires GUI-elements

Syntax:

**#X connect** \[source\] \[outlet\] \[target\] \[inlet\]**;\\r\\n**

Parameters:

\[source\]  
\[outlet\]  
\[target\]  
\[inlet\]

Example:

#X obj 30 27 midiin;  
#X obj 26 59 midiout;  
#X connect 0 0 1 0;  
#X connect 0 1 1 1;

Remarks:

**Obj**ects are virtually numbered in order of appearance in the file, starting from zero. Inlets and outlets of the objects are numbered likewise.



coords

Description:

Visual ranges of a frameset (window)

Syntax:

**#X coords** \[x\_from\] \[y\_to\] \[x\_to\] \[y\_from\] \[width\] \[heigth\] \[graph\_on\_parent\]**;\\r\\n**

Parameters:

\[x\_from\] - first index to display  
\[y\_to\] - dynamic range of graph upper border  
\[x\_to\] - last index to display  
\[y\_from\] - dynamic range of graph lower border  
\[width\] - relative horizontal size of graph  
\[heigth\] - relative vertical size of graph  
\[graph\_on\_parent\] - when set displayes child path, when unset graph is displayed like an object

Example:

#N canvas 0 0 452 302 graph1 0;  
#X coords 0 1 100 -1 200 140 1;  
#X restore 58 26 graph;

Remarks:

A coords statement must always be preceded with a canvas statement which also holds the graph name.  
A coords statement must always be terminated with a restore statement that has the reserved handle **graph  
**Off limit values will be displayed outside the graph in the PureData GUI.



floatatom

Description:

Defines a number box

Syntax:

**#X floatatom** \[x\_pos\] \[y\_pos\] \[width\] \[lower\_limit\] \[upper\_limit\] \[label\_pos\] \[label\] \[receive\] \[send\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within window  
\[y\_pos\] - vertical position within window  
\[width\] - number of digits  
\[lower\_limit\] - minimal value  
\[upper\_limit\] - maximum value  
\[label\_pos\] - label position relative to floatatom position. 0 = left, 1 = right, 2 = top, 3 = bottom  
\[label\] - floatatom label/name  
\[receive\] - receive symbol name  
\[send\] - send symbol name

Example:

#X floatatom 32 26 5 0 0 0 - - -;

Remarks:

When the value of \[upper\_limit\] minus the value of \[lower\_limit\] is less than one, or the \[width\] attribute is set to one, PureData resets these values both to zero.  
Floatatom and symbolatom are the only elements that uses "-" characters to indicate that no value has been assigned to its attributes \[label\], \[receive\] and \[send\].



msg

Description:

Defines a message

Syntax:

**#X msg** \[x\_pos\] \[y\_pos\] \[p1\] \[p2\] \[p3\] \[...\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[p1\] \[p2\] \[p3\] \[...\] the content of the message

Example:

#X msg 61 48 read audio.wav;

Remarks:

\-



obj

Description:

Defines an object

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] \[object\_name\] \[p1\] \[p2\] \[p3\] \[...\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[object\_name\] - name of the object (optional)  
\[p1\] \[p2\] \[p3\] \[...\] the parameters of the object (optional)

Example:

#X obj 55 50;  
#X obj 132 72 trigger bang float;

Remarks:

The first line is an example of an empty object. The second line describes a trigger object with its parameters**.  
**



bng

Description:

Defines a bang

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **bng** \[size\] \[hold\] \[interrupt\] \[init\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] **;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - square size of the gui element  
\[hold\] - hold time in milliseconds, ranges from 50 to 1000000000  
\[interrupt\] - interrupt time in milliseconds, ranges from 10 to 250  
\[init\] - bang on load  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color

Example:

#X obj 27 32 bng 15 10000 100 1 empty empty empty 0 -6 0 8 -262144 -1 -1;

Remarks:

Hold time is for how long you see a flash when you click on the bang, interrupt time is for how long you don't see it flash when you click on this object while it's flashing.\[send\], \[receive\] and \[label\] cannot be named "empty", this is a reserved name for when no value is assigned.



tgl

Description:

Defines a toggle

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **tgl** \[size\] \[init\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[init\_value\] \[default\_value\] **;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - square size of the gui element  
\[init\] - set on load  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[init\_value\] - value sent when the \[init\] attribute is set  
\[default\_value\] - default value when the \[init\] attribute is not set

Example:

#X obj 29 44 tgl 15 1 empty empty empty 0 -6 192 8 -262144 -1 -1 234 234;

Remarks:

\[send\], \[receive\] and \[label\] cannot be named "empty", this is a reserved name for when no value is assigned.  
The \[default\_value\] attribute can be changed in a patch and saved, this is why there are different values for \[init\_value\] and \[default\_value\]



nbx

Description:

Defines a Number2 number box

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **nbx** \[size\] \[height\] \[min\] \[max\] \[log\] \[init\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[log\_height\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - number of digits the element displays  
\[height\] - vertical size of element in pixels  
\[min\] - minimum value, typically -1e+037  
\[max\] - maximum value, typically 1e+037  
\[log\] - linear when unset, logarithmic when set  
\[init\] - when set outputs  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size in pixels  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[log\_height\] - logarithmic steps, accepts values from 10 to 2000, default is 256

Example:

#X obj 39 48 nbx 5 14 -1e+037 1e+037 0 0 empty empty empty 0 -6 0 10 -262144 -1 -1 0 256;

Remarks:

This element resembles the floatatom element, but is extended with gui and logarithmic parameters and (probably) more accurate.  
The attributes \[font\], \[font\_size\] and \[fg\_color\] also apply to the digits.



vsl

Description:

Defines a vertical slider

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **vsl** \[width\] \[height\] \[bottom\] \[top\] \[log\] \[init\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[default\_value\] \[steady\_on\_click\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[width\] - horizontal size of gui element  
\[height\] - vertical size of gui element  
\[bottom\] - minimum value  
\[top\] - maximum value  
\[log\] - when set the slider range is outputted logarithmically, otherwise it's output is linair  
\[init\] - sends default value on patch load  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[default\_value\] - default value times hundred  
\[steady\_on\_click\] - when set, fader is steady on click, otherwise it jumps on click

Example:

#X obj 50 38 vsl 15 128 0 127 0 0 empty empty empty 0 -8 0 8 -262144 -1 -1 0 1;

Remarks:

The vertical slider object and the horizontal slider are the only objects which have a default value multiplied by hundred. This purpose is unknown.



hsl

Description:

Defines a horizontal slider

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **hsl** \[width\] \[height\] \[bottom\] \[top\] \[log\] \[init\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[default\_value\] \[steady\_on\_click\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[width\] - horizontal size of gui element  
\[height\] - vertical size of gui element  
\[bottom\] - minimum value  
\[top\] - maximum value  
\[log\] - when set the slider range is outputted logarithmically, otherwise it's output is linair  
\[init\] - sends default value on patch load  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[default\_value\] - default value times hundred  
\[steady\_on\_click\] - when set, fader is steady on click, otherwise it jumps on click

Example:

#X obj 53 44 hsl 128 15 0 127 0 0 empty empty empty -2 -6 0 8 -262144 -1 -1 0 1;

Remarks:

The horizontal slider object and the vertical slider are the only objects which have a default value multiplied by hundred. This purpose is unknown.  



vradio

Description:

Defines a vertical radio button selector

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **vradio** \[size\] \[new\_old\] \[init\] \[number\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[default\_value\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - horizontal size and vertical size, depending on the number of radio buttons  
\[new\_old\] - send new and old value, or only the new value  
\[init\] - send default value on init  
\[number\] - amount of radio buttons  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[default\_value\] - default value to be sent on patch load when the \[init\] attribute has been set.

Example:

#X obj 48 42 vradio 15 1 0 8 empty empty empty 0 -6 0 8 -262144 -1 -1 0;

Remarks:

The purpose of the \[new\_old\] switch is unknown.



hradio

Description:

Defines a horizontal radio button selector

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **hradio** \[size\] \[new\_old\] \[init\] \[number\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[fg\_color\] \[label\_color\] \[default\_value\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - vertical size and horizontal size, depending on the number of radio buttons  
\[new\_old\] - send new and old value, or only the new value  
\[init\] - send default value on init  
\[number\] - amount of radio buttons  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[fg\_color\] - foreground color  
\[label\_color\] - label color  
\[default\_value\] - default value to be sent on patch load when the \[init\] attribute has been set.

Example:

#X obj -50 54 hradio 15 1 0 8 empty empty empty 0 -6 0 8 -262144 -1 -1 0;

Remarks:

The purpose of the \[new\_old\] switch is unknown.



vu

Description:

Defines a VU-meter

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **vu** \[width\] \[height\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[fontsize\] \[bg\_color\] \[label\_color\] \[scale\] \[?\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[width\] - horizontal size of element  
\[height\] - vertical size of element  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[label\_color\] - label color  
\[scale\] - when set the logarithmic scale is displayed  
\[?\] - unknown value, default is zero

Example:

#X obj 40 44 vu 15 120 empty empty -1 -8 0 8 -66577 -1 1 0;

Remarks:

We looked inside the source code but still couldn't see the purpose of the final value.



cnv

Description:

Defines a canvas, a gui component

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] **cnv** \[size\] \[width\] \[height\] \[send\] \[receive\] \[label\] \[x\_off\] \[y\_off\] \[font\] \[font\_size\] \[bg\_color\] \[label\_color\] \[?\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - size of selectable square  
\[width\] - horizontal size of the GUI-element  
\[height\] - vertical size of the GUI-element  
\[send\] - send symbol name  
\[receive\] - receive symbol name  
\[label\] - label  
\[x\_off\] - horizontal position of the label text relative to the upperleft corner of the object  
\[y\_off\] - vertical position of the label text relative to the upperleft corner of the object  
\[font\] - font type  
\[fontsize\] - font size  
\[bg\_color\] - background color  
\[label\_color\] - foreground color  
\[?\] - unknown value, default is zero

Example:

#X obj 27 40 cnv 15 100 60 empty empty empty 20 12 0 14 -233017 -66577 0;

Remarks:

We couldn't find the purpose of the final value.



\[name\]

Description:

Defines an external subpatch or library patch

Syntax:

**#X obj** \[x\_pos\] \[y\_pos\] \[name\] \[p1\] \[p2\] \[p3\] \[...\] **;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[name\] - name of the subpatch / library patch  
\[p1\] \[p2\] \[p3\] \[...\] - optional parameters

Example:

#X obj 121 102 subpatch;

Remarks:

If the external object isn't in the loaded libraries PureData searches in the folder of the main frame (window) to locate the object.



pd

Description:

Defines an internal subpatch

Syntax:

**#X restore** \[x\_pos\] \[y\_pos\] **pd** \[name\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[size\] - size of selectable square  
\[width\] - horizontal size of the GUI-element  
\[name\] - name of the subpatch

Example:

#N canvas 0 0 454 304 inc 0;  
#X obj 34 40 inlet;  
#X obj 34 95 outlet;  
#X obj 34 67 + 1;  
#X connect 0 0 2 0;  
#X connect 2 0 1 0;  
#X restore 90 124 pd inc;  
#X floatatom 90 99 5 0 0 0 - - -;  
#X floatatom 90 151 5 0 0 0 - - -;  
#X connect 0 0 2 0;  
#X connect 1 0 0 0;

Remarks:

Naturally the restore element which invokes a subpatch is preceded with a canvas element and the subpatch elements.  
Objects within a subpatch are counted seperately from a parent frame (window).



restore

Description:

Ends a canvas definition

Syntax:

**#X restore** \[x\_pos\] \[y\_pos\] \[type\] \[name\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[type\] - type of canvas, values are either "graph" or "pd"  
\[name\] - name of the subpatch, only used when the \[type\] attribute is set to "pd"

Example:

#N canvas 0 0 450 300 graph2 0;  
#X coords 0 1 100 -1 200 140 1;  
#X restore 27 30 graph;

Example:

#N canvas 0 0 452 302 subpatch 0;  
#X restore 64 69 pd subpatch;

Remarks:

The restore element is used only in combination with canvas elements. There are two uses:  
In the first example it defines a graph, in this case a coords element is required.  
In the second example it defines a subpatch. In this case only the canvas attribute \[name\] and the restore attribute \[name\] should correlate.



symbolatom

Description:

Defines an symbol box

Syntax:

**#X symbolatom** \[x\_pos\] \[y\_pos\] \[width\] \[lower\_limit\] \[upper\_limit\] \[label\_pos\] \[label\] \[receive\] \[send\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[width\] - amount of digits/characters  
\[lower\_limit\] - minimum value  
\[upper\_limit\] - maximum value  
\[label\_pos\] - label position relative to floatatom position. 0 = left, 1 = right, 2 = top, 3 = bottom  
\[label\] - label  
\[receive\] - receive symbol value  
\[send\] - send symbol value

Example:

#X symbolatom 36 37 10 0 0 0 - - -;

Remarks:

When the value of \[upper\_limit\] minus the value of \[lower\_limit\] is less than one, PureData resets these values both to zero.  
Symbolatom and floatatom are the only elements that uses "-" characters to indicate that no value has been assigned to its attributes \[label\], \[receive\] and \[send\].



text

Description:

Defines a comment

Syntax:

**#X text** \[x\_pos\] \[y\_pos\] \[comment\]**;\\r\\n**

Parameters:

\[x\_pos\] - horizontal position within the window  
\[y\_pos\] - vertical position within the window  
\[comment\] - custom string, spaces allowed  

Example:

#X text 28 25 comment;

Remarks:

ASCII return codes 13 and 10 are not stored, a semicolon character is preceded with the escape character backslash.



  


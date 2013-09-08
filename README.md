simple_xml
==========

This is a simple, header-only C++ library for XML handling. It is similar to TinyXML, but less feature-rich.  It is intended for situations where a basic XML library that can be included in a source tree is preferable to an external library.

Simple API's are provided for handling XML data loaded into strings via a bare-bones callback interface and a (more flexible) DOM API.  The DOM API also allows rudimentary manipulation of XML data, e.g. writing subtrees into streams, adding tags & attributes and so on.

I use this code for lightweight parsing of XML configuration and data files, which can be added to a project by simply including two header files.  However it is a very barebones library, so if you need anything more than the basic you should consider something like TinyXML.
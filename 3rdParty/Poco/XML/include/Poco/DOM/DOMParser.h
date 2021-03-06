//
// DOMParser.h
//
// $Id: //poco/1.3/XML/include/Poco/DOM/DOMParser.h#2 $
//
// Library: XML
// Package: DOM
// Module:  DOMParser
//
// Definition of the DOMParser class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef DOM_DOMParser_INCLUDED
#define DOM_DOMParser_INCLUDED


#include "Poco/XML/XML.h"
#include "Poco/SAX/SAXParser.h"


namespace Poco {
namespace XML {


class NamePool;
class Document;
class InputSource;
class EntityResolver;


class XML_API DOMParser
	/// This is a convenience class that combines a
	/// DOMBuilder with a SAXParser, with the optional
	/// support of a WhitespaceFilter.
{
public:
	DOMParser(NamePool* pNamePool = 0);
		/// Creates a new DOMParser. 
		/// If a NamePool is given, it becomes the Document's NamePool.

	~DOMParser();
		/// Destroys the DOMParser.

	void setEncoding(const XMLString& encoding);
		/// Sets the encoding used by the parser if no
		/// encoding is specified in the XML document.
		
	const XMLString& getEncoding() const;
		/// Returns the name of the encoding used by
		/// the parser if no encoding is specified in
		/// the XML document.

	void addEncoding(const XMLString& name, Poco::TextEncoding* pEncoding);
		/// Adds an encoding to the parser.

	void setFeature(const XMLString& name, bool state);
		/// Set the state of a feature.
		///
		/// If a feature is not recognized by the DOMParser, it is
		/// passed on to the underlying XMLReader.
		///
		/// The only currently supported feature is
		/// http://www.appinf.com/features/no-whitespace-in-element-content
		/// which, when activated, causes the WhitespaceFilter to
		/// be used.

	bool getFeature(const XMLString& name) const;
		/// Look up the value of a feature.
		///
		/// If a feature is not recognized by the DOMParser, the
		/// DOMParser queries the underlying SAXParser for the feature.

	Document* parse(const XMLString& uri);
		/// Parse an XML document from a location identified by an URI.

	Document* parse(InputSource* pInputSource);
		/// Parse an XML document from a location identified by an InputSource.

	Document* parseString(const std::string& xml);
		/// Parse an XML document from a string.

	Document* parseMemory(const char* xml, std::size_t size);
		/// Parse an XML document from memory.

	EntityResolver* getEntityResolver() const;
		/// Returns the entity resolver used by the underlying SAXParser.

	void setEntityResolver(EntityResolver* pEntityResolver);
		/// Sets the entity resolver on the underlying SAXParser.

	static const XMLString FEATURE_WHITESPACE;
	
private:
	SAXParser _saxParser;
	NamePool* _pNamePool;
	bool      _whitespace;
};


} } // namespace Poco::XML


#endif // DOM_DOMParser_INCLUDED

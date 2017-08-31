#pragma once

#include "BBTypes.h"

#include <stdio.h>

#import <c:\\windows\\system32\\msxml3.dll>

class CBBWinXmlParser
{
public:
	static const int maxNumericAttributeLength = 20; // for int and double, not strings
	static const int maxTagNameLength = 50;	

	CBBWinXmlParser();

	BBRETCODE Init();
	
	BBRETCODE LoadDocFromFile(const char *fileName);
	BBRETCODE ReadDocFromString(const char *xmlText);

	const char *GetOutermostTagName();

	// NOTE: attribute path must not contain the outermost tag
	BBRETCODE GetAttributeString(const char *attributePath, char *outValue, bool printErr = true);
	BBRETCODE GetAttributeDouble(const char *attributePath, double &outValue, bool printErr = true);
	BBRETCODE GetAttributeInt(const char *attributePath, int &outValue, bool printErr = true);

	void SetVerboseMode(bool isVerboseMode) { _isVerboseMode = isVerboseMode; }

protected:
	MSXML2::IXMLDOMDocumentPtr _plDomDocument;
	MSXML2::IXMLDOMElementPtr _pDocRoot;

	char _outermostTagName[maxTagNameLength];
	bool _wasInitialized;
	bool _wasDocumentRead;
	bool _isVerboseMode;


	BBRETCODE GetAttributeNode(const char *attributePath, VARIANT &value, bool printErr);
};
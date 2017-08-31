#include "BBWinXmlParser.h"
#include <tchar.h>

CBBWinXmlParser::CBBWinXmlParser()
{
	_wasDocumentRead = false;
	_wasInitialized = false;
	_outermostTagName[0] = '\0';
	_isVerboseMode = true;
}

BBRETCODE CBBWinXmlParser::Init()
{
	::CoInitialize(NULL);
	HRESULT hr = _plDomDocument.CreateInstance(CLSID_DOMDocument);
	if (FAILED(hr)) {
		// to be 100% accurate, we need to use _com_error er(hr) and then
		// _com_error::ErrorMessage() method to get the error...
		//printf("Creating XML DOM failed\n");
		return 1;
	}
	_wasInitialized = true;
	return NO_ERROR;
}

BBRETCODE CBBWinXmlParser::LoadDocFromFile(const char *fileName)
{
	if (!_wasInitialized) {
		if (_isVerboseMode) {
			printf(_FUNC_ "ERROR: Cannot load document, XML parser was not initialized!\n");
		}
		return 1;
	}
	VARIANT_BOOL vResult = _plDomDocument->load(fileName);
	if (vResult == VARIANT_FALSE) {
		// TODO: get the real error message
		//printf(_FUNC_ "ERROR: Failed to read XML file (check that file exists and is properly structured)\n");
		return 2;
	}
			
	// now that the document is loaded, we need to initialize the root pointer
	_pDocRoot = _plDomDocument->documentElement;
	_stprintf(_outermostTagName, _T("%s"), (LPCTSTR)bstr_t(_pDocRoot->GetbaseName()));
	_wasDocumentRead = true;
	return NO_ERROR;
}


BBRETCODE CBBWinXmlParser::ReadDocFromString(const char *xmlText)
{
	if (!_wasInitialized) {
		if (_isVerboseMode) {
			printf(_FUNC_ "ERROR: Cannot load document, XML parser was not initialized!\n");
		}
		return 1;
	}	
	_bstr_t bstrt(xmlText);
	VARIANT_BOOL vResult = _plDomDocument->loadXML(bstrt);
	if (vResult == VARIANT_FALSE) {
		// TODO: get the real error message
		if (_isVerboseMode) {
			printf(_FUNC_ "ERROR: Incorrect XML structure\n");
		}
		return 2;
	}
			
	// now that the document is loaded, we need to initialize the root pointer
	_pDocRoot = _plDomDocument->documentElement;
	_wasDocumentRead = true;
	return NO_ERROR;
}

const char *CBBWinXmlParser::GetOutermostTagName()
{
	return _outermostTagName;
}

BBRETCODE CBBWinXmlParser::GetAttributeString(const char *attributePath, char *outValue, bool printErr)
{
	VARIANT value;
	if (!_wasDocumentRead) {
		if (_isVerboseMode) {
			printf(_FUNC_ "ERROR: Document not loaded/read\n");
		}
		return 9; // document not loaded
	}
	if (GetAttributeNode(attributePath, value, printErr) != NO_ERROR) return 1; // node does not exist
	_stprintf(outValue, _T("%s"), (LPCTSTR)bstr_t(value));
	return NO_ERROR;
}

BBRETCODE CBBWinXmlParser::GetAttributeDouble(const char *attributePath, double &outValue, bool printErr)
{
	char tempStr[maxNumericAttributeLength];
	VARIANT value;
	if (GetAttributeNode(attributePath, value, printErr) != NO_ERROR) return 1;
	_stprintf(tempStr, _T("%s"), (LPCTSTR)bstr_t(value));
	outValue = atof(tempStr);
	return NO_ERROR;
}

BBRETCODE CBBWinXmlParser::GetAttributeInt(const char *attributePath, int &outValue, bool printErr)
{
	char tempStr[maxNumericAttributeLength];
	VARIANT value;
	if (GetAttributeNode(attributePath, value, printErr) != NO_ERROR) return 1;
	_stprintf(tempStr, _T("%s"), (LPCTSTR)bstr_t(value));
	outValue = atoi(tempStr);
	return NO_ERROR;
}

BBRETCODE CBBWinXmlParser::GetAttributeNode(const char *attributePath, VARIANT &value, bool printErr)
{
	MSXML2::IXMLDOMNodePtr pNode = _pDocRoot->selectSingleNode(attributePath);
	if (pNode == NULL) {
		if (printErr) {
			printf(_FUNC_ "ERROR: Did not find attribute '%s'\n", attributePath);
		}
		return 1;
	}
	pNode->get_nodeValue(&value);
	return NO_ERROR;
}
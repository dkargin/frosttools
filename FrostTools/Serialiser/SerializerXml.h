//----------------------------------------------------------)
//
//	SerializerXml.h
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#ifndef __SerializerXml_h__
#define __SerializerXml_h__

#include <msxml2.h>

#ifndef __SerializerUtils_h__
#	include "SerializerUtils.h"
#endif

#ifndef __Serializer_h__
#	include "Serializer.h"
#endif

#ifndef __SerializerPrim_h__
#	include "SerializerPrim.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn      {
namespace Serializer{
namespace Tree      {
namespace Xml       {

class NodeArrayImpl;
class NamedNodeListImpl;

//----------------------------------------------------------)

class NodeImpl 
	: public INode
	, public CRefcounted
{
public:
	NodeImpl(IXMLDOMNode* pNode);

	INamedNodeList* ChildElements() const;
	INodeArray*     ChildElements(const tstring& name) const;
	INamedNodeList* Attributes   () const;

	INode*  AddElement  (const tstring& tag, long index);
	INode*  AddAttribute(const tstring& name);

	tstring ReadText() const;
	void WriteText(const tstring& text);

protected:
	CComPtr<IXMLDOMNode> m_pXmlNode;
};
//----------------------------------------------------------)

class NamedNodeListImpl	: public Serializer::INamedNodeList
{
	typedef std::map<tstring, CPtrShared<NodeImpl> > NamedNodes; 
public:
	NamedNodeListImpl(IXMLDOMNodeList*);
	NamedNodeListImpl(IXMLDOMNamedNodeMap*);

	INode* GetByName(const tstring& name) const;

protected:
	NamedNodes m_mapNamedNodes;
};
//----------------------------------------------------------)

class NodeArrayImpl : public Serializer::INodeArray
{
public:
	NodeArrayImpl(IXMLDOMNodeList* pNodeList);

	long Count() const
		{ return (long)m_nodes.size(); }
	INode* GetByIndex(long index) const 
		{ return m_nodes[index]; }
protected:
	std::vector<CPtrShared<NodeImpl> > m_nodes;
};
//----------------------------------------------------------)

// Ошибки при работе с деревянным хранилищем

struct XmlStorageException : public StorageException
{
	XmlStorageException(const tstring& a_msg) 
		: StorageException(a_msg) 
	{}

	XmlStorageException() 
		: StorageException(ErrorInfo()) 
	{}
};
//----------------------------------------------------------)

template <typename DataType>
void Load( const tstring&            pathFile
		 , const tstring&            rootTag
		 , DataType*                 pData
		 , const MetaClass<DataType>& metaData = DefaultMetaClass<DataType>()
		 )
{
	CComPtr<IXMLDOMDocument> pDoc;
	if (FAILED(pDoc.CoCreateInstance(L"Msxml2.DOMDocument")))
		throw XmlStorageException();

	if (FAILED(pDoc->put_async(VARIANT_FALSE)))
		throw XmlStorageException();

	VARIANT_BOOL success;
	if (FAILED(pDoc->load(CComVariant(pathFile.c_str()), &success)))
		throw XmlStorageException();

	if (success != VARIANT_TRUE)
		throw XmlStorageException(_T("Msxml can't load xml file"));

	CComPtr<IXMLDOMElement> pRootElement;
	if (FAILED(pDoc->get_documentElement(&pRootElement)))
		throw XmlStorageException();

	CComBSTR nameRoot;
	if (FAILED(pRootElement->get_nodeName(&nameRoot)))
		throw XmlStorageException();

	if (CComBSTR(rootTag.c_str()) != nameRoot)
		throw XmlStorageException(_T("Incorrect type of xml documet"));

	*pData = metaData.ReadNode(NodeImpl(pRootElement));
}
//----------------------------------------------------------)

template<typename DataType>
void Save( const tstring&            pathFile
		 , const tstring&            rootTag
		 , const DataType&           data
		 , const MetaClass<DataType>& metaData = DefaultMetaClass<DataType>()
		 )
{
	CComPtr<IXMLDOMDocument> pDoc;
	if (FAILED(pDoc.CoCreateInstance(L"Msxml2.DOMDocument")))
		throw XmlStorageException();

	if (FAILED(pDoc->put_async(VARIANT_FALSE)))
		throw XmlStorageException();

	CComPtr<IXMLDOMProcessingInstruction> pProcessingInstruction;
	if (FAILED(pDoc->createProcessingInstruction(
		  CComBSTR("xml")
		, CComBSTR("version=\"1.0\" encoding=\"utf-8\"")
		, &pProcessingInstruction)))
	{
		throw XmlStorageException();
	}
	if (FAILED(pDoc->appendChild(pProcessingInstruction, NULL)))
		throw XmlStorageException();

	CComPtr<IXMLDOMElement> pRootElement;
	if (FAILED(pDoc->createElement(CComBSTR(rootTag.c_str()), &pRootElement)))
		throw XmlStorageException();

	if (FAILED(pDoc->putref_documentElement(pRootElement)))
		throw XmlStorageException();

	NodeImpl node(pRootElement);
	metaData.WriteNode(&node, data);

	if (FAILED(pRootElement->normalize()))
		throw XmlStorageException();

	////////////////////////////
	//
	// http://www.rsdn.ru/forum/Message.aspx?mid=70479
	//
	CComBSTR xml;
	if (FAILED(pDoc->get_xml(&xml)))
		throw XmlStorageException();

    CString strXml(xml);
    strXml.Replace(_T("><"), _T(">\n<"));
    xml = strXml;

	VARIANT_BOOL success;
	if (FAILED(pDoc->loadXML(xml, &success)))
		throw XmlStorageException();
	if (success != VARIANT_TRUE)
		throw XmlStorageException(_T("Msxml internal error"));

	/////////////////

	if (FAILED(pDoc->save(CComVariant(pathFile.c_str()))))
		throw XmlStorageException();
}

} // namespace Xml {
} // namespace TreeStorage {
} // namespace Serializer {
} // namespace Rsdn {

#endif  // __MXmlTree_h__
//==========================================================)
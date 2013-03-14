//----------------------------------------------------------)
//
//	MSerializerXml.cpp
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#include "stdafx.h"
#include <msxml2.h>
#include "SerializerXml.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn       {
namespace Serializer {
namespace Tree       {
namespace Xml        {

//----------------------------------------------------------)

NodeArrayImpl::NodeArrayImpl(IXMLDOMNodeList* pNodeList)
{
	long length;
	if (FAILED(pNodeList->get_length(&length)))
		throw XmlStorageException();

	for (int idx = 0; idx != length; idx++)
	{
		CComPtr<IXMLDOMNode> pXmlNode;
		if (FAILED(pNodeList->get_item(idx, &pXmlNode)))
			throw XmlStorageException();

		m_nodes.push_back(new NodeImpl(pXmlNode));
	}
}
//----------------------------------------------------------)
//----------------------------------------------------------)

NamedNodeListImpl::NamedNodeListImpl(IXMLDOMNodeList* pNodeList)
{
	long length;
	if (FAILED(pNodeList->get_length(&length)))
		throw XmlStorageException();

	for (long idx = 0; idx != length; idx++)
	{
		CComPtr<IXMLDOMNode> pNode; 
		if (FAILED(pNodeList->get_item(idx, &pNode)))
			throw XmlStorageException();

		CComBSTR name; 
		if (FAILED(pNode->get_nodeName(&name)))
			throw XmlStorageException();

		m_mapNamedNodes[tstring(CW2T(name))] = new NodeImpl(pNode);
	}
}

NamedNodeListImpl::NamedNodeListImpl(IXMLDOMNamedNodeMap* pNodeList)
{
	long length;
	if (FAILED(pNodeList->get_length(&length)))
		throw XmlStorageException();

	for (long idx = 0; idx != length; idx++)
	{
		CComPtr<IXMLDOMNode> pNode; 
		if (FAILED(pNodeList->get_item(idx, &pNode)))
			throw XmlStorageException();

		CComBSTR name; 
		if (FAILED(pNode->get_nodeName(&name)))
			throw XmlStorageException();

		m_mapNamedNodes[tstring(CW2T(name))] = new NodeImpl(pNode);
	}
}

Serializer::INode* NamedNodeListImpl::GetByName(const tstring& name) const
{
	NamedNodes::const_iterator it = m_mapNamedNodes.find(name);
	if (it == m_mapNamedNodes.end())
		return NULL;
	return it->second;
}
//----------------------------------------------------------)
//----------------------------------------------------------)

NodeImpl::NodeImpl(IXMLDOMNode* pNode)
		: m_pXmlNode(pNode)
	{}

INode* NodeImpl::AddElement( const tstring& tag                             
                           , long        // index
                           )
{
	CComPtr<IXMLDOMDocument> pDoc;
	if (FAILED(m_pXmlNode->get_ownerDocument(&pDoc)))
		throw XmlStorageException();

	CComPtr<IXMLDOMElement> pElement;
	if (FAILED(pDoc->createElement(CComBSTR(tag.c_str()), &pElement)))
		throw XmlStorageException();

	if (FAILED(m_pXmlNode->appendChild(pElement, NULL)))
		throw XmlStorageException();

	return new NodeImpl(pElement);
}

INode* NodeImpl::AddAttribute( const tstring& tag)
{
	HRESULT hr;

	DOMNodeType nodeType;
	if (FAILED(m_pXmlNode->get_nodeType(&nodeType)))
		throw XmlStorageException();

	assert(nodeType == NODE_ELEMENT);

	CComPtr<IXMLDOMElement> pElement;
	if (FAILED(m_pXmlNode.QueryInterface(&pElement)))
		throw XmlStorageException();

	CComPtr<IXMLDOMDocument> pDoc;
	if (FAILED(m_pXmlNode->get_ownerDocument(&pDoc)))
		throw XmlStorageException();

	CComPtr<IXMLDOMAttribute> pAttribute;
	if (FAILED(pDoc->createAttribute(CComBSTR(tag.c_str()), &pAttribute)))
		throw XmlStorageException();


	hr = pElement->setAttributeNode(pAttribute, NULL);
	if (FAILED(hr))
		throw XmlStorageException();

	return new NodeImpl(pAttribute);
}

Serializer::INamedNodeList* NodeImpl::ChildElements() const
{
	CComPtr<IXMLDOMNodeList> pXmlNodeList;
	if (FAILED(m_pXmlNode->get_childNodes(&pXmlNodeList)))
		throw XmlStorageException();
	return new NamedNodeListImpl(pXmlNodeList);
}

Serializer::INodeArray* NodeImpl::ChildElements(const tstring& name) const
{
	CComPtr<IXMLDOMNodeList> pXmlNodeList;
	if (FAILED(m_pXmlNode->selectNodes(CComBSTR(name.c_str()), &pXmlNodeList)))
		throw XmlStorageException();
	return new NodeArrayImpl(pXmlNodeList);
}
		
Serializer::INamedNodeList* NodeImpl::Attributes() const
{
	CComPtr<IXMLDOMNamedNodeMap> pNamedNodeMap;
	if (FAILED(m_pXmlNode->get_attributes(&pNamedNodeMap)))
		throw XmlStorageException();
	return new NamedNodeListImpl(pNamedNodeMap);
}

tstring NodeImpl::ReadText() const
{
	CComBSTR text;
	if (FAILED(m_pXmlNode->get_text(&text)))
		throw XmlStorageException();
	return (LPCTSTR)CW2CT(text);
}

void NodeImpl::WriteText(const tstring& text)
{
	m_pXmlNode->put_text(CComBSTR(text.c_str()));
}

} // namespace Xml {
} // namespace TreeStorage {
} //namespace Serializer {
} // namespace Rsdn {
//==========================================================)
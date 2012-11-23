//----------------------------------------------------------)
//
//	SerializerXml.cpp
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#include "stdafx.h"
#include "SerializerRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn      {
namespace Serializer{
namespace Tree      {
namespace Registry  {

//----------------------------------------------------------)

long NodeArrayImpl::Count() const
{
	return (long)m_nodes.size();
}

Serializer::INode* NodeArrayImpl::GetByIndex(long index) const
{
	return m_nodes[index];
}
//----------------------------------------------------------)
//----------------------------------------------------------)

Serializer::INode* NamedNodeListImpl::GetByName(const tstring& name) const
{
	NamedNodes::const_iterator it = m_mapNamedNodes.find(name);
	if (it == m_mapNamedNodes.end())
		return NULL;
	return it->second;
}
//----------------------------------------------------------)
//----------------------------------------------------------)
/*
NodeImpl::NodeImpl(NodeImpl& node)
	: m_nameAttribute(node.m_nameAttribute)
	, m_key(node.m_key)
{
}
*/

NodeImpl::NodeImpl ( bool           bNeedCreate
				   , HKEY           hKeyParent
				   , const tstring& nameSubKey
				   , const tstring& nameValue)
	: m_nameAttribute(nameValue)
	, m_pKeyParent(NULL)
{
	LONG ret;
	ret = bNeedCreate ? m_key.Create(hKeyParent, nameSubKey.c_str())
		              : m_key.Open  (hKeyParent, nameSubKey.c_str());
	if (ret != ERROR_SUCCESS)
		throw RegistryStorageException(ret);
}

NodeImpl::NodeImpl(CRegKey* pKey, const tstring& nameValue)
	: m_nameAttribute(nameValue)
	, m_pKeyParent(pKey)
{
}

inline	bool NodeImpl::IsAttribute() const
{
	return ! m_nameAttribute.empty();
}

INode* NodeImpl::AddElement( const tstring& tag, long index)
{
	if (IsAttribute())
		throw RegistryStorageException(_T("can't add element to attribute"));

	return new NodeImpl(true, m_key
		, tag + ((index != -1) ? (_T("_") + ItoT(index)) : _T("")));
}

INode* NodeImpl::AddAttribute(const tstring& nameValue)
{
	if (IsAttribute())
		throw RegistryStorageException(_T("can't add attribute to attribute"));

	LONG ret = m_key.SetStringValue(nameValue.c_str(), _T(""));
	if (ret != ERROR_SUCCESS)
		throw RegistryStorageException(ret);

	return new NodeImpl(&m_key, nameValue);
}

Serializer::INamedNodeList* NodeImpl::ChildElements() const
{
	if (IsAttribute())
		throw RegistryStorageException(_T("attribute has no child elements"));

	std::auto_ptr<NamedNodeListImpl> pNamedNodeList(new NamedNodeListImpl);
	for (int idx = 0; ; idx++)
	{
		std::vector<tchar> nameSubKey(1024, '\0');
		DWORD size = (DWORD)nameSubKey.size();
		LONG ret = const_cast<NodeImpl*>(this)->m_key.EnumKey(idx
			                                      , &nameSubKey[0], &size);
		if (ret == ERROR_NO_MORE_ITEMS)
			return pNamedNodeList.release();
		if (ret != ERROR_SUCCESS)
			throw RegistryStorageException(ret);

		pNamedNodeList->m_mapNamedNodes[&nameSubKey[0]] = new NodeImpl(false, m_key, &nameSubKey[0]);
	}
	throw RegistryStorageException(_T("EnumKey error"));
}

Serializer::INodeArray* NodeImpl::ChildElements(const tstring& nameElement) const
{
	if (IsAttribute())
		throw RegistryStorageException(_T("attribute has no child elements"));

	std::auto_ptr<NodeArrayImpl> pNodeArray(new NodeArrayImpl);
	for (int idx = 0; pNodeArray->m_nodes.size() == idx; idx++)
	{
		for (int idxKey = 0; ; idxKey++)
		{
			std::vector<tchar> buff(1024, '\0');
			DWORD size = (DWORD)buff.size();
			LONG ret = const_cast<NodeImpl*>(this)->m_key.EnumKey(idxKey, &buff[0], &size);
			if (ret == ERROR_NO_MORE_ITEMS)
				break;

			if (ret != ERROR_SUCCESS)
				throw RegistryStorageException(ret);

			tstring nameSubKey(&buff[0]);
			if (nameSubKey.find(nameElement) != 0)
				continue;

			tstring strIndex = nameSubKey.substr(nameElement.size() + 1); 
			if (_tstoi(strIndex.c_str()) == idx)
			{
				pNodeArray->m_nodes.push_back(new NodeImpl(false, m_key, nameSubKey));
				break;
			}
		}
	}
	return pNodeArray.release();
}
		
Serializer::INamedNodeList* NodeImpl::Attributes() const
{
	if (IsAttribute())
		throw RegistryStorageException(_T("Attribute has no attributes"));

	std::auto_ptr<NamedNodeListImpl> pNamedNodeList(new NamedNodeListImpl);
	for (int idx = 0; ; idx++)
	{
		std::vector<tchar> nameSubKey(1024, '\0');
		DWORD size = (DWORD)nameSubKey.size();
		LONG ret = ::RegEnumValue(m_key.m_hKey, idx, &nameSubKey[0], &size, 0, NULL, NULL, NULL);
		if (ret == ERROR_NO_MORE_ITEMS)
			return pNamedNodeList.release();
		if (ret != ERROR_SUCCESS)
			throw RegistryStorageException(ret);

		pNamedNodeList->m_mapNamedNodes[&nameSubKey[0]] 
			= new NodeImpl(&const_cast<NodeImpl*>(this)->m_key, &nameSubKey[0]);
	}
	throw RegistryStorageException(_T("RegEnumValue error"));
}

tstring NodeImpl::ReadText() const
{
	std::vector<tchar> buff(1024, '\0');
	DWORD size = (DWORD)buff.size();
	CRegKey& key = IsAttribute() ? *const_cast<NodeImpl*>(this)->m_pKeyParent
	                             :  const_cast<NodeImpl*>(this)->m_key;
	LONG ret = key.QueryStringValue(m_nameAttribute.c_str(), &buff[0], &size);
	if (ret != ERROR_SUCCESS)
		throw RegistryStorageException(ret);
	return &buff[0];
}

void NodeImpl::WriteText(const tstring& text)
{
	CRegKey& key = IsAttribute() ? *m_pKeyParent : m_key;
	LONG ret = key.SetStringValue(m_nameAttribute.c_str(), text.c_str());
	if (ret != ERROR_SUCCESS)
		throw RegistryStorageException(ret);
}

} // namespace Registry    (
} // namespace Tree        {
} // namespace Serializer  {
} // namespace Rsdn        {
//==========================================================)
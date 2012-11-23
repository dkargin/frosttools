//----------------------------------------------------------)
//
//	SerializerRegistry.h
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#ifndef __SerializerRegistry_h__
#define __SerializerRegistry_h__

#ifndef __Serializer_h__
#	include "Serializer.h"
#endif

#ifndef __Util_h__
#	include "Util.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn      {
namespace Serializer{
namespace Tree      {
namespace Registry  {

using namespace Utils;
class NodeImpl;

//----------------------------------------------------------)

class NamedNodeListImpl	: public Serializer::INamedNodeList
{
	typedef std::map<tstring, CPtrShared<NodeImpl> > NamedNodes; 
public:
	INode* GetByName(const tstring& name) const;

protected: friend class NodeImpl; 
	NamedNodes m_mapNamedNodes;
};
//----------------------------------------------------------)

class NodeArrayImpl : public Serializer::INodeArray
{
public:
	long Count() const;
	INode* GetByIndex(long index) const;

protected: friend class NodeImpl; 
	std::vector<CPtrShared<NodeImpl> > m_nodes;
};
//----------------------------------------------------------)

class NodeImpl 
	: public Serializer::INode
	, public CRefcounted
{
public:
	NodeImpl(const NodeImpl&);
	NodeImpl(bool create, HKEY, const tstring& nameSubKey, const tstring& nameValue = _T(""));
	NodeImpl(CRegKey*, const tstring& nameValue);

	INamedNodeList* ChildElements() const;
	INodeArray*     ChildElements(const tstring& name) const;
	INamedNodeList* Attributes   () const;

	INode* AddElement  (const tstring& tag, long index);
	INode* AddAttribute(const tstring& name);

	tstring ReadText() const;
	void WriteText(const tstring& text);

protected:
	bool    IsAttribute() const;

	CRegKey  m_key;       // invalid if IsAttribute

	const tstring  m_nameAttribute;
	CRegKey* const m_pKeyParent; // valid if IsAttribute
};
//----------------------------------------------------------)

// Ошибки при работе с деревянным хранилищем

struct RegistryStorageException : public StorageException
{
	RegistryStorageException(const tstring& a_msg) 
		: StorageException(a_msg) 
	{}

	RegistryStorageException(LONG errCode) 
		: StorageException(ErrorMsg((uint)errCode)) 
	{}
};
//----------------------------------------------------------)

template <typename DataType>
void Load( HKEY                      hKeyParent
		 , const tstring&            keyName
		 , DataType*                 pData
		 , const MetaClass<DataType>& metaData = DefaultMetaClass<DataType>()
		 )
{
	NodeImpl node(false, hKeyParent, keyName);
	*pData = metaData.ReadNode(node);
}
//----------------------------------------------------------)

template<typename DataType>
void Save( HKEY                      hKeyParent
		 , const tstring&            keyName
		 , const DataType&           data
		 , const MetaClass<DataType>& metaData = DefaultMetaClass<DataType>()
		 )
{
	NodeImpl node(true, hKeyParent, keyName);
	metaData.WriteNode(&node, data);
}

} // namespace Xml {
} // namespace Tree  {
} // namespace Serializer {
} // namespace Rsdn {

#endif  // __MSerializeRegistry_h__
//==========================================================)
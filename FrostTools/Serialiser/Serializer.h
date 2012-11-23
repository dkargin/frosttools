//----------------------------------------------------------)
//
//	Serializer.h
//
//	Designed by Andrey Martynov
//	mailto:rek@.rsdn.ru
//
//----------------------------------------------------------)

#ifndef __Serializer_h__
#define __Serializer_h__

#ifndef __SerializerUtils_h__
#	include "SerializerUtils.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Rsdn      {
namespace Serializer {

using namespace Utils;
//----------------------------------------------------------)

namespace Tree
{
interface INode;

//----------------------------------------------------------)

interface INamedNodeList
{
	virtual ~INamedNodeList(){}
	virtual INode* GetByName(const tstring& name) const = 0;
};
//----------------------------------------------------------)

interface INodeArray
{
	virtual ~INodeArray(){}
	virtual long   Count() const = 0;
	virtual INode* GetByIndex(long index) const = 0;
};
//----------------------------------------------------------)

struct INode // Абстрактная ветка (элемент или атрибут)
{
	virtual ~INode(){}

	virtual tstring ReadText() const = 0;
	virtual void WriteText(const tstring&) = 0;

	virtual INodeArray*     ChildElements(const tstring& name) const = 0;
	virtual INamedNodeList* ChildElements() const = 0;
	virtual INamedNodeList* Attributes() const = 0;

	virtual INode* AddElement  (const tstring& tag, long index = -1) = 0;
	virtual INode* AddAttribute(const tstring& name) = 0;
};

// Ошибка при работе с дереявнным хранилищем
struct StorageException : public Exception
{
	StorageException(const tstring& a_msg) 
		: Exception(a_msg) 
	{}
};

} // namespace Storage 
//----------------------------------------------------------)

namespace Primitives      {}
namespace FieldAttrubutes {}
namespace MetaClasses     {}

using namespace Primitives;
using namespace FieldAttrubutes;
using namespace MetaClasses;
using namespace Tree;

namespace FieldAttrubutes
{
//----------------------------------------------------------)
//
//	MetaData - данные, описывающие положение поля внутри структуры.
//		Одно метаданное обеспечивают заполнение/чтение данных одного 
//		поля в нутри структуры (IFieldMetaData). 
//
//	Поддерживается три типа метаданых - метаданые примитива, метаданые структуры, метаданые массива.
//
//	- Метаданые примитива (наследники PrimMetaData<StructType, PrimFieldOfStruct, PrimSerializer>). 
//		Не нужны другие метаданные, но нужен сериализатор. 
//		Есть два типа метаданых примитива - атрибутный и элементный 
//		(PrimAttributeMetaData и PrimElementMetaData). 
//
//	- Метаданые массива (VectorMetaData<StructType, ItemType>). 
//		Включает метаданные элемента. 
//
//	- Метаданые cтруктуры (StructMetaData<StructType, FieldOfStruct>). 
//		Включает метаданные всех полей. 
//
//----------------------------------------------------------)

	typedef const tchar* FieldName;

	template <typename StructType>
	struct FieldAttrubute : public CRefcounted
	{
		virtual void ReadField (const INode&, StructType*) = 0;
		virtual void WriteField(INode*, const StructType&) = 0;
	};

	template <typename StructType, typename FieldType>
	struct FieldBaseAttrubute : public FieldAttrubute<StructType>
	{
		FieldBaseAttrubute(FieldName name, FieldType StructType::* offset)
			: m_name(name), m_offset(offset)
			{}
		FieldName                     m_name;
		FieldType StructType::* const m_offset;
	};

	template <typename StructType, typename PrimFieldType>
	struct PrimFieldAttrubute;

} // namespace Fields  {
//----------------------------------------------------------)

namespace MetaClasses 
{
	typedef const tchar* ArrayItemName;

	template <typename Data>
	struct MetaClass
	{
		virtual Data ReadNode (const INode&) const = 0;
		virtual void WriteNode(INode*, const Data&) const = 0;
	};
}
//----------------------------------------------------------)

namespace FieldAttrubutes
{
	template <typename StructType>
	struct Layout : std::vector<CPtrShared<FieldAttrubute<StructType> > >
	{
		template <typename PrimFieldType>
		void Simple( FieldName                   name
		           , PrimFieldType StructType::* offset
		           , PrimFieldType               def
		           )
		{ 
			push_back(new PrimFieldAttrubute<StructType, PrimFieldType>(name, offset, def)); 
		}

		template <typename PrimFieldType>
		void SimpleM( FieldName                   name
		            , PrimFieldType StructType::* offset
		           )
		{ 
			push_back(new PrimFieldAttrubute<StructType, PrimFieldType>(name, offset)); 
		}

		template <typename PrimFieldType>
		void Attribute( FieldName                   name
		              , PrimFieldType StructType::* offset
		              , PrimFieldType               def
		              )
		{ 
			push_back(new PrimAttrFieldAttrubute<StructType, PrimFieldType>(name, offset, def)); 
		}

		template <typename PrimFieldType>
		void AttributeM( FieldName                   name
		               , PrimFieldType StructType::* offset
		               )
		{ 
			push_back(new PrimAttrFieldAttrubute<StructType, PrimFieldType>(name, offset)); 
		}

		template <typename FieldType>
		void Complex( FieldName                name
		            , FieldType StructType::*  offset
		            , FieldType                def
		            , const Layout<FieldType>& layout = DefaultLayout<FieldType>()
		            )
		{ 
			push_back(new StructFieldAttrubute<StructType, FieldType>(name, offset, def, layout));
		}

		template <typename FieldType>
		void ComplexM( FieldName                name
		            , FieldType StructType::*  offset
		            , const Layout<FieldType>& layout = DefaultLayout<FieldType>()
		            )
		{ 
			push_back(new StructFieldAttrubute<StructType, FieldType>(name, offset, layout));
		}

		template <typename ItemType>
		void Array( FieldName                           fieldName
		          , std::vector<ItemType> StructType::* offset
				  , ArrayItemName                       itemName
				  , const MetaClass<ItemType>&           metaDataItem = DefaultMetaClass<ItemType>()
				  )
		{ 
			push_back(new VectorFieldAttrubute<StructType, ItemType>(fieldName, itemName, offset, metaDataItem));
		}
	};

	// отсутствует обязательное поле
	struct MondatoryFieldException : public Exception
	{
		MondatoryFieldException(const tstring& a_msg) 
			: Exception(a_msg) 
		{}
	};
} // namespace Fields  {
//----------------------------------------------------------)

template <typename DataType>
const Layout<DataType>& DefaultLayout()
{
	static DataType::LayoutDefault g_layout;
	return g_layout;
}

template <typename Data>
const MetaClass<Data>& DefaultMetaClass()
{
	static StructClassMeta<Data> g_metaData(DefaultLayout<Data>());
	return g_metaData;
}

template <typename ArrayItemType>
const MetaClass<std::vector<ArrayItemType> >& DefaultVectorMetaClass(ArrayItemName itemName)
{
	static VectorClassMeta<ArrayItemType> g_metaData(itemName, DefaultMetaClass<ArrayItemType>());
	return g_metaData;
}
//----------------------------------------------------------)
namespace FieldAttrubutes {

template <typename StructType, typename PrimFieldType>
struct PrimBaseFieldAttrubute
	: public FieldBaseAttrubute<StructType, PrimFieldType>
{
	PrimBaseFieldAttrubute( FieldName       name
		      , PrimFieldType StructType::* offset
		      , PrimFieldType               def
		      )
		: FieldBaseAttrubute<StructType, PrimFieldType>(name, offset)
		, m_default(def), m_mandatory(false) 
	{}

	PrimBaseFieldAttrubute( FieldName       name
		      , PrimFieldType StructType::* offset
		      )
		: FieldBaseAttrubute<StructType, PrimFieldType>(name, offset)
		, m_default(), m_mandatory(true) 
	{}

	void ReadField(const INode& node, StructType* pD)
	{
		std::auto_ptr<INamedNodeList> nodeList (GetNodes(node));
		INode* pNodeChild = nodeList->GetByName(m_name);
		if (pNodeChild != NULL)
			pD->*m_offset = PrimClassMeta<PrimFieldType>().ReadNode(*pNodeChild);
		else
		{
			if (m_mandatory)
				throw MondatoryFieldException(m_name);
			pD->*m_offset = m_default;
		}
	}

	void WriteField(INode* pNode, const StructType& d)
	{
		std::auto_ptr<INamedNodeList> nodes    (GetNodes(*pNode));
		INode* pNodeChild = nodes->GetByName(m_name);
		if (pNodeChild != NULL)
			PrimClassMeta<PrimFieldType>().WriteNode(pNodeChild, d.*m_offset);
		else
		{
			std::auto_ptr<INode> pNewNodeChild(AddNode(pNode, m_name));
			PrimClassMeta<PrimFieldType>().WriteNode(pNewNodeChild.get(), d.*m_offset);
		}
	}

	virtual INamedNodeList* GetNodes(const INode&  node) = 0;
	virtual INode*          AddNode (INode* pNode, const tstring& tag) = 0;

protected:
	bool				m_mandatory;
	const PrimFieldType m_default;
};
//----------------------------------------------------------)

template <typename StructType, typename PrimFieldType>
struct PrimAttrFieldAttrubute
	: public PrimBaseFieldAttrubute<StructType, PrimFieldType>
{
	PrimAttrFieldAttrubute( FieldName              name
		             , PrimFieldType StructType::* offset
		             , PrimFieldType               def
		             )
		: PrimBaseFieldAttrubute<StructType, PrimFieldType>(name, offset, def) 
	{}

	PrimAttrFieldAttrubute(FieldName name, PrimFieldType StructType::* offset)
		: PrimBaseFieldAttrubute<StructType, PrimFieldType>(name, offset) 
	{}

	INamedNodeList* GetNodes(const INode& node)
	{
		return node.Attributes();
	}

	INode* AddNode (INode* pNode, const tstring& tag)
	{
		return pNode->AddAttribute(tag);
	}
};
//----------------------------------------------------------)

template <typename StructType, typename PrimFieldType>
struct PrimFieldAttrubute
	: public PrimBaseFieldAttrubute<StructType, PrimFieldType>
{
	PrimFieldAttrubute( FieldName                   name
	                  , PrimFieldType StructType::* offset
	                  , PrimFieldType               def
	                  )
		: PrimBaseFieldAttrubute<StructType, PrimFieldType>(name, offset, def) 
	{}

	PrimFieldAttrubute(FieldName name, PrimFieldType StructType::* offset)
		: PrimBaseFieldAttrubute<StructType, PrimFieldType>(name, offset) 
	{}

	INamedNodeList* GetNodes(const INode& node)
	{
		return node.ChildElements();
	}

	INode* AddNode (INode* pNode, const tstring& tag)
	{
		return pNode->AddElement(tag);
	}
};
//----------------------------------------------------------)

template <typename StructType, typename StructFieldType>
struct StructFieldAttrubute
	: public FieldBaseAttrubute<StructType, StructFieldType>
{
	StructFieldAttrubute( FieldName             name
		       , StructFieldType StructType::*  offset
		       , StructFieldType                def
			   , const Layout<StructFieldType>& layout   = DefaultLayout<StructFieldType>()
		       )
		: FieldBaseAttrubute<StructType, StructFieldType>(name, offset)
		, m_layout(layout)
		, m_default(def), m_mandatory(false)
	{}

	StructFieldAttrubute( FieldName             name
		       , StructFieldType StructType::*  offset
			   , const Layout<StructFieldType>& layout   = DefaultLayout<StructFieldType>()
		       )
		: FieldBaseAttrubute<StructType, StructFieldType>(name, offset)
		, m_layout(layout)
		, m_default(), m_mandatory(true)
	{}

	void ReadField(const INode& node, StructType* pD)
	{
		std::auto_ptr<INamedNodeList> pNodeList (node.ChildElements());
		INode* pNodeChild = pNodeList->GetByName(m_name);
		if (pNodeChild != NULL)
			pD->*m_offset = StructClassMeta<StructFieldType>(m_layout).ReadNode(*pNodeChild);
		else
		{
			if (m_mandatory)
				throw MondatoryFieldException(m_name);
			pD->*m_offset = m_default;
		}
	}

	void WriteField(INode* pNode, const StructType& d)
	{
		std::auto_ptr<INamedNodeList> pNodeList (pNode->ChildElements());
		INode* pNodeChild = pNodeList->GetByName(m_name);
		if (pNodeChild != NULL)
			StructClassMeta<StructFieldType>(m_layout).WriteNode(pNodeChild, d.*m_offset);
		else
		{   std::auto_ptr<INode> pNewNodeChild(pNode->AddElement(m_name));
			StructClassMeta<StructFieldType>(m_layout).WriteNode(pNewNodeChild.get(), d.*m_offset);
		}
	}

protected:
	bool				           m_mandatory;
	const StructFieldType          m_default;
	const Layout<StructFieldType>& m_layout;
};
//----------------------------------------------------------)

template <typename StructType, typename ItemType>
struct VectorFieldAttrubute
	: public FieldBaseAttrubute<StructType, std::vector<ItemType> >
{
	VectorFieldAttrubute( FieldName                           name 
		                , ArrayItemName                       itemName
		                , std::vector<ItemType> StructType::* offset
			            , const MetaClass<ItemType>&          metaData = DefaultMetaClass<ItemType>()
		                )
		: FieldBaseAttrubute<StructType, std::vector<ItemType> >(name, offset)
		, m_itemName(itemName)
		, m_metaData(metaData)
	{}

	void ReadField(const INode& node, StructType* pD)
	{
		std::auto_ptr<INamedNodeList> pNodeList(node.ChildElements());
		INode* pNodeChild = pNodeList->GetByName(m_name);
		if (pNodeChild == NULL)
			pD->*m_offset = std::vector<ItemType>();
		else
			pD->*m_offset = VectorClassMeta<ItemType>(m_itemName, m_metaData)
									.ReadNode(*pNodeChild);
	}

	void WriteField(INode* pNode, const StructType& d)
	{
		std::auto_ptr<INamedNodeList> pNodeList(pNode->ChildElements());
		INode* pNodeChild = pNodeList->GetByName(m_name);
		if (pNodeChild != NULL)
		{
			VectorClassMeta<ItemType>(m_itemName, m_metaData)
				.WriteNode(pNodeChild, d.*m_offset);
		}
		else
		{	std::auto_ptr<INode> pNewNodeChild(pNode->AddElement(m_name));
			VectorClassMeta<ItemType>(m_itemName, m_metaData)
				.WriteNode(pNewNodeChild.get(), d.*m_offset);
		}

	}
private:
	ArrayItemName              m_itemName;
	const MetaClass<ItemType>& m_metaData;
};
} // namespace StructLayout {
//----------------------------------------------------------)
//
//	MetaType 
//
//	Поддерживается три типа типов (метатипов) - примитив, структура, массив.
//
// - Примитив. 
//     Метатип не нуждается в метаданных, но нуждается в сериализаторе.
//     Разбор дерева рано или поздно приводит к данным этого типа. 
//     Поэтому здесь необходимо иметь простейшее преобразование в(из) строку(ки). 
//        Primitive<PrimType>.
//
// - Массив. 
//     Метатип тоже не нуждается в метаданных - его структура заранее известна, 
//     это повторяющиеся поля (элементы) одного типа.
//     Надо лишь описать тип элементов - нужен тип и его метатип.
//        VectorArrayItem<ItemType>
//
// - Структура. 
//     Метатип нуждается в метаданных (разметке полей).
//        Layout<StructType>
//
//----------------------------------------------------------)
namespace MetaClasses 
{

template <typename PrimType>
struct PrimClassMeta
	: public MetaClass<PrimType>
{
	PrimType ReadNode(const INode& node) const
	{
		return Primitive<PrimType>::Parse(node.ReadText());
	}

	void WriteNode(INode* pNode, const PrimType& prim) const
	{
		pNode->WriteText(Primitive<PrimType>::ToString(prim));
	}
};
//----------------------------------------------------------)

template <typename StructType>
struct StructClassMeta
	: public MetaClass<StructType>
{
	StructClassMeta(const Layout<StructType>& layout = DefaultLayout<StructType>())
		: m_layout(layout)
	{}

	StructType ReadNode(const INode& node) const 
	{
		StructType data;
		for_each(m_layout.begin(), m_layout.end()
			, ReadField(node, &data));
		return data;
	}

	void WriteNode(INode* pNode, const StructType& data) const 
	{
		for_each(m_layout.begin(), m_layout.end()
			, WriteField(pNode, data));
	}

protected:
	struct ReadField
	{
		ReadField(const INode& n, StructType* pD) : node(n), pData(pD) {}
		void operator() (const CPtrShared<FieldAttrubute<StructType> >& pField) 
			{ pField->ReadField(node, pData);}
		const INode& node;
		StructType*  pData;	
	}; 
	struct WriteField
	{
		WriteField(INode* pN, const StructType& d) : pNode(pN), data(d) {}
		void operator() (const CPtrShared<FieldAttrubute<StructType> >& pField) 
			{ pField->WriteField(pNode, data);}
		INode*            pNode;
		const StructType& data;	
	}; 
	const Layout<StructType>& m_layout;
};
//----------------------------------------------------------)

template <typename ItemType>
struct VectorClassMeta
	: public MetaClass<std::vector<ItemType> >
{
	VectorClassMeta( ArrayItemName              itemName 
	              , const MetaClass<ItemType>&  itemClassMeta = DefaultMetaClass<ItemType>()
				  )
		: m_itemName(itemName)
		, m_itemClassMeta(itemClassMeta)
	{}

	std::vector<ItemType> ReadNode(const INode& node) const 
	{
		std::vector<ItemType> vec;
		std::auto_ptr<INodeArray> pChildNodes(node.ChildElements(m_itemName));

		long count = pChildNodes->Count();
		vec.reserve(count);
		for (long idx = 0; idx != count; idx++)
		{
			INode* pNode = pChildNodes->GetByIndex(idx);
			vec.push_back(m_itemClassMeta.ReadNode(*pNode));
		}
		return vec;
	}

	void WriteNode(INode* pNode, const std::vector<ItemType>& d) const 
	{
		index count = d.size();
		for (index idx = 0; idx != count; idx++)
		{
			std::auto_ptr<INode> pNewNode(pNode->AddElement(m_itemName, (long)idx));
			m_itemClassMeta.WriteNode(pNewNode.get(), d[idx]);
		}
	}

private:
	ArrayItemName              m_itemName; 
	const MetaClass<ItemType>& m_itemClassMeta;
};

} // namespace StructLayout {
//----------------------------------------------------------)
//----------------------------------------------------------)

namespace Primitives
{
	template <typename PrimType>
	struct Primitive
	{
		static tstring ToString(const PrimType&)
		{ 
			throw ParseValueException("No primitive specialization");
		}
		static PrimType Parse(const tstring& s) 
		{ 
			throw ParseValueException("No primitive specialization");
		}
	};

	// неправильный формат значения
	struct ParseValueException : public Exception
	{
		ParseValueException(const tstring& a_msg) 
			: Exception(a_msg) 
		{}
	};

} //namespace Primitives
//----------------------------------------------------------)
//----------------------------------------------------------)

template <typename Data>
void ReadPrim(const INode& node, Data* pN)
{
	PrimClassMeta<Data>().ReadNode(node, pN);
}

template <typename Struct>
void Read(const INode& node, Struct* pStruct, const Layout<Struct> layout = DefaultLayout<Struct>())
{
	StructClassMeta<Struct>(layout).ReadNode(node, pStruct);
}

template <typename Data>
void Read(const INode& node, std::vector<Data>* pVector, ArrayItemName itemName, const MetaClass<Data>& metaData = DefaultMetaClass<Data>())
{
	VectorClassMeta<Data>(itemName, metaData).ReadNode(node, pVector);
}
//----------------------------------------------------------)
//----------------------------------------------------------)

template <typename PrimType>
void WritePrim(INode* pNode, const PrimType& d)
{
	PrimClassMeta<PrimType>().WriteNode(pNode, d);
}

template <typename Struct>
void Write(INode* pNode, const Struct& d, const Layout<Struct> layout = DefaultLayout<Struct>())
{
	StructClassMeta<Struct>(layout).WriteNode(pNode, d);
}

template <typename ArrayItemType>
void Write( const INode&                   node
		  , std::vector<ArrayItemType>*    pVector
		  , ArrayItemName                  itemName
		  , const MetaClass<ArrayItemType>& metaData = DefaultMetaClass<ArrayItemType>()
		  )
{
	VectorClassMeta<ArrayItemType>(itemName, metaData).WriteNode(node, pVector);
}

}; // namespace Serializer {
}; // namespace Rsdn       {

#endif  // __MFields_h__
//==========================================================)
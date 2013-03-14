#include "stdafx.h"

#include <fstream>

#include "classFactory.h"
/*
FactoryObject::FactoryObject(IObjectCreator *def)
	:definition(def)
{
	definition->notifyCreate(this);
}

FactoryObject::~FactoryObject()
{
	definition->notifyRelease(this);
}

//get object's definition

ObjectDef * FactoryObject::getDefinition() const 
{
	return definition;
}
//create a clone of this type
FactoryObject *FactoryObject::clone() const
{
	if(definition)
		return definition->create();
	else 
		return NULL;
}*/
ObjectCreator::ObjectCreator(IFactoryManager *fact,std::string type)
		:factory(fact),typeName(type)
{	
	objectsCreated=0;
	objectsDeleted=0;
}
ObjectCreator::~ObjectCreator()
{
//	assert(objects.size()==0);
	assert(objectsCreated==objectsDeleted);
}

void ObjectCreator::notifyRelease(FactoryObject *object)
{
	//std::set<FactoryObject*>::iterator it=objects.find(object);
	//assert(it!=objects.end());

	//objects.erase(object);
	objectsDeleted++;
}
void ObjectCreator::notifyCreate(FactoryObject *object)
{
	//objects.insert(object);
	objectsCreated++;
}


DefinitionXML::DefinitionXML(IFactory *d)
{
	def=d;//->createDefinition();
}
DefinitionXML::~DefinitionXML()
{
	//if(def)
	//	delete def;
}
void DefinitionXML::addDouble(const std::string &name,int offset)
{
	if(values.find(name)==values.end())
	values[name]=ValueXML(valueTypeFloat,offset);
}
void DefinitionXML::addInt(const std::string &name, int offset)
{
	if(values.find(name)==values.end())
	values[name]=ValueXML(valueTypeInt,offset);
}
void DefinitionXML::addFloat(const std::string &name, int offset)
{
	if(values.find(name)==values.end())
	values[name]=ValueXML(valueTypeFloat,offset);
}
void DefinitionXML::addString(const std::string &name,int offset)
{
	if(values.find(name)==values.end())
	values[name]=ValueXML(valueTypeString,offset);
}
//void DefinitionXML::a
void DefinitionXML::addObject(const std::string &name,const std::string &type, int offset)
{
	if(values.find(name)==values.end())
	{
	values[name]=ValueXML(valueTypeObject,offset);
	values[name].objType=type;
	}
}
void DefinitionXML::parseParameters (ClassFactory *owner,IFactory *target)
{
	std::map<std::string,ValueXML>::iterator it;
	std::string *pStr;
	std::string tmp;
	for(it=values.begin();it!=values.end();it++)
	{
		char *data=((char*)target)+it->second.offset;
		IFactory *d;
		switch(it->second.type)
		{
		case valueTypeInt:
			*((int*)data)=owner->getPropertyInt(it->first.c_str());
			break;
		case valueTypeFloat:

			*((float*)data)=owner->getPropertyFloat(it->first.c_str());
			break;
		case valueTypeString:
			pStr=((std::string*)data);
			*pStr=owner->getPropertyString(it->first.c_str());
			//*((std::string*)data)=owner->getPropertyString(it->first.c_str());
			break;
		case valueTypeObject:			
			d=owner->getPropertyObject(it->first.c_str(),it->second.objType);
			if(d)
				memcpy(data,&d,sizeof(void *));
			break;
		}
	}
}

ClassFactory::ClassFactory()
{
	doc=new TiXmlDocument;
	currentElement=NULL;
}

ClassFactory::~ClassFactory()
{
	delete doc;
	std::map<std::string,DefinitionXML*>::iterator it1;
	std::map<std::string,IFactory*>::iterator it2;
	
	for(it2=types.begin();it2!=types.end();it2++)
		delete it2->second;
	for(it1=templateLib.begin();it1!=templateLib.end();it1++)
		delete it1->second;
}



TiXmlElement* ClassFactory::findProperty(const char *name)
{
	TiXmlElement *pElem;
	TiXmlHandle handle(currentElement);
	if(!currentElement)return NULL;
	pElem=handle.FirstChildElement().Element();
	for( pElem; pElem; pElem=pElem->NextSiblingElement())	
		if(!strcmp(pElem->Value(),name))					
				return pElem;		
	return NULL;
}

void ClassFactory::registerTemplate(const std::string &name, IFactory *definition)
{
	DefinitionXML *defXML=new DefinitionXML(definition);
	//defXML->def=definition;
	
	templateLib[name]=defXML;	
}

IFactory* ClassFactory::registerType(const std::string &className, const char *definitionName)
{
	///ObjectDef *creator=creators[definitionName];
	DefinitionXML *defXML=templateLib[definitionName];
	IFactory *target;
	if(defXML)
	{
		target=defXML->def;//->createDefinition();
		//creator->createDefinition();		
		//definitions[className]->parseParameters(this);
		target->setClass(className);
		target->setType(defXML->def->getType());
		currentName=className;
		types[className]=target;
		defXML->parseParameters(this,target);
		return target;
	}
	return NULL;
}

IFactory * ClassFactory::getPropertyObject(const char *name, const std::string &type)
{
	static char res[256]="";
	res[0]=0;
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)
		{
			//TiXmlAttribute *attrib=prop->attributeSet.Find("value");			
			if(prop->Attribute("value"))	
			{
				strcpy_s(res,prop->Attribute("value"));		
				if(types[res]->getType()==type)
					return types[res];
			}
			else
			{
				//if no definition
				std::map<std::string,DefinitionXML*>::const_iterator it;
				if((it=templateLib.find(type))==templateLib.end())
					return NULL;
				TiXmlElement *tmpElement=currentElement;
				std::string tmpName=currentName;
				std::string newName=currentName+"."+(std::string)name;
				const char *newType=it->first.c_str();

				currentElement=prop;
				
				IFactory *res=registerType(newName,newType);
				currentName=tmpName;
				currentElement=tmpElement;
				return res;				
			}
			return NULL;
		}
	}
	return NULL;
}
void ClassFactory::propertyFloat(const std::string &object,const std::string &name,int offset)
{
	DefinitionXML *defXML=templateLib[object];	
	if(defXML)	
		defXML->addFloat(name,offset);
}
void ClassFactory::propertyInt(const std::string &object,const std::string &name,int offset)
{
	DefinitionXML *defXML=templateLib[object];	
	if(defXML)	
		defXML->addInt(name,offset);
}
void ClassFactory::propertyString(const std::string &object,const std::string &name,int offset)
{
	DefinitionXML *defXML=templateLib[object];	
	if(defXML)	
		defXML->addString(name,offset);
}
void ClassFactory::propertyObject(const std::string &object,const std::string &name,const std::string &type,int offset)
{
	DefinitionXML *defXML=templateLib[object];	
	if(defXML)	
		defXML->addObject(name,type,offset);
}
void ClassFactory::loadDefinitions(const char *filename)
{	
	doc->LoadFile(filename,TIXML_ENCODING_UTF8);	
	TiXmlHandle hDoc(doc);
	TiXmlHandle hRoot(0);
	TiXmlElement *pElem=hDoc.FirstChild("objects2").Element();
	hRoot=TiXmlHandle(pElem);
	char pCreator[100];
	char pName[100];
	const char * tmpPtr;

	pElem=hRoot.FirstChildElement().Element();
	for( pElem; pElem; pElem=pElem->NextSiblingElement())
	{		
		strcpy_s(pCreator,pElem->Value());
		if(strlen(pCreator))
		{
			if(tmpPtr=pElem->Attribute("name"))
			{
				strcpy_s(pName,tmpPtr);	
				currentElement=pElem;
				registerType(pName,pCreator);
			}
		}
	}
}
void ClassFactory::saveDefinitions(const char *filename)
{
}

//ask for a float parameter
float ClassFactory::getPropertyFloat(const char *name)
{
	float res=0;
	TiXmlElement *prop;
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)prop->QueryFloatAttribute("value",&res);
	}
	return res;
}
//ask for a float2 vector parameter
void ClassFactory::getPropertyFloat2(const char *name,float *p)
{
	static char res[256]="";
	res[0]=0;
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)
		{		
			strcpy_s(res,prop->Attribute("value"));		
			sscanf_s(res,"%f %f",p[0],p[1]);
		}
	}
}
//ask for a float3 vector parameter
void ClassFactory::getPropertyFloat3(const char *name,float *p)
{
	static char res[256]="";
	res[0]=0;
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)
		{		
			strcpy_s(res,prop->Attribute("value"));		
			sscanf_s(res,"%f %f %f",p,p+1,p+2);
		}
	}
}
//ask for an int parameter
int ClassFactory::getPropertyInt(const char *name)
{
	static std::map<std::string,int>::iterator it;
	int res=0;
	static char tmpStr[80];
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(!prop)return res;

		strcpy_s(tmpStr,prop->Attribute("value"));
		it=enumMap.find(tmpStr);
		if(it!=enumMap.end())//first search in enum table			
			return (it->second);
		else
			prop->QueryIntAttribute("value",&res);
	}
	return res;
}
//ask string parameter
std::string ClassFactory::getPropertyString(const char *name)
{	
	static char res[256]="";
	res[0]=0;
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)strcpy_s(res,prop->Attribute("value"));		
	}
	return std::string(res);
}

int ClassFactory::getPropertyEnum(const char *name)
{
	static char res[256]="";
	res[0]=0;
	TiXmlElement *prop;	
	if(currentElement)
	{		
		prop=findProperty(name);
		if(prop)
		{
			strcpy_s(res,prop->Attribute("value"));
			std::map<std::string,int>::iterator it=enumMap.find(res);
			if(it!=enumMap.end())//no enum element				
				return (it->second);
		}
	}
	return 0;
}
void ClassFactory::registerEnumConstant(char *name, int value)
{
	enumMap[std::string(name)]=value;
}

FactoryObject *ClassFactory::createObject(const std::string &name)
{
	IFactory * def=types[name];
	return (def)?def->create():NULL;
}

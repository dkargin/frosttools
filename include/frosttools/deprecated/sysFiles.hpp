#ifndef FROSTTOOLS_SYSFILES
#define FROSTTOOLS_SYSFILES
#pragma once
#include <list>

namespace frostTools
{
	class Path : public std::list<std::string>
	{
	public:
		Path(const std::string & path)
		{
			assign(path.c_str(),path.size());
		}
		Path(const char * path)
		{
			assign(path,strlen(path));
		}
		Path(){}
		std::string toString()const
		{
			std::string result;
			if(!empty())
				result = front();
			for(auto it = ++begin(); it != end(); it++)
			{
				result = result + "/" + *it;
			}
			return result;
		}
		std::string toRelative(const char * strAbsolute) const
		{
			Path absolute(strAbsolute);		
			Path result;
			auto base_it = begin();
			auto ab_it = absolute.begin();
			auto absolute_end = absolute.end();
			auto base_end = end();

			// 1. skip equal part
			while(true)
			{
				if( ab_it == absolute_end )
					break;
				if( base_it == base_end )
				{
					result.push_back(".");
					break;
				}
				if(*base_it != *ab_it)
					break;
				base_it++;
				ab_it++;
			}
			// 2. go up to equal part
			for(;base_it != base_end; ++base_it)
				result.push_back("..");
			// 3. go down
			for(;ab_it != absolute_end; ++ab_it)
				result.push_back(*ab_it);
			return result.toString();
		}
	protected:
		void assign(const char * path, size_t len)
		{
			char delim[] = "\\/";		
			const char * start = path;
			const char * end = NULL;
			while(end = strpbrk(start,delim))
			{
				push_back(std::string(start,end));
				start = end+1;
			}
			if(start - path)
			{
				push_back(std::string(start,path + len));
			}
		}
	};
}

inline std::string toRelative(const char * path, const char * base)
{
	return frostTools::Path(base).toRelative(path);
}

#include <direct.h>
inline std::string toAppRelative(const char * path)
{
	char base[MAX_PATH];
	getcwd(base,MAX_PATH);
	return toRelative(path,base);
}

///////////////////////////////////////////////////////
// Get file extension
///////////////////////////////////////////////////////
template<class String>
String getFileExt(const String &str)
{
	bool flag=false;
	String::const_iterator it=str.end();
	String::const_iterator begin=str.begin();
	for(it--;it!=begin;it--)
	{
		if(*it=='.')
		{
			flag=true;
			break;
		}
	}
	if(flag)
		return string(++it,str.end());
	else
		return string("");
}
////////////////////////////////////////////////////////
// File name without extension & directory
////////////////////////////////////////////////////////
template<class String>
String getFileName(const String &str)
{
	bool flag=false;
	String::const_iterator it=--str.end();
	String::const_iterator begin=str.begin();
	for(;it!=begin;it--)
	{
		if(*it==L'.')
			break;
	}
	String::const_iterator i2=it;
	for(;it!=begin;it--)
	{
		if(*it=='\\' || *it=='/')
		{
			flag=true;
			break;
		}
	}	
	if(flag)
		return String(++it,i2);
	else
		return String("");
}
///////////////////////////////////////////////////
// Filename without directory
///////////////////////////////////////////////////
template<class String>
String getFileName2(const String &str)
{
	bool flag=false;
	String::const_iterator it=--str.end();
	String::const_iterator begin=str.begin();	
	String::const_iterator i2=it;
	for(;it!=begin;it--)
	{
		if(*it=='\\' || *it=='/')
		{
			flag=true;
			break;
		}
	}	
	if(flag)
		return String(++it,++i2);
	else
		return String("");
}
//////////////////////////////////////////////////////
// Get file path from full name
//////////////////////////////////////////////////////
template<class String>
String getFilePath(const String &str)
{
	String::const_iterator it=--str.end();
	String::const_iterator begin=str.begin();
	for(;it!=begin;it--)
	{
		if(*it=='\\' || *it=='/')
			break;
	}
	return String(str.begin(),it);
}
////////////////////////////////////////////////////////////
// File name List
////////////////////////////////////////////////////////////
#include <windows.h>
//template<class Char>
class FileList: public std::list<std::basic_string<TCHAR>>
{
public:
	typedef TCHAR Char;
	typedef /*typename */std::basic_string<Char> String;	
	//typedef typename std::list<String>::iterator iterator;
	//typedef typename std::list<String>::size_type size_type;
private:
	typedef std::basic_ifstream<Char,std::char_traits<Char>> _istream;
	typedef std::basic_ofstream<Char,std::char_traits<Char>> _ostream;
	//std::list<String> files;	
	String parentDir;
public:
	
	FileList()
	{}
	FileList(const String &directory)
	{
		readDirectory(directory,TEXT(""));
	}
	void add(const String &path)
	{
		push_back(path);
	}
	void readList(const String &lst)
	{
		String name;
		clear();
		_istream stats(lst.c_str());		
		while(!stats.eof())	
		{			
			stats>>name;
				push_back(name);
		}
	}
	void writeList(const String &lst)
	{		
		_ostream stats(lst.c_str());		
		iterator it=begin();
		iterator e=end();
		for(;it!=e;it++)			
			stats<<*it<<std::endl;					
	}
	void readDirectory(const String &dir,const String &ext)
	{
		//clear();
		WIN32_FIND_DATA findFileData;
	
		int size=dir.size();
		String d=dir;
		
		if(size && (dir[size-1]!=TEXT('\\') || dir[size-1]!=TEXT('/')))
			d=d+TEXT('/');
		String path=d;
		path+=ext;
		
		HANDLE hFile=FindFirstFile(path.c_str(),&findFileData);

		if(hFile!=INVALID_HANDLE_VALUE)
		{
			do
			{
				push_back(d+String(findFileData.cFileName));
			
				
			}while(FindNextFile(hFile,&findFileData));	
		}
	}
};
#endif
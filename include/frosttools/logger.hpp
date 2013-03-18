#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <stdarg.h>
#include <stdio.h>
const bool brackets = true;
class Log
{
public:
	class Scoped;
	enum {maxThreat=-1};
	Log()
	{
	    level = 0;
	    threatLevel = 0;
	    last = NULL;
	    fill = ' ';
	}

	class Scoped
	{
		friend class Log;
		Scoped *prev;
		Log &root;
		const char *text;
		bool entry;
		int threat;
		void writeEntry()
		{
			if(!entry)
			{
				entry=true;
				if(prev)prev->writeEntry();
				if(text)root(maxThreat,text);
				if(brackets)root(maxThreat,"{");
				root.levelInc();
			}
		}
	public:
		Scoped(Log &l,const char *_text,int threatLevel) :root(l)
		{
		    threat = threatLevel;
		    text = _text;
		    entry = false;
		    prev = NULL;
			root.push(this);
		}
		~Scoped()
		{
			root.pop(this);
			if(entry)
			{
				root.levelDec();
				if(brackets)root(maxThreat,"}");
			}
		}
	};
	// игнорировать ли сообщение
	inline bool ignore(int threat)
	{
		if(threat<0)return false;
		if(last)threat+=last->threat;
		return threat>=0 && threat<threatLevel;
	}
	virtual void line(int threat,const char *format,...)
	{
		va_list	ap;
		va_start(ap,format);
		const int MAX_LEN=4096;
		static char buffer[MAX_LEN];
		if(format && !ignore(threat))
		{
			if(last && !last->entry)
				last->writeEntry();
			for(int i=0;i<level;i++)buffer[i]=fill;
#ifdef _MSC_VER
			vsprintf_s(buffer + level, MAX_LEN - level,format,ap);
#else
			vsnprintf(buffer + level, MAX_LEN - level,format,ap);
#endif

			buffer[MAX_LEN-1]=0;
			sendLine(buffer);
		}
		va_end(ap);
	}
	inline int levelInc()
	{
		return ++level;
	}
	inline int levelDec()
	{
		return level>0?--level:level;
	}
	void operator()(int threat,const char *format,...)
	{
		va_list	ap;
		va_start(ap,format);
		line(threat,format,ap);
		va_end(ap);
	}
	void setThreat(int threat)
	{
		threatLevel=threat;
	}
protected:

	void push(Scoped *scoped)
	{
		scoped->prev=last;
		last=scoped;
	}
	void pop(Scoped *scoped)
	{
		last=scoped->prev;
	}
	virtual void sendLine(const char *text)=0;
	char fill;
	int level;
	Scoped * last;
	int threatLevel;
};

class LogFILE: public Log
{
public:
	FILE *log;
	bool local;
	LogFILE(const char *path)
	{
		log=fopen(path,"w");
		local=log!=NULL?true:false;
	}
	LogFILE(FILE *target=stderr)
		:log(target),local(false)
	{}
	~LogFILE()
	{
		if(local)
		{
			fclose(log);
			log=NULL;
		}
	}
	virtual void sendLine(const char *text)
	{
		fprintf(log,"%s\n",text);
	}
};

#if defined(_RPT1)
class LogTrace: public Log
{
public:
	LogTrace(){}
	virtual void sendLine(const char *text)
	{
		if(text)
		{
			_RPT1(0,"%s\n",text);
		}
	}
};
#endif
typedef Log * LogPtr;

#ifdef __FUNCTION__
#define __FUNCTION_NAME__ __FUNCTION__
#else
#define __FUNCTION_NAME__ ("<unknown>")
#endif

#define LogScope(log,level) Log::Scoped __logFunction((log),__FUNCTION_NAME__,(level))
#define LogFunction(log) LogScope(log,0)
#endif

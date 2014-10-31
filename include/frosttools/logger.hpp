#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <stdarg.h>
#include <stdio.h>

namespace frosttools
{
/// If we use brackets
const bool brackets = true;

/// Logger class
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

	virtual ~Log() {}

	/// Scope for logger
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
		/// Constructor
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
	/// Check if thread should be ignored
	inline bool ignore(int threat)
	{
		if(threat<0)return false;
		if(last)threat+=last->threat;
		return threat>=0 && threat<threatLevel;
	}
	/// Write line to log
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
	/// increase log level
	inline int levelInc()
	{
		return ++level;
	}
	/// decrease log level
	inline int levelDec()
	{
		return level>0?--level:level;
	}
	/// Write line to log
	void operator()(int threat,const char *format,...)
	{
		va_list	ap;
		va_start(ap,format);
		line(threat,format,ap);
		va_end(ap);
	}
	/// Set threat level
	void setThreat(int threat)
	{
		threatLevel=threat;
	}
protected:

	/// Push a new scope
	void push(Scoped *scoped)
	{
		scoped->prev=last;
		last=scoped;
	}
	/// Pop scope
	void pop(Scoped *scoped)
	{
		last=scoped->prev;
	}
	/// write line
	virtual void sendLine(const char *text)=0;

	char fill;			///< Filler character
	int level;			///< Character level
	Scoped * last;		///< Last scope pointer
	int threatLevel;	///< Current threat level
};

/// File-based logger
class LogFILE: public Log
{
public:
	/// File used to put logs in
	FILE *log;
	/// Local?
	bool local;
	/// Constructor
	LogFILE(const char *path)
	{
		log=fopen(path,"w");
		local=log!=NULL?true:false;
	}
	/// Constructor
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
	/// Writes a line
	virtual void sendLine(const char *text)
	{
		fprintf(log,"%s\n",text);
	}
};

/// This log implementation does not write any data
class LogNull : public Log
{
public:
	virtual void sendLine(const char *text)
	{
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

} // namespace frosttools

#ifdef __FUNCTION__
#define __FUNCTION_NAME__ __FUNCTION__
#else
#define __FUNCTION_NAME__ ("<unknown>")
#endif

#define LogScope(log,level) frosttools::Log::Scoped __logFunction((log),__FUNCTION_NAME__,(level))
#define LogFunction(log) LogScope(log,0)
#endif


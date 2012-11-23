#ifndef FROSTTOOLS_SCRIPTER
#define FROSTTOOLS_SCRIPTER
#pragma once
///////////////////////////////////////////////////////////////
// Simple script handler
///////////////////////////////////////////////////////////////
extern "C"
{
	#include "lua.h"
	#include <lauxlib.h>
	#include <lualib.h>
	#include <tolua++.h>
}

class _Scripter
{		
public:	
	class NullType{};
	class UserBind
	{	
		friend class _Scripter;
		std::string type;
		void *ptr;
	public:
		UserBind(const std::string &t,void *obj):type(t),ptr(obj)		
		{}		
		UserBind(const UserBind &bind):type(bind.type),ptr(bind.ptr)
		{}
	};

	class Object
	{
		friend class _Scriper;
		_Scripter *owner;
		int index;
		bool pop;
	public:	
		Object(_Scripter *l):owner(l),pop(false)
		{
			index=lua_gettop(owner->luaVM);
		}
		Object(const Object &obj)
			:index(obj.index),pop(true),owner(obj.owner)
		{}
		~Object()
		{
			release();
		}
		void release()
		{
			if(pop)
				lua_pop(owner->luaVM,1);
			pop=false;
		}		
		bool call(const std::string &fn)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);		
			int res=0;									

			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			return true;
		}
		template<typename T1>
		bool call(const std::string &fn,T1 arg1)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);	
			int res=owner->push(arg1);							
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2>
		bool call(const std::string &fn,T1 arg1,T2 arg2)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);
			int res=owner->push(arg1)+
					owner->push(arg2);							
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2,typename T3>
		bool call(const std::string &fn,T1 arg1,T2 arg2,T3 arg3)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);
			int res=owner->push(arg1)+
					owner->push(arg2)+
					owner->push(arg3);		
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}	
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2,typename T3,typename T4>
		bool call(const std::string &fn,T1 arg1,T2 arg2,T3 arg3,T4 arg4)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);
			int res=owner->push(arg1)+
					owner->push(arg2)+
					owner->push(arg3)+
					owner->push(arg4);		
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2,typename T3,typename T4,typename T5>
		bool call(const std::string &fn,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);	
			int res=owner->push(arg1)+
					owner->push(arg2)+
					owner->push(arg3)+
					owner->push(arg4)+
					owner->push(arg5);
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
		bool call(const std::string &fn,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);	
			int res=owner->push(arg1)+
					owner->push(arg2)+
					owner->push(arg3)+
					owner->push(arg4)+
					owner->push(arg5)+
					owner->push(arg6);
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
		bool call(const std::string &fn,T1 arg1,T2 arg2,T3 arg3,T4 arg4,T5 arg5,T6 arg6,T7 arg7)
		{
			lua_getfield(owner->luaVM,-1,fn.c_str());
			if(lua_type(owner->luaVM,-1)!=LUA_TFUNCTION)
				return false;
			owner->pushValue(-2);	
			int res=owner->push(arg1)+
					owner->push(arg2)+
					owner->push(arg3)+
					owner->push(arg4)+
					owner->push(arg5)+
					owner->push(arg6)+
					owner->push(arg7);
			if(lua_pcall(owner->luaVM, res+1, 1, 0) != 0)
			{
				owner->_process_error();
				return false;
			}
			lua_pop(owner->luaVM,1);
			return true;
		}
		inline bool valid()
		{			
			//lua_settop(luaVM,index);
			bool res=(lua_type(owner->luaVM,-1)!=LUA_TNIL);
			//lua_pop(luaVM,1);
			return res;
		}
	};

	_Scripter(const std::string &_file)
		:luaVM(0),state(stateOffline),errFunc(0),externVM(false)
	{
		assign(_file.c_str());
	}
	_Scripter(lua_State *state)
		:luaVM(0),state(stateOffline),errFunc(0),externVM(false)
	{
		assign(state);
	}
	virtual ~_Scripter()
	{
		if(!externVM && luaVM)lua_close( luaVM );
	}
	bool assign(const char *fileName)
	{
		file=fileName;
		luaVM=lua_open();
		if(!luaVM)return false;
		prepareWrapper();	
		return true;
	}
	bool assign(lua_State *state)
	{
		if(!state)return false;
		luaVM=state;
		externVM=true;
		prepareWrapper();
		return true;
	}
	void prepareWrapper()
	{
		state=stateReady;
		errFunc=lua_sethook(luaVM,lineHook,LUA_MASKLINE,0);
		luaL_openlibs(luaVM);
	}	
	int runScript()
	{
		if(luaVM)
		{			
			if(luaL_loadfile(luaVM,file.c_str())==LUA_ERRFILE)
			{
				error=std::string("cannot load file <")+file+std::string(">");
				return 0;
			}		
			int res=lua_pcall(luaVM, 0, LUA_MULTRET, 0);
			error="";
			
			if(res)	// some error happened
			{
				_process_error();		
				return 0;
			}
		}
		return 1;
	}
	Object selectObject(const std::string &object)
	{
		lua_getfield(luaVM,LUA_GLOBALSINDEX,object.c_str());
		return Object(this);//lua_type(luaVM,-1)!=LUA_TNIL;
		//lua_getfield(L,-1,'foo');
		//pushValue(-2);
		//lua_pcall(luaVM,2,0,0);
		//popValue(1); // забрали нафиг объект
	}
	int selectField(const std::string &field)
	{
		lua_getfield(luaVM,-1,field.c_str());
		return lua_type(luaVM,-1)!=LUA_TNIL;
	}

	template<class Type>void addGlobal(const Type &type,const char *name);
	void pushValue(int i);
	void popValue(int i);
	int push(const UserBind &value);
	int push(const int &value);
	int push(const float &value);
	int push(const double &value);
	int push(const std::string &value);
	int push(const NullType &value);

	const char * getError()
	{
		return error.c_str();
	}
	void get(int &v,const char *name,int def=0)
	{
		lua_getglobal(luaVM,name);
		if(lua_type(luaVM,-1)==LUA_TNIL)
		{
			v=def; 
			return;
		}
		int r=lua_tointeger(luaVM,-1);
		lua_pop(luaVM,1); 
		v=r;
	}
	void get(float &v,const char *name,float def=0)
	{
		lua_getglobal(luaVM,name);
		if(lua_type(luaVM,-1)==LUA_TNIL)
		{
			v=def; 
			return;
		}
		float r=(float) lua_tonumber(luaVM,-1);
		lua_pop(luaVM,1); 
		v=r;
	}
	lua_State* getVM(){return luaVM;}
	int doCall(int arg)
	{
		if(lua_pcall(luaVM, arg, 1, 0) != 0)
		{
			std::string error(lua_tostring(luaVM, -1));
			//throw std::string(std::string(lua_tostring(luaVM, -1)));
			return 0;
		}
		return 1;
	}

protected:
	static void lineHook(lua_State *L, lua_Debug *ar)
	{
	}
	template<typename T1=NullArg,typename T2=NullArg,typename T3=NullArg,typename T4=NullArg,typename T5=NullArg>
	class FunctionProxy
	{
		_Scripter *owner;
	public:
		FunctionProxy(_Scripter * s):owner(s){}
		int operator()(const std::string &fn,	const T1 &arg1,
												const T2 &arg2,
												const T3 &arg3,
												const T4 &arg4,
												const T5 &arg5)
		{
			int res=0;
			res+=owner->push(arg1);
			res+=owner->push(arg2);
			res+=owner->push(arg3);
			res+=owner->push(arg4);
			res+=owner->push(arg5);

			if(lua_pcall(owner->luaVM, res, 1, 0) != 0)
			{
				std::string error(lua_tostring(owner->luaVM, -1));
				//throw std::string(std::string(lua_tostring(luaVM, -1)));
			}
			return 0;
		}
	};
	void _process_error()
	{
		error=lua_tostring(luaVM, -1);	
		_RPT1(0,"LUA error:%s\n",error.c_str());
		lua_pop(luaVM,1);
		//throw std::string(std::string(lua_tostring(luaVM, -1)));
	}	
	template<typename T1,typename T2,typename T3,typename T4,typename T5>
	void _call_fn(const std::string &fn,const T1 &arg1=NullArg(),const T2 &arg2=NullArg(),const T3 &arg3=NullArg(),const T4 &arg4=NullArg(),const T5 &arg5=NullArg())
	{				
		//lua_getglobal(luaVM, fn.c_str());

		int res=0;
		res+=push(arg1);
		res+=push(arg2);
		res+=push(arg3);
		res+=push(arg4);
		res+=push(arg5);

		if(lua_pcall(luaVM, res, 1, 0) != 0)
		{
			std::string error(lua_tostring(luaVM, -1));
			//throw std::string(std::string(lua_tostring(luaVM, -1)));
		}
	}
	std::string file;
	std::string error;
	lua_State* luaVM;
	int errFunc;
	bool externVM;

	enum ScriptState
	{
		stateOffline,
		stateReady,
		stateErrorLocal,	
	}state;
};
//////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////
template<class Type> inline void _Scripter::addGlobal(const Type &type,const char *name)
{
	push(type);
	lua_setglobal(luaVM, name);
}
inline void _Scripter::pushValue(int i)
{
	lua_pushvalue(luaVM,i);
}
inline void _Scripter::popValue(int i)
{
	lua_pop(luaVM,1);
}
inline int _Scripter::push(const UserBind &value)
{
	tolua_pushusertype(luaVM,value.ptr,value.type.c_str());
	return 1;
}
inline int _Scripter::push(const int &value)
{
	lua_pushinteger(luaVM, value);
	return 1;
}
inline int _Scripter::push(const float &value)
{
	lua_pushnumber(luaVM, value);
	return 1;
}
inline int _Scripter::push(const double &value)
{
	lua_pushnumber(luaVM, value);
	return 1;
}
inline int _Scripter::push(const std::string &value)
{
	lua_pushstring(luaVM, value.c_str());
	return 1;
}
inline int _Scripter::push(const NullType &value)
{
	//lua_pushstring(L, value.c_str());
	return 0;
}
#endif
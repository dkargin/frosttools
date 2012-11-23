#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP
template<class Type,int Max>
struct Ring
{
	typedef Type value_type;
	typedef unsigned int size_type;
	static const size_type max=Max;
	value_type values[max];
	size_type count;
	size_type start;
	Ring():count(0),start(0){}
	// записываем в конец новое значение
	void push_back(const value_type &val)
	{
		if(count==max)
		{
			values[start]=val;
			start=index(1);
		}
		else
		{
			values[index(count)]=val;
			count++;
		}
	}
	void push_front(const value_type &val)
	{
		start=index(-1);
		values[index(0)]=val;
		if(count<max)		
			count++;		
	}
	value_type pop_back()
	{
		value_type res;
		if(count)
		{			
			count--;
			res=values[index(count)];
		}
		return res;
	}
	value_type pop_front()
	{
		value_type res;
		if(count)
		{
			start=index(1);
			count--;
			res=values[index(-1)];
		}
		return res;
	}
	inline bool empty() const
	{
		return count==0;
	}
	inline size_type size() const
	{
		return count;
	}
	const value_type & operator[](const size_type &i) const
	{
		return values[index(i)];
	}
	value_type & operator[](const size_type &i)
	{
		return values[index(i)];
	}
	size_type index(const size_type &i) const
	{		
		return (start+i<0?labs(i+max):i)%max;
	}
};
#endif
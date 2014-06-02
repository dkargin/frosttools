/////////////////////////////////////////////////////////////////////////////////
// ������ � �������� ������������. 
// ����������� � ����������� ������ ����������� (��������). 
/////////////////////////////////////////////////////////////////////////////////
#ifndef _MATH_SET
#define _MATH_SET
#pragma once
using namespace std;

namespace frosttools
{

// ������������ Target ����������� == � != ��� ������� � ��� ����������� ������� equal
template<class Target> struct EqualTraits
{
	friend inline bool operator==(const Target &a,const Target &b)
	{
		return Target::equal(a,b);
	}
	friend inline bool operator!=(const Target &a,const Target &b)
	{
		return !Target::equal(a,b);
	}
};
// ���������� ��������������� ���������� ������ ������ ����������.
// ��������� ������� += -= *= /=
template<class Target,class Scalar> struct ArythmeticTraits
{
	friend inline Target operator+(const Target &a,const Target &b)
	{
		Target res=a;
		return a+=b;
	}
	friend inline Target operator-(const Target &a,const Target &b)
	{
		Target res=a;
		return a-=b;
	}
	friend inline Target operator*(const Target &a,const Scalar &scakar)
	{
		Target res=a;
		return res*=scalar;
	}
	friend inline Target operator*(const Scalar &scalar,const Target &a)
	{
		Target res=a;
		return res*=scalar;
	}
	friend inline Target operator/(const Target &a,const Scalar &scalar)
	{
		Target res=a;
		return res*=(Scalar(1)/scalar);
	}
};
// ����������� ��������� ���� � ������� [-pi,pi]
inline float clampAngle(float angle)
{
	if(angle>=M_PI)	angle-=M_PI*2;
	if(angle<-M_PI)	angle+=M_PI*2;
	return angle;
}

struct AngleRange
{
	static float RangeMin()
	{
		return -M_PI;
	}
	static float RangeMax()
	{
		return M_PI;
	}
	static float RangeLength()
	{
		return RangeMax()-RangeMin();
	}
};
// contains angle segment. If max<min - it is splitted on two segments [min,pi]+[-pi,max]
// all values are locked in [-pi,pi] segment
struct Segment: public EqualTraits<Segment>
{
	typedef AngleRange RangeDesc;
	float min,max;
	////////////////////////////////////
	// strict boundaries:
	// smin=true,smax=true:		range=(min,max)
	// smin=true,smax=false:	range=(min,max]
	// to be implemented
	bool smin,smax;	
	// set sero range
	Segment():min(0),max(0),smin(true),smax(true){}
	Segment(float mi,float ma):min(mi),max(ma),smin(false),smax(false)
	{
		fix();
	}
	Segment(float _min,bool _smin,float _max,bool _smax):min(_min),max(_max),smin(_smin),smax(_smax)
	{
		fix();
	}
	Segment(const Segment &seg):min(seg.min),max(seg.max),smin(seg.smin),smax(seg.smax)
	{}
	// return full segment
	inline static Segment makeFull()
	{
		Segment res;
		res.max = M_PI;
		res.min = -M_PI;
		res.smin = false;
		res.smax = false;
		return res;
	}
	// return number
	inline static Segment makeSingle(float v)
	{
		Segment res;
		res.max = v;
		res.min = v;
		res.smin = false;
		res.smax = false;
		return res;
	}
	// return zero segment
	inline static Segment makeZero()
	{
		Segment res;
		res.max = 0;
		res.min = 0;
		res.smin = true;
		res.smax = true;
		return res;
	}
	// if range is empty
	bool zero()const
	{
		return max==min && smin && smax;
	}
	// if range is single number
	bool single()const
	{
		return max==min && !smin && !smax;
	}
	// if range is full circle
	bool full()const	
	{
		return fabs(max-min)==RangeDesc::RangeLength() && (!smin || !smax);
	}
	inline void fix()
	{
		min = clampAngle(min);
		max = clampAngle(max);
	}
	// ����� ����������
	float length()const
	{
		return (min > max)?RangeDesc::RangeLength() + max - min:max - min;
	}
	float mid()const
	{
		return (min > max)?(RangeDesc::RangeLength() + min + max)/2:(min + max)/2;
	}
	static float clamp(float angle)
	{
		const float rmax = RangeDesc::RangeMax();
		const float rmin = RangeDesc::RangeMin();

		if(angle > rmax)
			angle -= RangeDesc::RangeLength();
		if(angle < rmin)	
			angle += RangeDesc::RangeLength();
		return angle;
	}
	/// return nearest border
	float nearestBorder(float val)const
	{
		///// do not try to make call with inversed segment - one day you'll get stack overflow
		//if(!contains(val,false))
		//	return (~(*this)).nearestBorder(val);

		val = clamp(val);
		const float rmax = RangeDesc::RangeMax();
		const float rmin = RangeDesc::RangeMin();

		if(min > max) // if segment crosses max range
		{
			/// if contain value
			if(val <= rmax && val >= min) return min;
			if(val >= rmin && val <= max) return max;			
			// if value is outside
			return val - max < min - val? max : min;
		}
		else
		{
			if (val >= min && val <= max)	// if contains value
				return ( max - val < val - min )? max : min;			
			else							// if value is outside 
			{
				/// check if it belongs to [rmin, min] 
				if( val < min && val > rmin)
					return ( min - val < (val - rmin + rmax) - max )? min : max;
				else /// or [max, rmax]
					return ( val - max < min - ( rmin + val - rmax ) )? max : min;
			}
		}

		//assert(false);
		return 0;
	}
	bool containsStrict(float val) const
	{
		val = clamp(val);

		if(min > max) // if cross max range
		{		
			const float rmax = RangeDesc::RangeMax();
			const float rmin = RangeDesc::RangeMin();

			if((smin ? val>min : val>=min) && (val <= rmax))
				return true;
			if((val >= rmin) && (smax?val<max:val<=max))
				return true;
			else
				return false;			
		}
		else
		{
			return ((smin?val>min:val>=min) && (smax?val<max:val<=max));
		}	
	}
	// if contains point
	bool contains(float val, bool strict = true)const
	{
		if(strict)
			return containsStrict(val);

		val = clamp(val);

		if(min > max) // if cross max range
		{		
			const float rmax = RangeDesc::RangeMax();
			const float rmin = RangeDesc::RangeMin();

			if( val >= min && val <= rmax)
				return true;
			if( val >= rmin && val <= max)
				return true;
			else
				return false;			
		}
		else
		{
			return val >= min && val <= max;
		}		
	}
	// ���������� ��� ��������. ����� ��� ������ ��������� (Segments)
	struct Compare
	{
		inline bool operator()(const Segment &a,const Segment &b)const
		{
			return a.mid()<b.mid();
		}
	};
	// Relations for segments A and B
	enum Relation
	{
		Empty,				// no relations
		Overlap,			// A contains B
		Subset,				// B contains A
		LowerIntersection,	// A after B, single intersection
		UpperIntersection,	// A before B, single intersection
		DualIntersection,	// both sides intersection
		Equal,				// A equal B
		Complementary,		// A = !B
		//Error,			// Maybe?
	};
	static Relation classify(const Segment &a,const Segment &b)
	{
		bool lower=a.contains(b.min);
		bool upper=a.contains(b.max);
		bool lowerB=b.contains(a.min);
		bool upperB=b.contains(a.max);

		// to be improved by strict boundaries
		if(a==b)
			return Equal;
		if(a.min==b.max && a.max==b.min && a.smin!=b.smax && a.smax!=b.smin)
			return Complementary;
		else if(!lower && !upper && !lowerB && !upperB)
			return Empty;
		else if( lower &&  upper &&  lowerB &&  upperB)
			return DualIntersection;
		else if( lower &&  upper /*&& !lowerB && !upperB*/)
			return Overlap;
		else if(/*!lower && !upper && */ lowerB &&  upperB)
			return Subset;
		
		else if( lower && !upper)
			return UpperIntersection;
		else if(!lower &&  upper)
			return LowerIntersection;
		return Empty;
	}
	// segment addition
	inline static int and(const Segment &a,const Segment &b,Segment result[2])
	{	
		//Relation relation=classify(a,b);
		switch(classify(a,b))
		{
		case Empty:
			result[0]=a;
			result[1]=b;
			return 2;
		case Overlap:
		case Equal:
			result[0]=a;
			return 1;
		case Subset:		
			result[0]=b;
			return 1;
		case Complementary:
		case DualIntersection:
			result[0]=makeFull();
			return 1;
		case UpperIntersection:
			result[0]=Segment(a.min,a.smin,b.max,b.smax);
			return 1;
		case LowerIntersection:
			result[0]=Segment(b.min,b.smin,a.max,a.smax);
			return 1;
		}
		return 0;
	}
	inline static int or(const Segment &a,const Segment &b,Segment result[2])
	{
		switch(classify(a,b))
		{
		case Empty:
		case Complementary:
			return 0;
		case Overlap:
		case Equal:
			result[0]=b;
			return 1;
		case Subset:
			result[0]=a;
			return 1;
		case DualIntersection:
			result[0]=Segment(b.min,b.smin,a.max,a.smax);
			result[1]=Segment(a.min,a.smin,b.max,b.smax);
			return 2;
		case UpperIntersection:
			result[0]=Segment(b.min,b.smin,a.max,a.smax);			
			return 1;
		case LowerIntersection:
			result[0]=Segment(a.min,a.smin,b.max,b.smax);
			return 1;
		}
		return 0;
	}
	static inline bool equal(const Segment &a,const Segment &b)
	{
		return a.min==b.min && a.max==b.max && a.smin==b.smin && a.smax==b.smax;
	}
	friend Segment operator~(const Segment &seg);
};	

/// ���������� �� �������
inline Segment operator~(const Segment & seg)
{
	Segment result;
	result.smax = !seg.smax;
	result.smin = !seg.smin;
	result.max = seg.min;
	result.min = seg.max;
	return result;
}

// ������������ ����� ���������. . 
struct Segments:public set<Segment,Segment::Compare>
{
	typedef set<Segment,Segment::Compare> Parent;
	Segments(){}
	Segments(const Segment &seg)
	{
		insert(seg);
	}
	inline Segments(const Segments &seg)
		:Parent(seg.begin(),seg.end())
	{}
	// ������ ���������
	static Segments makeFull()
	{
		Segments result;
		result.insert(Segment::makeFull());
		return result;
	}
	static Segments makeSingle(float val)
	{
		Segments result;
		result.insert(Segment::makeSingle(val));
		return result;
	}
	static Segments makeZero()
	{
		Segments result;
		result.insert(Segment::makeZero());
		return result;
	}

	bool isFull()const
	{
		return !empty() && begin()->full();
	}

	bool isEmpty()const
	{
		return empty();
	}

	bool isSingle()const
	{
		return !empty() && begin()->single();
	}

	void assign(const Segments &seg)
	{
		clear();
		this->insert(seg.begin(),seg.end());
	}
	bool contains(float angle)const
	{
		for(const_iterator it=cbegin();it!=cend();it++)
			if(it->contains(angle))
				return true;
		return false;
	}
	// �����������. ����� �� ��������
	Segments & operator|=(const Segment &seg)
	{
		
		Segment b=seg;
		Segments result;
		bool merged=false;
		if(b.full())
		{
			assign(makeFull());
			return *this;
		}
		for(iterator it=begin();it!=end();it++)
		{
			if(b.zero())
				break;
			const Segment &a=*it;
			Segment::Relation relation=Segment::classify(a,b);
			if(relation==Segment::Empty)
				result.insert(a);
			else if(relation==Segment::Overlap || relation==Segment::Equal)
			{
				result.insert(a);
				b=Segment::makeZero();				
			}
			else if(relation==Segment::Subset)
				continue;
			else if(relation==Segment::Complementary|| relation==Segment::DualIntersection)
			{
				assign(makeFull());
				return *this;
			}
			else if(relation==Segment::UpperIntersection)
				b=Segment(a.min,a.smin,b.max,b.smax);
			else if(relation==Segment::LowerIntersection)
				b=Segment(b.min,b.smin,a.max,a.smax);		
		}
		if(!b.zero())
			result.insert(b);			
		assign(result);
		return *this;
	}
	// �����������. ������� �� ��������, �� � �� �����������. 
	Segments & operator&=(const Segment &seg)
	{
		Segment b=seg;
		Segments result;
		Segment opResult[2];
		bool merged=false;
		for(iterator it=begin();it!=end();it++)
		{
			if(b.zero())
				break;
			const Segment &a=*it;
			Segment::Relation relation=Segment::classify(a,b);
			if(relation==Segment::Empty)			
				continue;
			else if(relation==Segment::Overlap || relation==Segment::Equal)
			{
				result.insert(b);
				b=Segment::makeZero();
			}
			else if(relation==Segment::Subset)
				b=Segment::makeZero();
			else if(relation==Segment::Complementary|| relation==Segment::DualIntersection)
			{
				assign(makeFull());
				return *this;
			}
			else if(relation==Segment::UpperIntersection)
				b=Segment(a.min,a.smin,b.max,b.smax);
			else if(relation==Segment::LowerIntersection)
				b=Segment(b.min,b.smin,a.max,a.smax);		
		}
		assign(result);
		return *this;
	}
};

template<class _Real> struct _Range: public EqualTraits< _Range<_Real> >
{
	typedef _Real Real;
	typedef const Real &crReal;
	typedef _Range range_type;
	typedef const range_type &crRange;
	enum SideType
	{
		Basic,	// []
		Strict,	// ()
		Inf,	// +inf for max or -inf for min
	}tmin,tmax;

	enum Relation
	{
		Before,				// A before B
		After,				// A after B
		Empty,				// no relations
		Overlap,			// A contains B
		Subset,				// B contains A
		LowerIntersection,	// A after B, single intersection
		UpperIntersection,	// A before B, single intersection
		Equal,				// A equal B
		Complementary,		// A = !B
	};
	Real min,max;
	_Range(){}
	_Range(crReal a,crReal b)
	{
		
		min=a;
		max=b;
		tmin=Basic;
		tmax=Basic;
		if(max<min)	std::swap(max,min);		
	}
	_Range(crReal a,SideType ta,crReal b,SideType tb)
	{
		min=a;
		tmin=ta;
		max=b;
		tmax=tb;
		if(max<min)	std::swap(max,min);		
	}
	_Range(const range_type &range):min(range.min),max(range.max),tmin(range.tmin),tmax(range.tmax){}
	static inline range_type makeFull()
	{
		return range_type(0,Inf,0,Inf);			// [-inf,+inf]
	}
	static inline range_type makeZero()
	{
		return range_type(0,Strict,0,Strict);	// (0,0)
	}
	static inline range_type makeNumber(crReal v)
	{
		return range_type(v,Basic,v,Basic);		// [v,v]
	}
	static inline range_type makeInf(crReal v,bool max)
	{
		return max?range_type(v,Basic,v,Inf):range_type(v,Inf,v,Basic);
	}
	// if range is single number [val,val]
	inline bool isNumber()const
	{
		return tmin==Basic && tmax==Basic && min==max;
	}
	// if [-inf,+inf]
	inline bool isFull()const
	{
		return tmin==Inf && tmax==Inf;
	}
	// if any bound is inf
	inline bool isInf()const
	{
		return tmin==Inf || tmax==Inf;
	}
	// if range is empty
	inline bool isZero()const
	{
		return tmin==Strict && tmax==Strict && min==max;
	}
	// if val is lower than min bound
	inline bool isLesser(crReal val) const
	{
		if(tmin==Basic)
			return val<min;
		else if(tmin==Strict)
			return val<=min;
		return false;							// for -Inf
	}
	inline Real mid()const
	{
		assert(!isInf());
		return (min+max)/2;
	}
	inline Real length()const
	{
		assert(!isInf() && !isZero());
		return (max-min)/2;
	}
	inline bool isHigher(crReal val) const
	{
		if(tmax==Basic)
			return val>max;
		else if(tmax==Strict)
			return val>=max;
		return false;							// for +Inf
	}
	inline bool contains(crReal val) const
	{
		return !isLesser(val) && !isHigher(val);
	}
	static inline bool touchMinMax(crRange a,crRange b)
	{
		return a.min==b.max && ((a.tmin==Strict && b.tmax==Basic) || (a.tmin==Basic && b.tmax==Strict));
	}
	static inline bool touchMaxMin(crRange a,crRange b)
	{
		return a.max==b.min && ((a.tmax==Strict && b.tmin==Basic) || (a.tmax==Basic && b.tmin==Strict));
	}
	static Relation relation(crRange a,crRange b)
	{
		if(equal(a,b))
			return Equal;
		if(	(a.tmin==Inf && b.tmax==Inf && touchMaxMin(a,b))||
			(a.tmax==Inf && b.tmin==Inf && touchMinMax(a,b)))
			return Complementary;
		if(a.isFull())
			return Overlap;
		if(b.isFull())
			return Subset;

		if(!a.isInf() && !b.isInf())
		{			
			if( a.isLesser(b.min) &&  a.isLesser(b.max))
				return After;
			if( a.isHigher(b.min) &&  a.isHigher(b.max))
				return Before;
			if( a.isLesser(b.min) &&  a.isHigher(b.max))
				return Subset;
			if(!a.isLesser(b.min) && !a.isHigher(b.max))
				return Overlap;
			if( a.isLesser(b.min) &&  a.contains(b.max))
				return LowerIntersection;
			if( a.isHigher(b.max) &&  a.contains(b.min))
				return UpperIntersection;
		}
		return Empty;
	}
	static inline bool equal(crRange a,crRange b)
	{
		return a.min==b.min && a.max==b.max && a.tmin==b.tmin && a.tmax==b.tmax;
	}
	// ���������� ��� ��������. ����� ��� ������ ��������� (Segments)
	struct Compare
	{
		inline bool operator()(const range_type &a,const range_type &b)const
		{
			return a.tmax==Inf || b.tmax!=Inf || a.max>b.max;
		}
	};
};

// ����� �����������. ������ ��������� ��������������� � ���������������� �������� � ���� ��������
template<class _RangeType> class _RangeSet
{
public:
	typedef typename _RangeType::Real Real;
	typedef _RangeType Range;
	typedef const Range &crRange;
	typedef _RangeSet<_RangeType> my_type;
	//typedef std::set<Range,typename Range::Compare> Ranges;
	typedef std::list<Range> Ranges;
	Ranges ranges;
public:
	_RangeSet(){}
	_RangeSet(crRange range)
	{
		ranges.insert(range);
	}
	_RangeSet(const _RangeSet &rs)
	{
		ranges.assign(rs.ranges.begin(),rs.ranges.end());
	}
	void clear()
	{
		ranges.clear();
	}
	static inline my_type makeFull()
	{			
		return my_type(Range::makeFull());
	}
	static inline my_type makeZero()
	{
		return my_type();
	}
	static inline my_type makeNumber(const Real &v)
	{
		return my_type(Range::makeNumber(v));
	}
	bool isZero()const
	{
		return ranges.empty();
	}
	bool isInf()const
	{
		if(isZero())return false;
		return ranges.front().isInf() || ranges.back().isInf();
	}
	// �����������. �� ������ ����� �� ��������
	my_type & operator|=(crRange range)
	{
		Range b = range;
		Ranges::iterator it = ranges.begin();
		while(it != ranges.end() && !b.isZero())
		{
			Range &a = *it;
			Range::Relation rel=Range::relation(a,b);
			if(rel==Range::Subset)				//
				it = ranges.erase(it);
			else if(rel==Range::LowerIntersection)
			{
				a = Range(b.min,b.tmin,a.max,a.tmax);
				//b=Range::makeZero();
				//break;
				return *this;
			}
			else if(rel==Range::UpperIntersection)
			{
				b = Range(a.min,a.tmin,b.max,b.tmax);
				it = ranges.erase(it);
			}
			//else if(rel==Range::Before)
			//	break;
			else if(rel==Range::After)
				break;
			else if(rel==Range::Complementary)	// make full
			{
				ranges.clear();
				ranges.push_back(Range::makeFull());
				break;
			}
			else if(rel==Range::Equal || rel==Range::Overlap)
			{
				return (*this);
				//b=Range::makeZero();
				//break;
			}
			else		//(rel==Range::Before)
				it++;
		}
		if(!b.isZero())
			ranges.insert(it,b);	// insert b before it

		return *this;
	}
	Real min()const
	{
		return ranges.front().min;
	}
	Real max()const
	{
		return ranges.back().max;
	}
	
	// �����������
	my_type & operator &=(crRange range)
	{
		Range b=range;
		Ranges::iterator it=ranges.begin();
		while(it != ranges.end() && !b.isZero())
		{
			Range &a=*it;
			Range::Relation rel=Range::relation(a,b);
			if(rel==Range::Subset)				//+
				it++;
			else if(rel == Range::LowerIntersection)//+
			{
				a = Range(b.max,b.tmax,a.min,a.tmin);
				//b=Range::makeZero();
				break;
//				return *this;
			}
			else if(rel == Range::UpperIntersection)//+
			{
				a = Range(b.min,b.tmin,a.max,a.tmax);
				it++;
				//it=ranges.erase(it);				
			}
			else if(rel == Range::Before)//+
			{
				it = ranges.erase(it);
			}
			else if(rel == Range::After)//+
			{
				it=ranges.erase(it);
			}
			else if(rel == Range::Complementary)	//+ make full
			{
				ranges.clear();
				//ranges.push_back(Range::makeFull());
				break;
			}
			else if(rel == Range::Equal ||rel == Range::Overlap) //+
			{
				ranges.clear();
				ranges.push_back(b);
				break;
			}
			//else if(rel==Range::Overlap)
			//	break;
			else		//(rel==Range::Before)
				it++;
		}
		//if(!b.isZero())
		//	ranges.insert(it,b);	// insert b before it

		return *this;
	}
};
}// namespace frosttools
#endif

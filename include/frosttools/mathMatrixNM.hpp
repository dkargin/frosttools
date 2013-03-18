#pragma once
#ifndef MATH_MATRIX
#error "include <mathMatrix.hpp> first"
#endif

template<class Real>
class MatrixNM
{
	typedef MatrixOrder<Real,true> Base;
	typedef Real value_type;
	typedef MatrixNM<Real> my_type;
	typedef my_type vector_type;
	typedef my_type column_type;
	typedef my_type row_type;
	typedef my_type transposed_type;
	int width;
	int height;
	Real *data;
public:
	Real * getData()
	{
		return data;
	}
	MatrixNM()
		:width(0),height(0),data(NULL)
	{}
	MatrixNM(int c,int r)
	{
		width=c;
		height=r;
		data=new Real[c*r];
	}
	MatrixNM(int c,int r,Real *d)
	{
		width=c;
		height=r;
		data=new Real[c*r];
		for(int i=0;i<c*r;i++)
			data[i]=d[i];
	}
	MatrixNM(const MatrixNM &m)
	{
		width=m.width;
		height=m.height;
		data=new Real[width*height];
		for(int i=0;i<width*height;i++)
			data[i]=m.data[i];
	}
	~MatrixNM()
	{
		if(data)
			delete []data;
	}
	int cols() const
	{
		return width;
	}
	int rows() const
	{
		return height;
	}
	Real & operator()(int c,int r)
	{
		return Base::get(data,cols(),rows(),c,r);
	}
	const Real & operator()(int c,int r) const
	{
		return Base::get(data,cols(),rows(),c,r);
	}
	MatrixNM& operator += ( const MatrixNM& a)
	{
		for ( register int i = 0; i < cols; i++ )
			for ( register int j = 0; j < rows; j++ )
				this->x [i][j] += a.x [i][j];
		return *this;
	}
	MatrixNM& operator -= ( const MatrixNM& a)
	{
		for ( register int i = 0; i < cols; i++ )
				for ( register int j = 0; j < rows; j++ )
					this->x [i][j] -= a.x [i][j];

		return *this;
	}
	MatrixNM& operator *= ( const Real& v)
	{
		for ( register int i = 0; i < cols; i++ )
			for ( register int j = 0; j < rows; j++ )
				this->x [i][j] *= v;

		return *this;
	}	
	void swapRows(int a,int b)
	{
		assert(a<rows && b<rows);
		Real tmp;
		for(int i=0;i<cols;i++)
		{
			tmp=value(i,a);value(i,a)=value(i,b);value(i,b)=tmp;
		}
	}
	void swapCols(int a,int b)
	{
		assert(a<cols && b<cols);
		Real tmp;
		for(int i=0;i<rows;i++)
		{
			tmp=value(a,i);value(a,i)=value(b,i);value(b,i)=tmp;
		}
	}
	//void readCol(int n,Real *c)
	//{
	//	assert(n<cols);
	//	for(int i=0;i<rows;i++)
	//		c[i]=value(n,i);
	//}
	//void writeCol(int n,Real *c)
	//{
	//	assert(n<cols);
	//	for(int i=0;i<rows;i++)
	//		value(n,i)=c[i];
	//}
	//void readRow(int n,Real *c)
	//{
	//	assert(n<rows);
	//	for(int i=0;i<cols;i++)
	//		c[i]=value(i,n);
	//}
	//void writeRow(int n,Real *c)
	//{
	//	assert(n<rows);
	//	for(int i=0;i<cols;i++)
	//		value(i,n)=c[i];
	//}
	Real & value(int c,int r)
	{
		assert(c<cols && r<rows);
		return data[c+cols*r];
	}
};


template<class Real>
MatrixNM<Real> operator + (const MatrixNM<Real> &a,const MatrixNM<Real> &b)
{
	MatrixNM<Real> res(a);
	res+=b;
	return res;
}

template<class Real>
MatrixNM<Real> operator - (const MatrixNM<Real> &a,const MatrixNM<Real> &b)
{
	MatrixNM<Real> res(a);
	res-=b;
	return res;
}

template<class Real>
MatrixNM<Real> operator * (const MatrixNM<Real> &a,const Real &b)
{
	MatrixNM<Real> res(a);
	res*=b;
	return res;
}

template<class Real>
MatrixNM<Real> operator / (const MatrixNM<Real> &a,const Real &b)
{
	MatrixNM<Real> res(a);
	res/=b;
	return res;
}

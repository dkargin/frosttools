#pragma once
#define MATH_MATRIX

// defines operations based on Real scalar type
template<class Real,bool order> struct MatrixOrder
{
	typedef typename MathTypes<Real>::size_type size_type;
	template<class R> inline static void getCol(const Real *data,size_type size,size_type c,R *col);
	template<class R> inline static void setCol(Real *data,size_type size,size_type c,const R *col);
	template<class R> inline static void getRow(const Real *data,size_type size,size_type r,R *row);
	template<class R> inline static void setRow(Real *data,size_type size,size_type r,const R *row);
};
///////////////////////////////////////////////////////////////////////////
/// specialisation for row-ordered matrix
///////////////////////////////////////////////////////////////////////////
template<class Real> struct MatrixOrder<Real,true>
{
	typedef typename MathTypes<Real>::size_type size_type;
	inline static const Real & get(const Real *data,size_type sx,size_type sy,size_type x,size_type y)
	{
		assert(x>=0 && x<sx && y<sy && y>=0);
		return data[x+sx*y];
	}
	inline static Real & get(Real *data,size_type sx,size_type sy,size_type x,size_type y)
	{
		assert(x>=0 && x<sx && y<sy && y>=0);
		return data[x+sx*y];
	}
	template<class R> inline static void getCol(const Real *data,size_type sx,size_type sy,size_type c,R *res)
	{
		assert(c<sx);
		const Real *ptr=data+c;
		for(size_type j=sy;j;j--,ptr+=sx)
			*res++=*ptr;
	}
	template<class R> inline static void setCol(Real *data,size_type sx,size_type sy,size_type c,const R *res)
	{
		assert(c<sx);
		Real *ptr=data+c;
		for(size_type j=sy;j;j--,ptr+=sx)
			*ptr=*res++;
	}
	template<typename R> inline static void getRow(const Real *data,size_type sx,size_type sy,size_type r,R *res)
	{
		assert(r<sy);
		for(size_type j=0;j<sx;j++)
			res[j]=data[r*sx+j];
	}	
	template<typename R> inline static void setRow(Real *data,size_type sx,size_type sy,size_type r,const R *res)
	{
		assert(r<sy);
		for(size_type j=0;j<sx;j++)
			data[r*sx+j]=res[j];
	}
};

///////////////////////////////////////////////////////////////////////////
/// specialisation for column-ordered matrix
///////////////////////////////////////////////////////////////////////////
template<class Real> struct MatrixOrder<Real,false>
{
	typedef typename MathTypes<Real>::size_type size_type;
	inline static const Real & get(const Real *data,size_type sx,size_type sy,size_type x,size_type y)
	{
		assert(x>=0 && x<sx && y<sy && y>=0);
		return data[sy*x+y];
	}
	inline static Real & get(Real *data,size_type sx,size_type sy,size_type x,size_type y)
	{
		assert(x>=0 && x<sx && y<sy && y>=0);
		return data[sy*x+y];
	}
	template<typename R> inline static void getCol(const Real *data,size_type sx,size_type sy,size_type c,R *res)
	{
		assert(c<sx);
		const Real *ptr=data+c*sy;
		for(size_type j=sx;j>0;j--)
			*res++=*ptr++;
	}
	template<typename R> inline static void setCol(Real *data,size_type sx,size_type sy,size_type c,const R *res)
	{
		assert(c<sx);
		Real *ptr=data+c*sy;
		for(size_type j=sx;j>0;j--)
			*ptr++=*res++;
	}
	// still slow variant
	template<typename R> inline static void getRow(const Real *data,size_type sx,size_type sy,size_type r,R *res)
	{
		assert(r<sy);
		for(size_type j=0;j<sy;j++)
			res[j]=data[j*sy+r];
	}	
	// still slow variant
	template<typename R> inline static void setRow(Real *data,size_type sx,size_type sy,size_type r,const R *res)
	{
		assert(r<sy);
		for(size_type j=0;j<sy;j++)
			data[j*sy+r]=res[j];
	}
};

// generic row order matrix
template<class Storage,bool row_order> class MatrixBase: public Storage
{
public:	
	typedef typename Storage::col_type col_type;
	typedef typename Storage::row_type row_type;
	typedef typename Storage::value_type value_type;
	typedef int size_type;

	typedef MatrixOrder<value_type,row_order> Order;
	typedef MatrixBase<Storage,row_order> matrix_type;
	typedef MatrixBase<Storage,row_order> my_type;

	inline static bool rowOrder()
	{
		return row_order;
	}	
	value_type & operator()(size_type x,size_type y)
	{
		return Order::get(this->c,Storage::cols(),Storage::rows(),x,y);
	}
	const value_type & operator()(size_type x,size_type y) const
	{
		return Order::get(this->c,Storage::cols(),Storage::rows(),x,y);
	}
	value_type get(size_type x,size_type y) const
	{
		return Order::get(this->c,Storage::cols(),Storage::rows(),x,y);
	}
	void set(size_type x,size_type y,const value_type &r)
	{
		Order::get(this->c,Storage::cols(),Storage::rows(),x,y)=r;
	}
	// for NxM matrix compatibility
	template<class R>inline void getCol(int i,R *res) const
	{
		Order::getCol(this->c,Storage::cols(),Storage::rows(),i,res);
	}
	template<class R>inline void setCol(int i,const R *vec)
	{
		Order::setCol(this->c,Storage::cols(),Storage::rows(),i,vec);
	}
	col_type col(int i) const
	{		
		col_type res = Storage::make_col();
		Order::getCol(this->c,Storage::cols(),Storage::rows(),i,(value_type*)res);
		return res;
	}	
	template<class tVector>	void col(int i,const tVector &vec)
	{		
		Order::setCol(this->c,Storage::cols(),Storage::rows(),i,(const value_type*)vec);
	}
	template<class R> void getRow(int i,R *res) const
	{
		Order::getRow(this->c,Storage::cols(),Storage::rows(),i,res);
	}
	template<class R> void setRow(int i,const R *res)
	{
		Order::setRow(this->c,Storage::cols(),Storage::rows(),i,res);
	}
	row_type row(int i) const
	{
		row_type res = Storage::make_row();
		Order::getRow(this->c,Storage::cols(),Storage::rows(),i,(value_type*)res);
		return res;
	}
	template<class tVector> void row(int i,const tVector &vec)
	{		
		Order::setRow(this->c,Storage::cols(),Storage::rows(),i,(const value_type*)vec);
	}
	operator value_type *()
	{
		return Storage::ptr();
	}
	operator const value_type *() const
	{
		return Storage::ptr();
	}
	value_type det() const
	{
		const int ncols = Storage::cols();
		const int nrows = Storage::rows();

		if(nrows != ncols)
			return 0;
	    //const Real *c=(const Real*)this;

		value_type result(0);

		for(int i=0; i < ncols; i++)
		{
			value_type pos,neg;
			neg=pos=row(0,i);
			for(int j=1,p=i+i,n=i-1;j < nrows;j++,p++)
			{
				if(p >= ncols)
					p=0;
				if(n < 0)
					n = ncols - 1;
				neg*=get(n,j);
				pos*=get(p,j);
			}
			result=result+pos-neg;
		}
		return result;		
	}	
	void swapRows(int a,int b)
	{
		assert(a<Storage::rows() && b<Storage::rows());
		row_type tmp = Storage::make_row();
		row_type r = Storage::make_row();
		getRow(a,(value_type*)tmp);
		getRow(b,(value_type*)r);
		setRow(a,(value_type*)r);
		setRow(b,(value_type*)tmp);
	}
	void swapCols(int a,int b)
	{
		assert(a<Storage::cols() && b<Storage::cols());
		col_type tmp = Storage::make_col();
		col_type r = Storage::make_col();
		getCol(a,(value_type*)tmp);
		getCol(b,(value_type*)r);
		setCol(a,(value_type*)r);
		setcol(b,(value_type*)tmp);
	}
};

////////////////////////////////////////////////////////
// Variable size matrix
////////////////////////////////////////////////////////
template<class Real>
class MatrixNM: public MatrixBase<StorageDynamic<Real >,true>
{
public:
	MatrixNM(){};
	MatrixNM(int w,int h)
	{
		assign_val(Real(0),w,h);
	}
	MatrixNM(const MatrixNM &m)
	{
		assign(m.c,m.cols(),m.rows());
	}
	template<class R>MatrixNM(const R* data,int w,int h)
	{
		assign(data,w,h);
	}
};
//template<> struct mult_res<Matrix<Real,X,Y>,Matrix<Real,Y,Z>
//{
//	typedef Matrix<Real,X,Z> type;
//};
////////////////////////////////////////////////////////
// Constant size matrix
////////////////////////////////////////////////////////
// ���������, �� ��� ���� ��� ������ ������� ��� column-order?
template<class Real,int _X,int _Y,bool order>
class Matrix: public MatrixBase<StorageStatic<Real,_X,_Y>,order>
{
public:
	enum{X=_X,Y=_Y};
	typedef int size_type;
	typedef Real value_type;
	typedef MatrixBase<StorageStatic<Real,X,Y>,order> parent_type;
	typedef Matrix<Real,X,Y,order> my_type;
	typedef my_type matrix_type;
	typedef Matrix<Real,Y,X,order> transposed_type;
	

	my_type & operator = (const my_type &m)
	{
		this->assign((const value_type*)m);
		return *this;
	}	
	
	/*template<typename Real,bool row_order>
	typename Matrix<Real,N,M,row_order>::col_type operator *= ( const Matrix<Real,N,M,row_order>& m, const typename Matrix<Real,N,M,row_order>::row_type & v )
	{
		typename Matrix<Real,N,M,row_order>::col_type res=v[0]*m.col(0);
		for(int i=1;i<m.cols();i++)
		//{
			//typename Matrix<Real,N,M,row_order>::col_type c=m.col(i);
			res+=(v[i]*m.col(i));
		//}
		return *this;
	}*/
};
////////////////////////////////////////////////////////
// generic Square matrix
////////////////////////////////////////////////////////
template<class Real,int N,bool order>
class MatrixSquare: public Matrix<Real,N,N,order>
{
public:
	typedef MatrixSquare<Real,N,order> my_type;
	typedef my_type transposed_type;

	static my_type identity(Real v=Real(1))
	{	
		my_type res;
		int size_x = N;
		for ( int i = 0, ind = 0; i < res.size(); i++)
			if(i==ind)
			{
				res.c[i]=v;
				ind+=(size_x+1);
			}
			else
				res.c[i]=Real(0);
		return res;
	}

	inline typename my_type::col_type project(const typename my_type::row_type &a)const//returns a coordinates in this system
	{
		typename my_type::col_type res;
		for(int i = 0; i < this->cols(); i++)
			res[i]=::vecProjectLen(a, this->col(i));
		return res;
//		return vec3(::vecProjectLen(a-origin(),col(0)),
//					::vecProjectLen(a-origin(),axisY()),
//					::vecProjectLen(a-origin(),axisZ()));
	}
	my_type & operator *=(const my_type &b)
	{
		const int X = this->cols();
		const int Y = this->rows();
		my_type res;
		for(int i=0;i < X;i++)
			for(int j=0;j < Y;j++)
				res(i,j)=this->row(j)&b.col(i);
		*this=res;
		return *this;
	}
};
//////////////////////////////////////////////////////////////////
// Reduce to triangle form, using Gauss method, in order[] return actual row order
//////////////////////////////////////////////////////////////////
template<class _Matrix> _Matrix reduceToTriangle(const _Matrix &source,int *order=NULL)
{
	_Matrix result(source);
	/// no row swapping?
	/// 
	/*
	���� 1� �������, �������� �� ������ ����� �������, ����� ��������� 1� ������������ = 0
	*/
	int x,y;
	int i=0;

	if(order)
		for(int r=0;r<source.rows();r++)
			order[r]=r;
	while(true)
	{
		typename _Matrix::value_type v;
		// find first unalligned position
		for(x=i;x<source.cols();x++)
			for(y=i;y<source.rows();y++)
				if(v=result(x,y))
					goto swap;					
		break;	// if we got here - stop algorithm
swap:		
		if(y!=i)
		{
			result.swapRows(i,y);
			if(order)
			{
				int tmp = order[i]; order[i] = order[y]; order[y] = tmp;
				//std::swap(order[i],order[y]);
			}
		}
		i=x;
		for(y=i+1;y<source.rows();y++)
		{
			typename _Matrix::value_type t=result(i,y);
			if(!t)continue;
			t/=v;
			for(x=i;x<source.cols();x++)
				result(x,y)=result(x,y)-t*result(x,i);
		}
		i++;
	}
	for(y=0;y<source.rows();y++)
	{
		typename _Matrix::value_type v=result(y,y);
		if(!v)
			break;
		for(x=y;x<source.cols();x++)
			result(x,y)/=v;
	}
	return result;
}
template<class _Matrix>int realRows(const _Matrix &source)
{
	typedef typename _Matrix::value_type value_type;
	int y=source.rows()-1;
	static const value_type zero(0);
	for(;y>=0;y--)
	{
		for(int x=0;x<source.cols();x++)
			if(source(x,y)!=zero)
				goto end;
	}
end:
	return y+1;
}
template<class _Matrix>int rank(const _Matrix &source)
{
	return realRows(reduceToTriangle(source));
}
//////////////////////////////////////////////////////////////////
// Gramm-Shmidt orthonormalisation
//////////////////////////////////////////////////////////////////
template<class _Matrix> _Matrix orthonormalise(const _Matrix &source)
{
	assert(source.cols()==source.rows());	// only square matrices are allowed
	_Matrix result(source);
	const int size=source.cols();
	// normalise all columns
	typename _Matrix::col_type axis = vecNormalise(result.col(0));
	result.col(0,axis);
	// then we compute orthogonal axes.
	for(int i=0;i<size-1;i++)
	{
		typename _Matrix::col_type proj=vecProject(result.col(i+1),result.col(i));
		result.col(i+1,vecNormalise(result.col(i+1)-proj));
	}
	return result;
}
////////////////////////////////////////////////////////////
// rotate around matrix center
////////////////////////////////////////////////////////////
template<class _Matrix> _Matrix &rotate(const _Matrix &src,bool dir=true)
{
	assert(src.cols()==src.rows());	// only square matrices allowed
	int size_x = src.cols();
	int size_y = src.rows();
	_Matrix tmp;
	if(dir)
		for(int y=0,i=0;y<src.rows();y++)
			for(int x=0;x<src.cols();x++,i++)
			{
				int x1=size_y-y-1;
				int y1=x;
				tmp[x1+y1*size_x]=src.c[i];
			}
	else
		for(int y=0,i=0;y<src.rows();y++)
			for(int x=0;x<src.cols();x++,i++)
			{
				int x1=y;
				int y1=size_x-x-1;
				tmp[x1+y1*size_x]=src.c[i];
			}
	return tmp;
}
template<class _Matrix> _Matrix invert (const _Matrix &source)
{
	assert(source.cols()==source.rows());	// only square matrices are allowed
	_Matrix	out=_Matrix::identity(),tmp=source;
	typedef typename _Matrix::value_type value_type;
    const value_type * c=(const value_type *)source;
	const int size=source.cols();
	for ( int i = 0; i < size; i++ )
	{
		float	d = tmp(i,i);

		// make identity diagonal
		if ( d != 1.0f)
		{
			for ( int j = 0; j < size; j++ )
			{
				out(j,i) /= d;
				tmp(j,i) /= d;
			}
		}

		// ���������� ������, ���� ��������� �������
		for ( int j = 0; j < size; j++ )  // j- ������
			if ( j != i && tmp(i,j) != 0.0)	// ���� ��������� �������������� �������
			{
				float	mulBy =  tmp(i,j);
				// �������� i� ������� �� ���� ���������
				for ( int k = 0; k < size; k++ )
				{
					tmp(k,j) -= mulBy * tmp(k,i);
					out(k,j) -= mulBy * out(k,i);
				}
			}			
	}
	return out;
}

// Seems to be not completed
template<class _Matrix> _Matrix pseudoinvert (const _Matrix &source)
{
	int r=rank(source);
	return invert(source);
}
// slow ( but universal variant
template<class _Matrix> typename _Matrix::transposed_type transpose(const _Matrix &source)
{
	typename _Matrix::transposed_type result;
	typename _Matrix::col_type tmp;
	const int cols=source.cols();	
	const int rows=source.rows();

	for(int i=0;i<cols;i++)
		for(int j=0;j<rows;j++)
			result.set(j,i,source(i,j));
	return result;
}

//template<class Storage,bool row_order>
template<class Real,int X,int Y,int Z,bool order> Matrix<Real,X,Z,order> operator *( const Matrix<Real,X,Y,order> &a,const Matrix<Real,Y,Z,order> &b)
{
	Matrix<Real,X,Z,order> res;
	for(int i=0;i<X;i++)
		for(int j=0;j<Y;j++)
			res(i,j)=a.row(j)&b.col(i);
	return res;
}
//template<class Real,int X,int Y,int Z,bool order>
//Matrix<Real,X,Z,order> mult( const Matrix<Real,X,Y,order> &a,const Matrix<Real,Y,Z,order> &b)
//{
//	Matrix<Real,X,Z,order> res;
//	return res;
//}
//template<class MatrixA,class MatrixB>
//typename MatrixA::mult_res<MatrixB>::type 
//mult1( const MatrixA& a, const typename MatrixB& b )
//{
//	//typedef typename MatrixA::mult_res<MatrixB>::type result_type;
//	//typedef MatrixA result_type;
//	////const int size=N;
//	//result_type	res;
//
//	//if(row_order)
//	//for ( register int i = 0; i < size; i++ )
//	//	for ( register int j = 0; j < size; j++ )
//	//	{
//	//		Real sum(0);
//	//		for ( register int k = 0; k < size; k++ )
//	//			sum += a [i*size+k] * b [k*size+j];
//	//		res[i*size+j] = sum;
//	//	}
//	//else
//	//for ( register int i = 0; i < size; i++ )
//	//	for ( register int j = 0; j < size; j++ )
//	//	{
//	//		Real sum(0);
//	//		for ( register int k = 0; k < size; k++ )
//	//			sum += a [i+k*size] * b [k+j*size];
//	//		res[i+j*size] = sum;
//	//	}
////	return res;
//}
//
// �������� ������� �� ������-�������.
template<typename Real,int N,int M,bool row_order>
typename Matrix<Real,N,M,row_order>::col_type operator * ( const Matrix<Real,N,M,row_order>& m, const typename Matrix<Real,N,M,row_order>::row_type & v )
{
	typename Matrix<Real,N,M,row_order>::col_type res=v[0]*m.col(0);
	for(int i=1;i<m.cols();i++)
	//{
		//typename Matrix<Real,N,M,row_order>::col_type c=m.col(i);
		res+=(v[i]*m.col(i));
	//}
	return res;
}
typedef Matrix<float,4,4,true> Mt4x4r;
typedef Matrix<float,4,4,false> Mt4x4c;
//////////////////////// Derived functions /////////////////////////////

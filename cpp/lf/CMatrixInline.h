#include "my_lapack.h"

inline int imax(const int i1,const int i2) { return i1 > i2 ? i1 : i2; }

template<class T>
CMatrix<T>::CMatrix() : 
  _rows(0),_columns(0),_data(new T[_rows*_columns]), _ownsData(true)
{
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    } 
}

template<class T>
CMatrix<T>::CMatrix(int rows,int columns) : 
  _rows(rows),_columns(columns),_data(new T[_rows*_columns]), _ownsData(true)
{
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }
}

template<class T>
CMatrix<T>::CMatrix(int rows, int columns, T * data, bool ownsData) :
  _rows(rows),_columns(columns), _data(data), _ownsData(ownsData)
{ }

template<class T>
CMatrix<T>::CMatrix(const CMatrix & v) 
{ 
  _rows = v._rows;
  _columns = v._columns;
  _data = new T[_rows*_columns]; 
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1); 
    } 
  memcpy(_data,v._data,sizeof(T)*_rows*_columns);
}

template<class T>
CMatrix<T> & CMatrix<T>::operator=(const CMatrix & v)
{
  if (_rows != v._rows || _columns != v._columns)
    {
      delete [] _data;
      _rows = v._rows;
      _columns = v._columns;
      _data = new T[_rows*_columns];
      if (_data == NULL)
	{
	  printf("error\n");
	  exit(1);
	}
    }

  memcpy(_data,v._data,sizeof(T)*_rows*_columns);
  return *this;
}

template<class T>
T & CMatrix<T>::get(int i,int j)
{
  assert(i>=0 && i<_rows && j>=0 && j<_columns);
  return _data[i + j*_rows];
}
  
template<class T>
T CMatrix<T>::get(int i,int j) const
{
  assert(i>=0 && i<_rows && j>=0 && j<_columns);
  return _data[i + j*_rows];
}

template<class T>
CVector<T> CMatrix<T>::getRow(int i) const
{
  assert(i>=0 && i<_rows);
  double * data = new double[_columns];
  for(int j=0;j<_columns;j++)
    data[j] = get(i,j);
  return CVector<T>(_columns,data);
}

template<class T>
CVector<T> CMatrix<T>::getColumn(int j) const
{
  assert(j>=0 && j<_columns);
  double * data = new double[_rows];
  memcpy(data,&_data[j*_rows],_rows*sizeof(T));
  return CVector<T>(_rows,data);
}

template<class T>
CMatrix<T> CMatrix<T>::operator*(const CMatrix<T> & v) const
{
  assert(_columns == v._rows);

  T * data = new T[_rows*_columns];
  if (data == NULL)
    {
      printf("error\n");
      exit(1);
    }

  for(int i=0;i<_rows;i++)
    for(int j=0;j<v._columns;j++)
      {
	T & val = data[i+j*v._columns];
	val = 0;

	for(int k=0;k<_columns;k++)
	  val += get(i,k)*v.get(k,j);
      }

  return CMatrix<T>(_rows,v._columns,data);
}

template<class T>
CVector<T> 
CMatrix<T>::operator*(const CVector<T> & v) const
{
  assert(v.Length() == _columns);

  T * data = new T[_rows];
  if (data == NULL)
    {
      printf("error\n");
      exit(1);
    }

  for(int i=0;i<_rows;i++)
    {
      data[i] = 0;
      for(int j=0;j<_columns;j++)
	data[i] += get(i,j)*v[j];
    }

  return CVector<T>(_rows,data);
}

template<class T>
CMatrix<T>
CMatrix<T>::transpose() const
{
  CMatrix mat(_columns,_rows);
  for(int i=0;i<_rows;i++)
    for(int j=0;j<_columns;j++)
      mat.get(j,i) = get(i,j);

  return mat;
}


#ifdef LAPACK
inline CVector<double> 
CMatrix<double>::symmetricEigenvalues()
{
  assert(_rows == _columns);
  
  CMatrix<double> evecsfull(*this);
  
  double *eigsfull = new double[_rows];
  
  if (eigsfull == NULL)
    {
      printf("Can't allocate eigenvalues\n");
      exit(1);
    }

  // decompose into tridiagonal form
  char jobz = 'N';
  char uplo = 'U';
  int lwork = _rows*(_columns+2);
  double * work = new double[lwork];
  int info;

  if (work==NULL)
    {
      printf("error\n");
      exit(1);
    }

  DSYEV(&jobz,&uplo,&_rows,evecsfull._data,&_rows,eigsfull,work,&lwork,&info);

  if (info != 0)
    {
      printf("dsyev failed\n");
      exit(1);
    }
  
  delete [] work;

  return CVector<double>(_rows,eigsfull);
}


inline CVector<double> 
CMatrix<double>::symmetricEigenvectors(CMatrix<double> & evecs,
				      int maxVal, int minVal)
{
  int nVecs = maxVal - minVal + 1;
  
  assert(_rows == _columns);
  
  assert(evecs.rows() == _rows && evecs.columns() == nVecs);
  
  CMatrix<double> evecsfull(*this);
  
  double *eigsfull = new double[_rows];
  
  if (eigsfull == NULL)
    {
      printf("Can't allocate eigenvalues\n");
      exit(1);
    }

  // decompose into tridiagonal form
  char jobz = 'V';
  char uplo = 'U';
  int lwork = _rows*(_columns+2);
  double * work = new double[lwork];
  int info;
  if (work==NULL)
    {
      printf("error\n");
      exit(1);
    }
  //  DSYEVX(&jobz,&range,&uplo,&_rows,data._data,&_rows,&vl,&vu,&il,&iu,
  //	  0,&m,eigs,evecs._data,&_rows,work,&lwork,iwork,
  //	  ifail,&info);
  DSYEV(&jobz,&uplo,&_rows,evecsfull._data,&_rows,eigsfull,work,&lwork,&info);
  //  assert(m==nVecs);

     // we could also call the "expert" function to only extract some
     // eigenvalues

  if (info != 0)
    {
      printf("dsyev failed\n");
      exit(1);
    }

  /*  
#ifndef NDEBUG
  printf("evals = ");
  for(int i=0;i<_rows;i++)
    printf("%f ",eigsfull[i]);
  printf("\n");

  printf("testing eigenmatrix from dsyev: diff = \n");
  for(int i=0;i<_rows;i++)
    {
      CVector<double> v = evecsfull.getColumn(i);
      CVector<double> diff = (*this)*v - v*eigsfull[i];

      printf("%f ",diff.L2());
    }
  printf("\n");
#endif
  */

  delete [] work;

  // copy the specified eigenvectors from the full eigenmatrix 

  assert(minVal >= 1 && maxVal <= _rows);

  if (minVal == 1 && maxVal == _rows)
    {
      evecs = evecsfull;
      return CVector<double>(_rows,eigsfull);
    }
  else
    {
      double * eigs2 = new double[evecs.columns()];
      for(int j=0;j<evecs.columns();j++)
	{
	  int srccol = j+minVal-1;
	  for(int i=0;i<_rows;i++)
	    evecs.get(i,j) = evecsfull.get(i,srccol);

	  eigs2[j] = eigsfull[srccol];
	}
      return CVector<double>(evecs.columns(),eigs2);
    }
}

inline void
linearRegression(const vector<Neigh*> & inputData,
		 const vector<CVector<double>*> & targetData,
		 CMatrix<double> & A,
		 CVector<double> & B)
{
  assert(inputData.size() == targetData.size());
 
  int npts = inputData.size();
  int indim = inputData.front()->Length();
  int tardim = targetData.front()->Length();
  int nlhs = indim + 1;
  
  int ndat = max(max(npts,nlhs),tardim);
  int lwork = ndat*ndat;

  assert(A.columns() == indim);
  assert(A.rows() == tardim);
  assert(B.Length() == tardim);

  int nsingvals = min(indim,tardim);

  double * singvals = new double[nsingvals];
  double * work = new double[lwork];
  if (singvals == NULL || work == NULL)
    {
      printf("error");
      exit(1);
    }

  CMatrix<double> lhs(ndat,nlhs);
  CMatrix<double> rhs(ndat,tardim);

  // pad system of equations with 0's, up to ndat
  lhs.clear();  rhs.clear();

  int i,j;

  for(i=0;i<npts;i++)
    {
      for(j=0;j<indim;j++)
	lhs.get(i,j) = (*inputData[i])[j];
      lhs.get(i,indim) = 1;

      for(j=0;j<tardim;j++)
	rhs.get(i,j) = (*targetData[i])[j];
    }

  double rcond;
  int rank;
  int info;

  DGELSS(&ndat,&nlhs,&tardim,lhs.data(),&ndat,rhs.data(),&ndat,
  	 singvals,&rcond,&rank,work,&lwork,&info);

  if (info != 0)
    {
      printf("DGELSS failed with info = %d\n",info);
      exit(1);
    }
  
  delete [] singvals;
  delete [] work;

  for(i=0;i<A.rows();i++)
    for(j=0;j<A.columns();j++)
      A.get(i,j) = rhs.get(j,i);

  for(i=0;i<B.Length();i++)
    B[i] = rhs.get(A.columns(),i);
}
  /* 
  // compute means and scatter matrices
 CVector<double> inputMean(indim);
  CVector<double> targetMean(tardim);
  CMatrix<double> inputScatter(indim,indim);
  CMatrix<double> targetScatter(tardim,tardim);
  CMatrix<double> jointScatter(tardim,indim);
  inputMean.clear(); targetMean.clear(); inputScatter.clear();
  targetScatter.clear(); jointScatter.clear();

  vector<Neigh*> iterator idit;
  vector<CVector<double>*> iterator tdit;
  int i,j;
  
  for(idit = inputData.begin(); idit != inputData.end(); idit ++)
    inputMean += *idit;
  inputMean /= inputData.size();

  for(tdit = targetData.begin(); idit != targetData.end(); idit ++)
    targetMean += *tdit;
  targetMean /= targetData.size();

  for(idit = inputData.begin(), tdit = targetData.begin();
      idit != inputData.end();
      idit ++, tdit ++)
    {
      CVector<double> 

      for(int i=0;i<indim;i++)
	for(int j=0;j<indim;j++)
	  inputScatter->get(i,j) = (*
}
*/
#else
inline void
linearRegression(const vector<Neigh*> & inputData,
		 const vector<CVector<double>*> & targetData,
		 CMatrix<double> & A,
		 CVector<double> & B)
{
  printf( "Linear regression not supported, bailing...\n" );
  exit( 1 );
}

#endif  // LAPACK



template <class T>
CMatrix<T> 
CMatrix<T>::outer(const CVector<T> & v1,const CVector<T> & v2)
{
  CMatrix<T> result(v1.Length(),v2.Length());
  for(int i=0;i<v1.Length();i++)
    for(int j=0;j<v2.Length();j++)
      result.get(i,j) = v1[i]*v2[j];

  return result;
}

template<class T>
T CMatrix<T>::sumsqr() const
{
  T result = _data[0]*_data[0];
  for(int i=1;i<_rows*_columns;i++)
    result += _data[i]*_data[i];

  return result;
}

#ifdef LAPACK
inline CMatrix<double> & CMatrix<double>::operator+=(const CMatrix & v)
{
  assert(_rows == v._rows && _columns == v._columns);
  double alpha = 1;
  int one = 1;
  int size = _rows*_columns;
  DAXPY(&size,&alpha,v._data,&one,_data,&one);
  return *this;
}
#endif

template<class T>
CMatrix<T> & CMatrix<T>::operator+=(const CMatrix & v) 
{
  assert(_rows == v._rows && _columns == v._columns);
  for(int i=0;i<_rows*_columns;i++)
    _data[i] += v._data[i];
  return *this;
}






template<class T>
void CMatrix<T>::printFloat(FILE * fp)
{
  for(int i=0;i<_rows;i++)
    {
      for(int j=0;j<_columns;j++)
	fprintf(fp,"%.8f ",get(i,j));
      printf("\n");
    }
		
}


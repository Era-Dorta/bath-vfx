#include "my_lapack.h"

static const int i_one = 1;

template<class T>
CVector<T>::CVector() : _length(0),_data(new T[_length]), _ownsData(true)
{
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    } 
}

template<class T>
CVector<T>::CVector(int len) : 
  _length(len),_data(new T[_length]),_ownsData(true)
{
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }
}

template<class T>
CVector<T>::CVector(const CVector & v) 
{ 
  _length = v._length; 
  _data = new T[_length]; 
  _ownsData = true;
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1); 
    } 
  memcpy(_data,v._data,sizeof(T)*_length);
}

template<class T>
T & CVector<T>::operator[](const int index) 
{
  assert(index>=0 && index<_length); 
  return _data[index]; 
}
template<class T>
T CVector<T>::operator[](const int index) const 
{ 
  assert(index>=0 && index<_length);
  return _data[index];
}
  
template<class T>
CVector<T> & CVector<T>::operator=(const CVector & v) 
{ 
  if( _length != v._length ) 
  {
    _length = v._length;
    _data = new T[_length];
    if (_data == NULL) 
      {
	printf("error\n");
	exit(1);
      } 
  }
					   
  memcpy( _data, v._data, sizeof(T)*_length );
  return *this;
}

#ifdef LAPACK
inline CVector<double> & CVector<double>::operator+=(const CVector & v)
{
  assert(_length == v._length);
  double alpha = 1;
  int one = 1;
  DAXPY(&_length,&alpha,v._data,&one,_data,&one);
  return *this;
}

inline CVector<double> & CVector<double>::operator-=(const CVector & v)
{
  assert(_length == v._length);
  double alpha = -1;
  int one = 1;
  DAXPY(&_length,&alpha,v._data,&one,_data,&one);
  return *this;
}
#endif

template<class T>
CVector<T> & CVector<T>::operator+=(const CVector & v) 
{
  assert(_length == v._length);
  for(int i=0;i<_length;i++)
    _data[i] += v._data[i];
  return *this;

}

template<class T>
CVector<T> & CVector<T>::operator-=(const CVector & v) 
{
  assert(_length == v._length);
  for(int i=0;i<_length;i++) 
  _data[i] -= v._data[i]; 
  return *this;
}

template<class T>
CVector<T> & CVector<T>::operator/=(const CVector & v) 
{
  assert(_length == v._length);
  for(int i=0;i<_length;i++)
  _data[i] /= v._data[i]; 
  return *this;
}
template<class T>
CVector<T> & CVector<T>::operator/=(const T & val) 
{ 
  for(int i=0;i<_length;i++)
  _data[i] /= val;
  return *this;
}

template<class T>
CVector<T>  CVector<T>::operator+(const CVector & v) const
{
  assert(_length == v._length);
  T * data=new T[_length];
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    } 
  for(int i=0;i<_length;i++) 
    data[i] = _data[i]+v._data[i];
  return CVector(_length,data);
}
template<class T>
CVector<T>  CVector<T>::operator-(const CVector & v) const 
{ 
  assert(_length == v._length);
  T * data=new T[_length];
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }
  for(int i=0;i<_length;i++) 
    data[i] = _data[i]-v._data[i];
  return CVector(_length,data);
}
template<class T>
CVector<T>  CVector<T>::operator+(const T & v) const 
{ 
  T * data=new T[_length];
  if (_data == NULL)
    {
      printf("error\n");
      exit(1);
    }
  for(int i=0;i<_length;i++) 
    data[i] = _data[i]+v;
  return CVector(_length,data);
}
template<class T>
CVector<T> CVector<T>::operator-(const T & v) const
{ 
  T * data=new T[_length];
  if (_data == NULL)
    {
      printf("error\n");
      exit(1);
    }
  for(int i=0;i<_length;i++) 
    data[i] = _data[i]-v;
  return CVector(_length,data); 
}

template<class T>
CVector<T>  CVector<T>::operator*(const T & v) const 
{ 
  T * data=new T[_length];
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }
  for(int i=0;i<_length;i++)
    data[i] = _data[i]*v;
  return CVector(_length,data); 
}

template<class T>
CVector<T> CVector<T>::mult(const CVector & v) const
{
  assert(_length == v._length);
  T * data=new T[_length];
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }
  for(int i=0;i<_length;i++)
    data[i] = _data[i]*v._data[i];
  return CVector(_length,data); 
}

template<class T>
CVector<T>  CVector<T>::operator/(const T & v) const
{ 
  T * data=new T[_length];
  if (_data == NULL) 
    {
      printf("error\n");
      exit(1);
    }

  for(int i=0;i<_length;i++)
    data[i] = _data[i]/v;

  return CVector(_length,data);
}

/*
#ifdef LAPACK
inline double CVector<double>::dot(const CVector & v) const
{
  assert(_length == v._length);
  int one=1;
  return DDOT(&_length,_data,&one,v._data,&one);
}
#endif
*/

template<class T>
T CVector<T>::dot(const CVector & v) const 
{ 
  assert(_length == v._length);
  T sum = 0; 
  for(int i=0;i<_length;i++)
    sum += _data[i]*v._data[i];
  return sum; 
}
template<class T>
bool CVector<T>::operator==(const CVector & v) const 
{
  assert(_length == v._length); 
  for(int i=0;i<_length;i++)    
  if (_data[i] != v._data[i])   
  return false; 
  return true;
}
template<class T>
bool CVector<T>::operator!=(const CVector & v) const
{
  return ! (*this == v);
}

/*
#ifdef LAPACK
inline double CVector<double>::dist(const CVector & v) const
{
  CVector vec = *this - v; 
  int one = 1;
  return DNRM2(&_length,vec._data,&one);
}
#endif
*/

template<class T>
double CVector<T>::dist2(const CVector & v) const
{
  CVector vec = *this - v; 
  return vec.dot(vec);
}

template<class T>
double CVector<T>::dist(const CVector & v) const
{
  return sqrt(dist2(v));
}

template<class T>
double CVector<T>::maxAbsEl() 
{
  double v = (*this)[0]; 
  if (v < 0) v = -v;
  for(int i=1;i<Length(); i++) 
    if ((*this)[i] > v)
      v = (*this)[i];
    else
      if (-(*this)[i] > v)
	v = -(*this)[i];
  return v;
}

template<class T>
void CVector<T>::printFloat(FILE * fp)
{
  for(int i=0;i<_length;i++)
    fprintf(fp,"%.8f ",_data[i]);
}
      
#ifdef LAPACK
inline double CVector<double>::L2() const
{
  int one = 1;
  return DNRM2(&_length,_data,&one);
}
#endif

template<class T> 
double CVector<T>::L2() const { return sqrt(dot(*this)); }

template<class T> 
T CVector<T>::sum() const 
{
  double r = _data[0];
  for(int i=1;i<_length;i++)
    r += _data[i];
  return r;
}

template<class T> 
T CVector<T>::L2sqr() const { return dot(*this); }

inline CVector<double> CVector<double>::sigmoid() const 
{
  double * data = new double[_length];
  if (data == NULL)
    {
      printf("error\n");
      exit(1);
    }

  for(int i=0;i<_length;i++)
    data[i] = 1/(1+exp(-_data[i]));

  return CVector<double>(_length,data);
}


inline CVector<double> CVector<double>::tanh() const 
{
  double * data = new double[_length];
  if (data == NULL)
    {
      printf("error\n");
      exit(1);
    }

  for(int i=0;i<_length;i++)
    {
      double e1 = exp(_data[i]);
      double e2 = exp(-_data[i]);

      data[i] = (e1-e2)/(e1+e2);
    }

  return CVector<double>(_length,data);
}



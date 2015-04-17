#ifndef __CVECTOR_H__
#define __CVECTOR_H__

#include <stdio.h>

#include "cvec2t.h"
#include "compat.h"

#include "ANN/ANN.h"

template<class T>
class CMatrix;

class MLP;

template<class T>
class CVector
{
 private:
  int _length;
  T * _data;
  bool _ownsData;
    
 public:
  CVector();
  CVector(int len);
  CVector(const CVector & v);
  virtual ~CVector() { if (_ownsData) delete [] _data; }
  T & operator[](const int index);
  T operator[](const int index) const ;
  int Length() const { return _length; } 
  void clear() { memset(_data,0,_length*sizeof(T)); }
  CVector & operator=(const CVector & v) ;
  CVector & operator+=(const CVector & v) ;
  CVector & operator-=(const CVector & v) ;
  CVector & operator/=(const CVector & v) ;
  CVector & operator/=(const T & val) ;
  CVector operator+(const CVector & v) const;
  CVector operator-(const CVector & v) const ;
  CVector operator+(const T & v) const ;
  CVector operator-(const T & v) const;
  CVector operator*(const T & v) const;
  CVector operator/(const T & v) const;
  CVector mult(const CVector & v) const;
  T dot(const CVector & v) const ;
  bool operator==(const CVector & v) const ;
  bool operator!=(const CVector & v) const;
  double L2() const;
  T L2sqr() const;
  double dist(const CVector & v) const;
  double dist2(const CVector & v) const;
  double maxAbsEl() ;
  CVector sigmoid() const;
  CVector tanh() const;
  T sum() const;

  void copy(T * data) { memcpy(_data,data,_length*sizeof(T)); }

  void printFloat(FILE * fp = stdout);

 private:
  friend CMatrix<T>;
  friend MLP;
  CVector(int len,T * data,bool ownsData=true) : 
    _length(len),_data(data),_ownsData(ownsData) { };
};



// typedef CVec2T<int> Point2;

// later, Point2 will go back to being a CVec2T<int>, and this will change to an ImageLocation or something:
struct Point2
{
public:
  int imageID;
  CVec2T<int> location;
  
  Point2( void ) : location(-1,-1), imageID(-1) {}
  Point2( const Point2 &p ) : location(p.location), imageID( p.imageID ) {}
  Point2( int ix, int iy ) : location( ix, iy ), imageID( -1 ) {}
  Point2( int ix, int iy, int imageID ) : location( ix, iy ), imageID( imageID ) {}
  bool operator==(const Point2 & p) { return imageID == p.imageID &&
					location == p.location; }
  bool operator!=(const Point2 & p) { return !(*this == p); }
  bool operator<(const Point2 & p ) const { return imageID < p.imageID ||
					(imageID == p.imageID &&
					 location < p.location); }
  bool operator>(const Point2 & p ) const { return imageID > p.imageID ||
					(imageID == p.imageID &&
					 location > p.location); }


  Point2 &operator=( const Point2 &p )
  {
    location = p.location;
    imageID = p.imageID;
    return *this;
  }

  int x( void ) { return location.x(); }
  int y( void ) { return location.y(); }

  // cheesy emulation of CVec2T:
  operator int*( void )
    { return &location[0]; }
  operator const int*( void ) const
  { return &location[0]; }

  int ImageID( void ) const { return imageID; }
  int & ImageID() { return imageID; }
};

  
struct Neigh : public CVector<double>
{
  Point2 loc;
  bool constSrc;
  enum { MAXDIM = 4 };
  int srcPixelDim;
  double constSrcColor[MAXDIM];
  
  Neigh() {}
  Neigh(int length, Point2 p) : CVector<double>(length), loc(p), constSrc(false), srcPixelDim(0) {}
  Neigh(const CVector<double> & v) : CVector<double>(v), loc(-1, -1, -1), constSrc(false), srcPixelDim(0) {}
  Neigh(const Neigh & n) : CVector<double>(n), loc(n.loc), constSrc(n.constSrc), srcPixelDim( n.srcPixelDim )
  {
    if( n.constSrc )
      for( int index = 0; index < n.srcPixelDim; index++ )
	constSrcColor[index] = n.constSrcColor[index];
  }
  
  Neigh & operator=(const CVector<double> & v)
  {
    loc = Point2( -1, -1, -1 );
    constSrc = false;
    srcPixelDim = 0;
    
    *((CVector<double>*)this) = v;
    return *this;
  }
  
  Neigh & operator=(const Neigh & n)
  {
    loc = n.loc;
    constSrc = n.constSrc;
    srcPixelDim = n.srcPixelDim;
    if( constSrc )
      for( int index = 0; index < n.srcPixelDim; index++ )
        constSrcColor[index] = n.constSrcColor[index];
    
    *((CVector<double>*)this) = n;
    return *this;
  }
};


#include "CVectorInline.h"


#endif

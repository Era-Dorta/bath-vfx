#ifndef __CMATRIX_H__
#define __CMATRIX_H__

#include "CVector.h"

class MLP;

void linearRegression(const vector<Neigh*> & inputData,
		      const vector<CVector<double>*> & targetData,
		      CMatrix<double> & A,
		      CVector<double> & B);

template <class T>
class CMatrix
{
 private:
  int _rows;
  int _columns;
  T * _data;  // data stored in row-major order for library compatibility
  bool _ownsData;

 public:
  CMatrix();
  CMatrix(int rows,int columns);
  CMatrix(const CMatrix & v);
  ~CMatrix() { if (_ownsData) delete [] _data; }

  T & get(int i,int j);
  T get(int i,int j) const;
  void clear() { memset(_data,0,_rows*_columns*sizeof(T)); }
  CMatrix<T> & operator=(const CMatrix<T> & v);
  CMatrix<T> & operator+=(const CMatrix<T> & v);
  CVector<T> operator*(const CVector<T> & v) const;
  CMatrix<T> operator*(const CMatrix<T> & v) const;
  CVector<T> symmetricEigenvalues(); 
  CVector<T> symmetricEigenvectors(CMatrix & evecs,int maxVec,int minVec); 
  CMatrix<T> transpose() const;
  CVector<T> getRow(int i) const;
  CVector<T> getColumn(int j) const;

  T * data() { return _data; }

  T sumsqr() const;

  void copy(T * data) { memcpy(_data,data,_rows*_columns*sizeof(T)); }

  void printFloat(FILE * fp = stdout);

  int rows() const { return _rows; }
  int columns() const { return _columns; }

  static CMatrix<T> outer(const CVector<T> & v1,const CVector<T> & v2);

 private:
  friend MLP;
  CMatrix(int rows, int columns, T * data, bool ownsData = true);
};


#include "CMatrixInline.h"


#endif

#ifndef __TSVQR_H__
#define __TSVQR_H__

#include <math.h>
#include <assert.h>
#include <vector>

#include "CMatrix.h"

#include "ANN/ANN.h"

using namespace std;

class TSVQR
{
 public:
  TSVQR(const vector<Neigh*> & inputData, 
  	const vector<CVector<double>*> & targetData,
	int maxDepth = 20, Neigh *cent1=NULL);
  
  CVector<double> apply(const CVector<double> & pt,
			unsigned int & num_comparisons,
			int numBacktracks = 8) const;
  Neigh findNearest(const CVector<double> & pt,
			unsigned int & num_comparisons,
			int numBacktracks = 8) const;
  const TSVQR * findNearestHelper(const CVector<double> & pt,
			    unsigned int & num_comparisons,
			    int & numBacktracks) const;
  void stats(int & depth, int & numLeaves);
  ~TSVQR();

  // private:

  bool m_leafNode;
  TSVQR * m_posBranch;
  TSVQR * m_negBranch;

  Neigh m_point;
  Neigh m_center1, m_center2;
  CMatrix<double> * m_A;
  CVector<double> * m_b;
};


#endif

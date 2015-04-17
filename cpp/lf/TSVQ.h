#ifndef __TSVQ_H__
#define __TSVQ_H__

#include <math.h>
#include <assert.h>
#include <vector>

#include "CMatrix.h"
#include "sampler.h"

using namespace std;

template <class PointN>
class TSVQ
{
 public:
  TSVQ(const vector<PointN*> & points, float maxError = 0,
       int maxDepth = 20, PointN *cent1=NULL);
  const PointN & findNearest(const PointN & pt,unsigned int & numComparisons);
  const PointN &findNearest(const PointN & pt,unsigned int & numComparsions,
			    int numBacktracks = 8);
  const PointN &findNearestHelper(const PointN & pt, unsigned int & numComparisons,
				  int &numBacktracks);
  const void sample(const PointN &pt, Sampler<Point2> & samp,
		    unsigned int & numComparisons,
		    int numBacktracks = 8);
  const void sampleHelper(const PointN &pt, Sampler<Point2> & samp,
			  unsigned int & numComparisons,int & numBacktracks);

  static PointN centroid(const vector<PointN*> & pts);
  static PointN sqr(const PointN & p);
  static PointN psqrt(const PointN & p);
  static PointN stdev(const vector<PointN*> & pts, const PointN & mean);
  void stats(int & depth, int & numLeaves);
  ~TSVQ();

  // private:
  bool m_leafNode;
  PointN m_point;
  PointN m_center1, m_center2;
  TSVQ * m_posBranch;
  TSVQ * m_negBranch;
};


#include "TSVQInline.h"
 

#endif

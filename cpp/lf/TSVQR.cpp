#include "TSVQ.h"
#include "TSVQR.h"
#include "CMatrix.h"

TSVQR::TSVQR(const vector<Neigh*> & inputData, 
	     const vector<CVector<double>*> & targetData,
	      int maxDepth, Neigh *cent1) 
{
  Neigh cent = cent1==NULL?TSVQ<Neigh>::centroid(inputData):*cent1;

  // check for leaf node conditions
  m_leafNode = (maxDepth <= 0 || inputData.size() == 1);
  m_posBranch = NULL;
  m_negBranch = NULL;

  assert(inputData.size() > 0);

  Neigh vec;
  double maxel;

  if (!m_leafNode)
    {
      vec = TSVQ<Neigh>::stdev(inputData,cent);
      maxel = vec.maxAbsEl();
      if (maxel > 1e-8*inputData.size())
	vec = vec * (1e-8/maxel);
      else
	m_leafNode = true;
    }

  if (m_leafNode)
    // create the leaf node
    {
      // pick a representative at random

      // we really should pick the representative closest to the centroid
      m_point = **inputData.begin();

      int indim = (*inputData.begin())->Length();
      int tardim = (*targetData.begin())->Length();

      m_A = new CMatrix<double>(tardim,indim);
      m_b = new CVector<double>(tardim);

      if (m_A == NULL || m_b == NULL)
	{
	  printf("can't allocate matrix\n");
	  exit(1);
	}
	
      m_A->clear();

      if (inputData.size() == 1)
	*m_b = **targetData.begin();
      else
	linearRegression(inputData,targetData,*m_A,*m_b);
      
      assert(m_A->rows() == tardim && m_A->columns() == indim);

      return;
    }

  m_center1 = cent - vec;
  m_center2 = cent + vec;
		
  assert(m_center1 != m_center2);

  Neigh oldcenter1, oldcenter2;
		
  int npts1, npts2;

  int n = 0; // for debugging
  do
    {
      n++;
      oldcenter1 = m_center1;
      oldcenter2 = m_center2;

      npts1 = 0;
      npts2 = 0;

      for(vector<Neigh*>::const_iterator it = inputData.begin(); 
	  it != inputData.end(); ++it)
	{
	  if ( oldcenter1.dist(**it) < oldcenter2.dist(**it) )
	    {
	      if (npts1 == 0)
		m_center1 = **it;
	      else
		m_center1 += **it;
	      npts1 ++;
	    }
	  else
	    {
	      if (npts2 == 0)
		m_center2 = **it;
	      else
		m_center2 += **it;
	      npts2 ++;
	    }
	}

      // check for unstable initialization
      if (npts1 == 0)
	{
	  assert(n == 1);
	  m_center1 = *inputData[0];  npts1 =1;
	  m_center2 -= *inputData[0];  npts2 --;
	}
      else
	if (npts2 == 0)
	  {
	    assert(n == 1);
	    m_center2 = *inputData[0]; npts2 =1;
	    m_center1 -= *inputData[0]; npts1 --;
	  }

      m_center1 /= npts1;
      m_center2 /= npts2;
    }
  while (m_center1 != oldcenter1);

  assert(m_center2.dist(oldcenter2) < 1e-8 || n == 1);
  
  vector<Neigh*> inpts1, inpts2;
  vector<CVector<double>*> tarpts1, tarpts2;

  vector<Neigh*>::const_iterator it;
  vector<CVector<double>*>::const_iterator tdit;
  for(it = inputData.begin(), tdit = targetData.begin();
      it != inputData.end(); ++it, ++tdit)
    if ( m_center1.dist(**it) < m_center2.dist(**it) )
      {
	inpts1.push_back(*it); 
	tarpts1.push_back(*tdit);
      }
    else
      {
	inpts2.push_back(*it);
	tarpts2.push_back(*tdit);
      }

  assert(tdit == targetData.end());

  assert(inpts1.size() == npts1 && tarpts1.size() == npts1);
  assert(inpts2.size() == npts2 && tarpts2.size() == npts2);

  // create an internal tree node
  m_posBranch = new TSVQR(inpts1,tarpts1,maxDepth-1,&m_center1);
  m_negBranch = new TSVQR(inpts2,tarpts2,maxDepth-1,&m_center2);
}

CVector<double> TSVQR::
apply(const CVector<double> & pt, unsigned int & numComparisons, 
	    int numBacktracks) const
{
  const TSVQR * nearestLeaf = findNearestHelper( pt, numComparisons, 
						 numBacktracks );
  //  return nearestLeaf->m_point;
  return (*nearestLeaf->m_A)*pt + *nearestLeaf->m_b;
  //  return nearestLeaf->m_point.loc;
}

Neigh TSVQR::
findNearest(const CVector<double> & pt, unsigned int & numComparisons, 
	    int numBacktracks) const
{
  const TSVQR * nearestLeaf = findNearestHelper( pt, numComparisons, 
						 numBacktracks );
  //  return (*nearestLeaf->m_A)*pt + *nearestLeaf->m_b;
  return nearestLeaf->m_point;
}


const TSVQR * TSVQR::
findNearestHelper(const CVector<double> & pt, unsigned int &numComparisons, 
		  int &numBacktracks) const
{
  if (m_leafNode)
    {
      numBacktracks--;
      return this;
    }

  TSVQR *nearChild;
  TSVQR *farChild;

  assert(m_posBranch != NULL);
  assert(m_negBranch != NULL);

  numComparisons += 2;

  if ( m_center1.dist(pt) < m_center2.dist(pt) )
    {
      nearChild = m_posBranch;
      farChild = m_negBranch;
    }
  else
    {
      nearChild = m_negBranch;
      farChild = m_posBranch;
    }

  const TSVQR * result = nearChild->findNearestHelper( pt, numComparisons, 
						       numBacktracks );
  if( numBacktracks > 0 )
    {
      const TSVQR * newResult = 
	farChild->findNearestHelper( pt, numComparisons, numBacktracks );

      numComparisons += 2;

      if( newResult->m_point.dist(pt) < result->m_point.dist(pt) )
	return newResult;
    }

  return result;
}

void TSVQR::stats(int & depth, int & numLeaves)
{
  if (m_leafNode)
    {
      depth = 0;
      numLeaves = 1;
    }
  else
    {
      m_posBranch->stats(depth,numLeaves);
      int d2, nl;
      m_negBranch->stats(d2,nl);
      if (d2 > depth) depth = d2+1; else depth = depth+1;
      numLeaves = numLeaves + nl;
    }
}


TSVQR::~TSVQR() 
{ 
  if (m_posBranch != NULL) delete m_posBranch;
  if (m_negBranch != NULL) delete m_negBranch;
}



 

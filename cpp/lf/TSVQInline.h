
template <class PointN>
TSVQ<PointN>::TSVQ(const vector<PointN*> & points, float maxError,
		   int maxDepth, PointN *cent1) 
{
  assert(points.size() > 0);
  const PointN &cent = (cent1==NULL) ? centroid(points) : *cent1;

  // check for leaf node conditions
  m_leafNode = (maxDepth <= 0 || points.size() == 1);
  m_posBranch = NULL;
  m_negBranch = NULL;

  if ( !m_leafNode)
    {
      m_leafNode = true;
      for(vector<PointN*>::const_iterator it = points.begin(); it != points.end(); ++ it)
	if ( cent.dist(*(*it)) > maxError)
	  {
	    m_leafNode = false;
	    break;
	  }
    }

  PointN vec;

  if (!m_leafNode)
    {
      vec = stdev(points,cent);
      double maxel = vec.maxAbsEl();

      if (maxel > 1e-8*points.size())
	vec = vec * (1e-8/maxel);
      else
	m_leafNode = true;
    }

  if (m_leafNode)
    // create the leaf node
    {
      // pick the representative closest to the centroid
      int best=0;
      if (points.size() > 2)
	{
	  double bestDist = cent.dist2(**points.begin());
	  for(int i=1;i<points.size();i++)
	    {
	      double d2 = cent.dist2(*points[i]);
	      if (d2 < bestDist)
		{
		  best=i;
		  bestDist = d2;
		}
	    }
	}
      m_point = *points[best];
      
      return;
    }

  // debugging
  m_center1 = cent - vec;
  m_center2 = cent + vec;
		
  //PointN oldcenter1, oldcenter2;
		
  assert(m_center1 != m_center2);
  //      assert(!m_center1.isnan());
  //      assert(!m_center2.isnan());

  PointN oldcenter1, oldcenter2;
		
  int npts1, npts2;

  int n = 0; // for debugging
  do
    {
      n++;
      oldcenter1 = m_center1;
      oldcenter2 = m_center2;

      npts1 = 0;
      npts2 = 0;

      for(vector<PointN*>::const_iterator it = points.begin(); 
	  it != points.end(); ++it)
	{
	  if ( oldcenter1.dist2(**it) < oldcenter2.dist2(**it) )
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
	  m_center1 = *points[0];  npts1 =1;
	  m_center2 -= *points[0];  npts2 --;
	}
      else
	if (npts2 == 0)
	  {
	    assert(n == 1);
	    m_center2 = *points[0]; npts2 =1;
	    m_center1 -= *points[0]; npts1 --;
	  }

      m_center1 /= npts1;
      m_center2 /= npts2;
    }
  while (m_center1 != oldcenter1);

  assert(m_center2.dist(oldcenter2) < 1e-8 || n == 1);
  
  vector<PointN*> pts1, pts2;
  for(vector<PointN*>::const_iterator it = points.begin(); 
      it != points.end(); ++it)
    if ( m_center1.dist2(**it) < m_center2.dist2(**it) )
      pts1.push_back(*it); 
    else
      pts2.push_back(*it);

  // create an internal tree node
  m_posBranch = new TSVQ(pts1,maxError,maxDepth-1,&m_center1);
  m_negBranch = new TSVQ(pts2,maxError,maxDepth-1,&m_center2);
}

template <class PointN>
const PointN & TSVQ<PointN>::findNearest(const PointN & pt,
					 unsigned int & numComparisons)
{
  if (m_leafNode)
    return m_point;

  numComparisons += 2;

  if ( m_center1.dist2(pt) < m_center2.dist2(pt) )
    {
      assert(m_posBranch != NULL);
      return m_posBranch->findNearest(pt,numComparisons);
    }
  else
    {
      assert(m_negBranch != NULL);
      return m_negBranch->findNearest(pt,numComparisons);
    }
}

template <class PointN>
const PointN &TSVQ<PointN>::
findNearest(const PointN & pt, unsigned int & numComparisons, 
	    int numBacktracks)
{
  return findNearestHelper( pt, numComparisons, numBacktracks );
}

template <class PointN>
const PointN &TSVQ<PointN>::
findNearestHelper(const PointN & pt, unsigned int &numComparisons, 
		  int &numBacktracks)
{
  if (m_leafNode)
    {
      numBacktracks--;
      return m_point;
    }

  TSVQ *nearChild;
  TSVQ *farChild;

  assert(m_posBranch != NULL);
  assert(m_negBranch != NULL);

  numComparisons += 2;

  if ( m_center1.dist2(pt) < m_center2.dist2(pt) )
    {
      nearChild = m_posBranch;
      farChild = m_negBranch;
    }
  else
    {
      nearChild = m_negBranch;
      farChild = m_posBranch;
    }

  const PointN &result = nearChild->findNearestHelper( pt, numComparisons, 
						       numBacktracks );
  if( numBacktracks > 0 )
    {
      const PointN &newResult = farChild->findNearestHelper(pt,numComparisons,
							    numBacktracks );

      numComparisons += 2;

      if( newResult.dist2(pt) < result.dist2(pt) )
	return newResult;
    }

  return result;
}

template <class PointN>
PointN TSVQ<PointN>::centroid(const vector<PointN*> & pts)
{
  assert(!pts.empty());
  vector<PointN*>::const_iterator it = pts.begin();
  PointN c = (**it);
  /*
  return c;
  */

  for(++it; it != pts.end(); ++it)
    c += (**it);
  return c / pts.size();
}

template <class PointN>
PointN TSVQ<PointN>::sqr(const PointN & p)
{
  PointN r = p;
  for(int i=0;i<r.Length();i++)
    r[i] = p[i]*p[i];
  return r;
}

template <class PointN>
PointN TSVQ<PointN>::psqrt(const PointN & p)
{
  PointN r = p;
  for(int i=0;i<r.Length();i++)
    r[i] = sqrt(p[i]);
  return r;
}

template <class PointN>
PointN TSVQ<PointN>::stdev(const vector<PointN*> & pts, const PointN & mean)
{
  assert(!pts.empty());
  vector<PointN*>::const_iterator it = pts.begin();
  PointN v = sqr(**it - mean);
  for(++it; it != pts.end(); ++it)
    v = v + sqr(**it - mean);
  return psqrt(v);
}

template <class PointN>
void TSVQ<PointN>::stats(int & depth, int & numLeaves)
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

template <class PointN>
TSVQ<PointN>::~TSVQ() 
{ 
  if (m_posBranch != NULL) delete m_posBranch;
  if (m_negBranch != NULL) delete m_negBranch;
	  
}

template <class PointN>
const void TSVQ<PointN>::
sample(const PointN &pt, Sampler<Point2> & samp,
		  unsigned int & numComparisons,int numBacktracks)
{
  sampleHelper(pt,samp,numComparisons,numBacktracks);
}

template <class PointN>
const void TSVQ<PointN>::
sampleHelper(const PointN &pt, Sampler<Point2> & samp,
	     unsigned int & numComparisons,int & numBacktracks)
{
  if (m_leafNode)
    {
      numBacktracks--;

      float dist = pt.dist(m_point);

      numComparisons ++;

      samp.addElement(dist,m_point.loc);

      return;
    }

  TSVQ *nearChild;
  TSVQ *farChild;

  assert(m_posBranch != NULL);
  assert(m_negBranch != NULL);

  numComparisons += 2;

  if ( m_center1.dist2(pt) < m_center2.dist2(pt) )
    {
      nearChild = m_posBranch;
      farChild = m_negBranch;
    }
  else
    {
      nearChild = m_negBranch;
      farChild = m_posBranch;
    }

  nearChild->sampleHelper(pt,samp,numComparisons,numBacktracks);

  if( numBacktracks > 0 )
    farChild->sampleHelper(pt,samp,numComparisons,numBacktracks);
}


 


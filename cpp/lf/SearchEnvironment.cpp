#include "SearchEnvironment.h"
#include "sampler.h"

/////////////////////////////////////////
// ANN stuff
/////////////////////////////////////////

CANNSearchEnvironment::CANNSearchEnvironment( vector<Neigh*> &neighborhoods, 
					      Sampler<Point2> * sampler ) : 
  CSearchEnvironment( sampler )
{
  printf( "Building ANN tree..." );
  
  m_numPoints = neighborhoods.size();
  m_annDim = neighborhoods[0]->Length();  // TODO: get this
  m_annPoints = annAllocPts( m_numPoints, m_annDim );
  m_annQueryPoint = annAllocPt( m_annDim );
  m_neighborhoodLocations.erase( m_neighborhoodLocations.begin(), m_neighborhoodLocations.end() );

  for( int index = 0; index < m_numPoints; index++ )
    {
      Neigh &neigh = *(neighborhoods[ index ]);
      for( int index2 = 0; index2 < m_annDim; index2++ )
	(m_annPoints[index])[index2] = neigh[index2];

      m_neighborhoodLocations.push_back( neigh.loc );
    }
  
  m_annTree = new ANNkd_tree( m_annPoints, m_numPoints, m_annDim );
  //  m_annTree = new ANNbd_tree( m_annPoints, m_numPoints, m_annDim );
  assert( m_annTree != NULL );

  m_annEpsilon = 0.0f;

  printf( " Done\n" );
}

CANNSearchEnvironment::~CANNSearchEnvironment( void )
{
  printf( "Destroying ANN tree\n" );

  // destroy allocated stuff
  delete m_annTree;
  annDeallocPts( m_annPoints );
  annDeallocPt( m_annQueryPoint );
}

Point2
CANNSearchEnvironment::FindBestMatchLocation( Neigh &neighborhood )
{
  assert( m_annTree );

  for( int index = 0; index < m_annDim; index++ )
    m_annQueryPoint[index] = neighborhood[index];
  
  const int MAX_SAMPLES = 100;
  int numSamples = min( m_numPoints, MAX_SAMPLES );
  ANNidx annResultIndex[MAX_SAMPLES];
  ANNdist annResultDist[MAX_SAMPLES];
  m_annTree->annkSearch( m_annQueryPoint, numSamples, annResultIndex, 
			 annResultDist, m_annEpsilon );

  if( m_sampler != NULL )
  {
    m_sampler->eraseSamples();
    for( int index = 0; index < numSamples; index++ )
    {
      m_sampler->addElement( annResultDist[index], 
			     m_neighborhoodLocations[annResultIndex[index]] );
    }
    
    if( m_sampler->size() == 0 )
      return Point2( -1, -1 );
    
    return m_sampler->UniformSample().second;
  }
  else
    return m_neighborhoodLocations[annResultIndex[0]];
}



/////////////////////////////////////////
// TSVQ stuff
/////////////////////////////////////////

CTSVQSearchEnvironment::CTSVQSearchEnvironment( vector<Neigh*> &neighborhoods,
						Sampler<Point2> *sampler,
						float maxError, int maxDepth ) :
  CSearchEnvironment( sampler )
{
  m_maxTSVQerror = maxError;
  m_maxTSVQdepth = maxDepth;

  printf("Making TSVQ tree.\n");
  m_tsvqTree = new TSVQ<Neigh>( neighborhoods, m_maxTSVQerror, m_maxTSVQdepth );
  printf( "Done making tree, computing stats...\n" );
  
  int depth, numLeaves;
  m_tsvqTree->stats( depth, numLeaves );
  printf("Done. Depth = %d, Num leaves = %d\n",depth, numLeaves);
}

CTSVQSearchEnvironment::~CTSVQSearchEnvironment( void )
{
  printf( "killing TSVQ tree\n" );
  delete m_tsvqTree;
}

/*
void
compare(TSVQ<Neigh> * tree, TSVQR *rtree)
{
  //      printf("comparing trees\n");

  if (tree->m_leafNode == rtree->m_leafNode)
    printf("m_leafnode is different\n");
  
  if (tree->m_leafNode)
    {
      double d =tree->m_point.dist2(rtree->m_point);
      if (d != 0)
	printf("mismatch d = %f\n",d);
      return;
    }
 
  double d1 = tree->m_center1.dist2(rtree->m_center1);
  double d2 = tree->m_center2.dist2(rtree->m_center2);

  if (d1!=0 || d2!=0)
    printf("mismatch d1 = %f, d2 = %f\n",d1,d2);
	     
  compare(tree->m_posBranch,rtree->m_posBranch);
  compare(tree->m_negBranch,rtree->m_negBranch);
}
*/

Point2
CTSVQSearchEnvironment::FindBestMatchLocation( Neigh &neighborhood )
{
  assert( m_tsvqTree );

  m_sampler->eraseSamples();
  m_tsvqTree->sample( neighborhood, *m_sampler, m_numComparisons, 
		      m_TSVQbacktracks );

  return m_sampler->UniformSample().second;
}

// LearnTextureDlg.cpp : implementation file
//

#include <time.h>
#include <deque>
#include <GL/glut.h>

using namespace std;

#include "sampler.h"
#include "MLP.h"
#include "SearchEnvironment.h"

#ifdef _WIN32
// inline int isnan( double x ) { return _isnan(x); }
#define isnan(x) (_isnan(x))
#endif

inline static int pow2( int exp )
{
  return 1 << exp;
}

inline static int RandInt( int numVals )
{
  double val = (double)numVals * ((double)rand() / (double)RAND_MAX);
  return (int)val % numVals;
}

/////////////////////////////////////////////////////////////////////////////
// CFilterLearner class

template < class TSource, class TFilter >
CFilterLearner<TSource,TFilter>::CFilterLearner( void )
  : m_filteredExamplePyramid(  ),
    m_filteredPyramid(  ),
    m_sourceExamplePyramid(  ),
    m_sourcePyramid(  ),
    m_sourcesPyramid(), 
    m_neighborhoodWidth(9),
    m_sourceFac(0.1),
    m_destImage(NULL),
    m_sampler(NULL),
    m_kernel(NULL),
    m_numLevels(3),
    m_levelWeighting(1),
    m_useSourceImages(false),
    m_createSrcLocHisto(false),
  m_ashikhminLastLevel(false),
  m_onePixelSource(false)
{
  srand( (unsigned int)time(NULL) );
}

template < class TSource, class TFilter >
CFilterLearner<TSource,TFilter>::~CFilterLearner()
{
  if (m_destImage != NULL)
    delete m_destImage;
}

template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
SetNeighborhood(int nw, bool useSplineWeights)
{
  if (m_kernel != NULL && nw == m_neighborhoodWidth &&
      useSplineWeights == m_useSplineWeights)
    return;

  if (m_kernel != NULL)
    delete m_kernel;

  m_kernel = new CMatrix<double>(nw+1,nw);

  if (m_kernel == NULL)
    {
      printf("can't allocate kernel\n");
      exit(1);
    }
  
  m_neighborhoodWidth = nw;
  m_useSplineWeights = useSplineWeights;

  if (!m_useSplineWeights)
    for(int width=1;width<=m_neighborhoodWidth;width+=2)
      for(int i=0;i<width;i++)
	m_kernel->get(width,i) = 1.0/width;
  else
    {
      for(int width=1;width<=m_neighborhoodWidth;width+=2)
	{
	  float denom=1 << (width-1);
	  m_kernel->get(width,0) = 1/denom;
	  for(int i=1;i<m_neighborhoodWidth;i++)
	    m_kernel->get(width,i) = m_kernel->get(width,i-1)*(width-i)/i;
	}
    }
	  
  for(int width=1;width<=m_neighborhoodWidth;width+=2)
    {
      printf("kernel[%d] = ",width);
      for(int i=0;i<width;i++)
	printf("%f ",m_kernel->get(width,i));
      printf("\n");
    }
}

template < class TSource, class TFilter >
void
CFilterLearner<TSource,TFilter>::Synthesize( void )
{
  assert( m_destMaskPyramid.NLevels() > 0 );
  int numLevels = m_filteredPyramid.NLevels();

  m_keepSearchStats = false;
  m_numSearches = 0;
  m_numTreeHits = 0;
  m_numPredHits = 0;
  m_numSearchHits = 0;
  m_numPredOnlyHits = 0;
  m_totalSearchDist = 0;

  if (m_useBias || m_useGain)
    {
      printf("Switching to vector search for bias/gain\n");
      m_searchType = VECTOR_SEARCH;
    }

  if (m_filteredBaseProcedure == COPY_SYNTH && !m_useSourceImages)
    {
      printf("Error: can't Copy source with no source; reverting to normal\n");
      m_filteredBaseProcedure = FILTER_SYNTH;
    }

  if ((m_useFilterModeMask || m_useTargetModeMask) &&
      (m_searchType == MLP_SEARCH || m_searchType == TSVQR_SEARCH))
    {
      printf("Warning: can't use mode mask with this search type; reverting\n");
      m_searchType = IMAGE_SEARCH;
    }      

  m_ignoreModeMaskPass = false;


  if (m_useFilterModeMask && !m_useTargetModeMask)
    {
      // create the mode mask for the base level
      m_pass = 0;
      m_ignoreModeMaskPass = true;
      m_useSourceImagesNow = m_useSourceImages; // (should always be true)
      SynthesizePyramidLevel(numLevels-1);
    }

  if (m_filteredBaseProcedure == COPY_SYNTH)
    {
      printf("Warning: Correct Laplacian pyramid for copying source not implemented yet\n");
      m_filteredPyramid[numLevels-1].copy(m_sourcePyramid[numLevels-1]);
      m_destMaskPyramid[ numLevels-1 ].set();
    }
  else
    {
      for(m_pass = 0;m_pass < m_numPasses;m_pass++)
	{
	  //	  printf("    *********** pass = %d ************* \n",m_pass);
	  m_useSourceImagesNow = m_useSourceImages && 
	    (m_pass == 0 || m_useSourceImagesAfterFirstPass);
	  SynthesizePyramidLevel(numLevels-1);
	  //	  printf("    ** after ** pass = %d ************* \n",m_pass);


  if (m_keepSearchStats)
    printf("total searches: %d\n\ttree hits: %f%% (%d)\n\tpred hits: %f%% (%d)\n\tpred only hits: %f%% (%d)\n\tsearch hits: %f%% (%d)\n\tave search dist: %f\n",m_numSearches,(100.0*m_numTreeHits)/m_numSearches,
	   m_numTreeHits,
	   (100.0*m_numPredHits)/m_numSearches,m_numPredHits,
	   (100.0*m_numPredOnlyHits)/m_numSearches,m_numPredOnlyHits,
	   (100.0*m_numSearchHits)/m_numSearches,m_numSearchHits,
	   m_totalSearchDist/float(m_numSearches));
  m_totalSearchDist = 0;
	}
    }  

  for( int level = numLevels-2; level >= 0; level-- )
    {
      m_numComparisons = 0;
  
      for(m_pass=0;m_pass < m_numPasses;m_pass++)
	{
	  m_useSourceImagesNow = m_useSourceImages && 
	    (m_pass == 0 || m_useSourceImagesAfterFirstPass);
	  SynthesizePyramidLevel( level );


  if (m_keepSearchStats)
    printf("total searches: %d\n\ttree hits: %f%% (%d)\n\tpred hits: %f%% (%d)\n\tpred only hits: %f%% (%d)\n\tsearch hits: %f%% (%d)\n\tave search dist: %f\n",m_numSearches,(100.0*m_numTreeHits)/m_numSearches,
	   m_numTreeHits,
	   (100.0*m_numPredHits)/m_numSearches,m_numPredHits,
	   (100.0*m_numPredOnlyHits)/m_numSearches,m_numPredOnlyHits,
	   (100.0*m_numSearchHits)/m_numSearches,m_numSearchHits,
	   m_totalSearchDist/float(m_numSearches));
  m_totalSearchDist = 0;
	}
      printf("Total comparisons for level %d: %d\n",level,m_numComparisons);
    }
}


template < class TSource, class TFilter >
void
CFilterLearner<TSource,TFilter>::
SynthesizePyramidLevel( int level )
{
  m_numPixelsSynthesized = 0;
  m_avgNumCandidates = 0;

  TFiltImage &result = m_filteredPyramid[ level ];
  Image<GLubyte> &resultMask = m_destMaskPyramid[ level ];
  Image<srcCoord> & sources = m_sourcesPyramid[level];
  
  int destWidth = m_filteredPyramid[ level ].width();

  int count = 0;

  printf("Pass number %d\n",m_pass+1);

  int maxLevel = min( level+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );
  int numLevels = maxLevel - level;

  float coherenceFac = 1+m_coherenceEps/pow(m_coherencePow,level);

  /*
  if (level < m_filteredPyramid.NLevels()-2)
    {
      for(int y=0;y<result.height();y++)
	{
	  for(int x=0;x<result.width();x++)
	    {
	      int srcx=x/2;
	      int srcy=y/2;
	      if (srcx >= m_filteredPyramid[level+1].width())
		srcx--;
	      if (srcy >= m_filteredPyramid[level+1].height())
		srcy--;
	      sources.Pixel(x,y,0) = 
		m_sourcesPyramid[level+1].Pixel(srcx,srcy,0)*2;
	      sources.Pixel(x,y,1) = 
		m_sourcesPyramid[level+1].Pixel(srcx,srcy,1)*2;
	      if (sources.Pixel(x,y,0) >= result.width())
		sources.Pixel(x,y,0) --;
	      if (sources.Pixel(x,y,1) >= result.height())
		sources.Pixel(x,y,1) --;

	      for(int d=0;d<result.dim();d++)
		result.Pixel(x,y,d) = 
		  m_filteredExamplePyramid[0][level].
		  Pixel(sources.Pixel(x,y,0),sources.Pixel(x,y,1),d);
	    }
	  SignalRedraw();
	}
      return;
    }
  */  

  vector<Neigh*> neighborhoods;
  vector<CVector<double> *> pixels;
  bool pointsGathered = false;
  bool pixelsGathered = false;

  TSVQR * rtree = NULL;
  MLP * mlp = NULL;
  CSearchEnvironment *searchEnvironment = NULL;

  // auxiliary mode mask data
  vector<Neigh*> neighborhoodsMM;
  bool pointsGatheredMM = false;
  CSearchEnvironment *searchEnvironmentMM = NULL;

  bool causal = bool(m_pass == 0);

  if (m_sampler == NULL)
    m_sampler = new Sampler<Point2>(m_samplerEpsilon);
  else
    m_sampler->setEpsilon(m_samplerEpsilon);

  m_useMMnow = false;
  m_sourceFacNow = m_finalSourceFac >= 0 && level == 0 ?
    m_finalSourceFac : m_sourceFac;

  // neighborhood-vector search methods:
  if( (m_usePCA || 
       m_searchType == TSVQ_SEARCH || 
       m_searchType == VECTOR_SEARCH || 
       m_searchType == HEURISTIC_SEARCH || 
       m_searchType == MLP_SEARCH ||
       m_searchType == TSVQR_SEARCH || 
       m_searchType == ANN_SEARCH ||
       m_searchType == HEURISTIC_ANN_SEARCH ) && 
      !(level==0 && m_ashikhminLastLevel) )
    {
      pointsGathered = true;
      pixelsGathered = (m_searchType == MLP_SEARCH || 
			m_searchType == TSVQR_SEARCH);
      
      GatherTrainingData( level, neighborhoods, 
			  pixelsGathered ? &pixels : NULL, 
			  m_useFilterModeMask);

      int numSolidNeighborhoods = 0;
      for( int tempIndex = 0; tempIndex < neighborhoods.size(); tempIndex++ )
      {
	if( neighborhoods[tempIndex]->constSrc )
	  numSolidNeighborhoods++;
      }
      printf( "%d / %d neighborhoods are solid-colored\n",
	      numSolidNeighborhoods, neighborhoods.size() );
      
      printf("%d data points.\n", neighborhoods.size());

      if (m_savePointsToFile && level == 0)
	savePointsToFile(neighborhoods);

      // gather a second set of neighborhoods for use with mode mask
      if (m_useFilterModeMask || m_useTargetModeMask)
	{
	  m_useSourceImagesNow = m_modeMaskWeight > 0;
	  m_sourceFacNow = m_modeMaskWeight;
	  pointsGatheredMM = true;
	  
	  GatherTrainingData( level , neighborhoodsMM, NULL, false);
	  printf("%d MM data points.\n",neighborhoodsMM.size());
	  
	  m_useSourceImagesNow = m_useSourceImages;
	}
    }

  m_sourceFacNow = m_finalSourceFac >= 0 && level == 0 ?
    m_finalSourceFac : m_sourceFac;

  if (neighborhoods.size() > 0)
    {
      /*
	tree = new TSVQ<Neigh>(neighborhoods,m_maxTSVQerror,m_maxTSVQdepth);
	rtree = new TSVQR(neighborhoods,pixels,m_maxTSVQdepth);

	for(int i=0;i<neighborhoods.size();i++)
	{
	Neigh testn = *neighborhoods[i];
	for(int j=0;j<testn.Length();j++)
	testn[j] *= (drand48()-.5);

	printf("tsvq test\n");
	Neigh n = tree->findNearest(testn,m_numComparisons,m_TSVQbacktracks);
		
	Neigh rpt = rtree->findNearest(testn,m_numComparisons,m_TSVQbacktracks);
		
	if (rpt != n)
	{
	printf("error, dist =%f \nn=",n.dist(rpt));
	n.printFloat();printf("\nrpt=");
	rpt.printFloat();
	printf("\n");
	}
	}		      
      */

      if ((m_useFilterModeMask || m_useTargetModeMask) && m_usePCA)
	{
	  printf("Warning: PCA unavailable with mode mask\n");
	  m_usePCA = false;
	}
      
      if (m_usePCA)
	{
#ifdef LAPACK
	  m_eigenvectors = PCA(neighborhoods,m_mean);
#else
	  printf("PCA unavailable\n");
	  exit(1);
#endif
	}

      if( (m_searchType == TSVQ_SEARCH || m_searchType == HEURISTIC_SEARCH ) &&
	  !(m_ashikhminLastLevel && level==0) )
	{
	  CTSVQSearchEnvironment *evt = new CTSVQSearchEnvironment( neighborhoods, m_sampler, m_maxTSVQerror, m_maxTSVQdepth );
	  evt->SetBacktracks( m_TSVQbacktracks );
	  evt->SetNumComparisons( m_numComparisons );
	  searchEnvironment = evt;

	  if ((m_useFilterModeMask || m_useTargetModeMask) && 
	      !m_ignoreModeMaskPass)
	    {	
	      CTSVQSearchEnvironment *evt = new CTSVQSearchEnvironment( neighborhoodsMM, m_sampler, m_maxTSVQerror, m_maxTSVQdepth );
	      evt->SetBacktracks( m_TSVQbacktracks );
	      evt->SetNumComparisons( m_numComparisons );
	      searchEnvironmentMM = evt;	
	    }
	}

      if (m_searchType == MLP_SEARCH)
	{
	  printf("Training MLP\n.");
	  assert(pixels.front()->Length() > 0);

	  mlp = new MLP(neighborhoods,pixels,m_MLPnumHidden,
			m_MLPhiddenDecayWeight,m_MLPoutputDecayWeight,
			m_MLPsigmoidal);
	}
      
      if( (m_searchType == ANN_SEARCH || 
	   m_searchType == HEURISTIC_ANN_SEARCH) &&
	  !(m_ashikhminLastLevel && level==0) )
	{
	  CANNSearchEnvironment *evt = 
	    new CANNSearchEnvironment( neighborhoods, m_sampler );
	  evt->SetANNEpsilon( m_annEpsilon );
	  searchEnvironment = evt;
	  
	  if ((m_useFilterModeMask || m_useTargetModeMask) && 
	      !m_ignoreModeMaskPass)
	    {
	      CANNSearchEnvironment *evt = 
		new CANNSearchEnvironment( neighborhoodsMM, m_sampler );
	      evt->SetANNEpsilon( m_annEpsilon );
	      searchEnvironmentMM = evt;
	    }
	}
      
      if (m_searchType == TSVQR_SEARCH)
	{
	  printf("Training TSVQR.\n");
	  assert(pixels.front()->Length() > 0);
	
	  rtree = new TSVQR(neighborhoods,pixels,m_maxTSVQdepth);
	
	  int depth, numLeaves;
	  rtree->stats(depth, numLeaves);
	
	  printf("Done. Depth = %d, Num leaves = %d\n",depth, numLeaves);
      }
    }
  
  bool usingPoints = (m_searchType == VECTOR_SEARCH || m_usePCA || 
		      m_searchType == HEURISTIC_SEARCH ) &&
    searchEnvironment == NULL && rtree == NULL && neighborhoods.size() > 0;

  // free up the point data if possible
  if (pointsGathered && !usingPoints)
    {
      printf( "freeing neighborhoods\n" );

      vector<Neigh*>::iterator it;

      for(it=neighborhoods.begin(); it != neighborhoods.end(); ++it)
	delete *it;
      neighborhoods.erase( neighborhoods.begin(), neighborhoods.end() );

      for(it=neighborhoodsMM.begin(); it != neighborhoodsMM.end(); ++it)
	delete *it;
      neighborhoodsMM.erase( neighborhoodsMM.begin(), neighborhoodsMM.end() );
    }	
  
  // free up pixels, if any
  for(vector<CVector<double>*>::iterator it=pixels.begin(); 
      it != pixels.end(); ++it)
    delete *it;
  pixels.erase( pixels.begin(), pixels.end() );

  char imgname[50];
  sprintf(imgname,"filtered%d-pass%d.png",level,m_pass);

  CSearchEnvironment *currentSearchEnvironment = searchEnvironment;
  vector<Neigh*> * currentNeighborhoods = &neighborhoods;

  // another diagnostic image
  vector< ColorImage *> sourceLocHistoImages;
  float maxLocHistoValue = 0;
  if( m_createSrcLocHisto )
  {
    for( int index = 0; index < m_filteredExamplePyramid.size(); index++ )
    {
      ColorImage &img = m_filteredExamplePyramid[index][level];
      sourceLocHistoImages.push_back( new ColorImage( img.width(), 
						      img.height(), 1, 0 ) );
    }
  }

  if (m_searchType == ASHIKHMIN_SEARCH && m_pass == 0)
    {
      Image<GLubyte> &resultMask = m_destMaskPyramid[ level ];

      // create random initialization
      for(int iy=0;iy<result.height();iy++)
	for(int ix=0;ix<result.width();ix++)
	  {
	    for(int d=0;d<result.dim();d++)
	      result.Pixel(ix,iy,d) = ((double)rand() / (double)RAND_MAX)*
		result.maxVal();

	    sources.Pixel(ix,iy,0) = RandInt(result.width());
	    sources.Pixel(ix,iy,1) = RandInt(result.height());
	    resultMask.Pixel(ix,iy) = resultMask.maxVal();
	  }
    }
  
  int inc = ((m_pass%2) == 0 || m_oneway) ? 1 : -1;
  int startx = ((m_pass%2) == 0 || m_oneway) ? 0 : result.width()-1;
  int starty = ((m_pass%2) == 0 || m_oneway) ? 0 : result.height()-1;

  for(int iy=starty;iy<result.height() && iy >= 0;iy+= inc) // ugh
    {
      //#ifdef __UNIX__
      // CRs probably work differently under NT
      printf("Line %d / %d\r",iy,result.height()-1);
      fflush(stdout);
      //#endif

      if ( ((iy+1) % 100) == 0)
	result.savePNG(imgname);

      SignalRedraw();

//      printf("level = %d, y = %d\n",level,iy);
      for(int ix=startx;ix<result.width() && ix>=0;ix+=inc)  //ugh again
	{
	  bool computed = false;
	  Point2 bestLoc;
	  bool locComputed = false;

	  // do special two-level Ashikhmin search for last level:
	  if( m_ashikhminLastLevel && level == 0 )
	  {
	    bestLoc = FindBestMatchLocationAshikhmin( Point2( ix, iy ), level, 
						      true );
	    locComputed = true;
	  }
      
	  if (m_cheesyBoundaries && level < m_filteredPyramid.NLevels()-1
	      && level <= 1 && !locComputed)
	    {
	      int radius = (m_neighborhoodWidth-1)/2;
	      if (ix < radius || ix + radius + numLevels >= result.width() ||
		  iy < radius || iy + radius + numLevels >= result.height() )
		{
		  int srcx = ix/2, srcy = iy/2;
		  
		  if (srcx >= m_sourcesPyramid[level+1].width())
		    srcx --;
		  if (srcy >= m_sourcesPyramid[level+1].height())
		    srcy --;
		  
		  bestLoc[0] = 2*m_sourcesPyramid[level+1].Pixel(srcx,srcy,0);
		  bestLoc[1] = 2*m_sourcesPyramid[level+1].Pixel(srcx,srcy,1);
		  bestLoc.ImageID() = m_sourcesPyramid[level+1].Pixel(srcx,
								      srcy,2);

		  locComputed = true;
		}	      
	    }


	  if ((m_useFilterModeMask || m_useTargetModeMask) && 
	      !m_ignoreModeMaskPass && !locComputed)
	    {
	      // check if we use the filter at this pixel
	      ColorImage * maskIm; 
	      int srcx, srcy;
	      if (level == m_filteredPyramid.NLevels()-1 ||
		  m_useTargetModeMask)
		{
		  maskIm = &m_modeMaskPyramid[level];
		  srcx = ix;
		  srcy = iy;
		}
	      else
		{
		  maskIm = &m_modeMaskPyramid[level+1];
		  srcx = ix/2;
		  srcy = iy/2;
		  if (srcx >= maskIm->width())
		    srcx --;
		  if (srcy >= maskIm->height())
		    srcy --;
		}

	      m_useMMnow = maskIm->Pixel(srcx,srcy) == 0;
	      m_useSourceImagesNow = !m_useMMnow || m_modeMaskWeight>0;
	      
	      if (m_useMMnow)
		m_sourceFacNow = m_modeMaskWeight;
	      else
		m_sourceFacNow = m_finalSourceFac >= 0 && level == 0 ?
		  m_finalSourceFac : m_sourceFac;

	      currentNeighborhoods = m_useMMnow ? &neighborhoodsMM :
		&neighborhoods;

	      if( m_searchType == ANN_SEARCH || 
		  m_searchType == HEURISTIC_ANN_SEARCH ||
		  m_searchType == TSVQ_SEARCH ||
		  m_searchType == HEURISTIC_SEARCH )
		{
		  currentSearchEnvironment = m_useMMnow ? 
		    searchEnvironmentMM : searchEnvironment;
		}
	    }

	  if (!locComputed)
	  switch (m_searchType)
	    {
	    case HEURISTIC_ANN_SEARCH:
	    case HEURISTIC_SEARCH:
	      bestLoc = FindBestMatchLocationHeuristic( Point2(ix,iy), level,
							currentSearchEnvironment );
	      break;
	      
	    case TSVQ_SEARCH:
	    case ANN_SEARCH:
	      bestLoc = FindBestMatchLocation( Point2( ix, iy ), level,
					       currentSearchEnvironment );
	      break;

	    case ASHIKHMIN_SEARCH:
	      bestLoc = FindBestMatchLocationAshikhmin( Point2( ix, iy ), 
							level);
	      break;
	      
	    case VECTOR_SEARCH:
	      if (neighborhoods.size() == 0)
		bestLoc = FindBestMatchLocation( Point2( ix, iy ), level );
	      else
		computed = 
		  FindBestMatchLocationSearchPoints(Point2(ix,iy), level, 
						    *currentNeighborhoods, 
						    bestLoc);
	      break;

	    case TSVQR_SEARCH:
	      if (rtree != NULL)
		computed = ApplyTSVQR( rtree, Point2(ix,iy), level, bestLoc);
	      else
		bestLoc = FindBestMatchLocation( Point2( ix, iy ), level );
	      break;

	    case MLP_SEARCH:
	      computed = ApplyMLP( mlp, Point2(ix,iy), level, bestLoc );
	      break;
	      
	    default:
	    case IMAGE_SEARCH:
	      bestLoc = FindBestMatchLocation( Point2( ix, iy ), level );
	      break;
	    }

	  if (computed)
	    {
	      if (m_useFilterModeMask || m_useTargetModeMask)
		printf("Warning: filter mode mask set with regression\n");

	      ix ++;
	      continue;
	    }

	  if (m_coherenceEps != 0 && bestLoc[0] >= 0 && bestLoc[1] >= 0)
	    bestLoc = ApplyPrediction(Point2(ix,iy),level,bestLoc,
				      coherenceFac);

	  if( bestLoc[0] < 0 || bestLoc[1] < 0 )
	    {
	      static bool firstTime = true; // ??
	      assert( firstTime || m_searchType == ASHIKHMIN_SEARCH );
	      firstTime = false;

	      if (m_useRandom)
		{
		  // pick random example image to use, then pick a location
		  int imageNum = RandInt( m_filteredExamplePyramid.size() );
		  int rx = RandInt( m_filteredExamplePyramid[imageNum][level].width() );
		  int ry = RandInt( m_filteredExamplePyramid[imageNum][level].height() );
		  printf( "picking random src loc for pixel %d %d at %d; loc = %d %d in image # %d\n",
			  ix, iy, level, rx, ry, imageNum );
		  bestLoc = Point2( rx, ry);
		  bestLoc.ImageID() = imageNum;

		  // for debugging
		  //		  bestLoc = Point2(8,8);
		  //		  bestLoc.ImageID() = imageNum;
		  //		  printf("!!! debug start %d %d \n",bestLoc[0],bestLoc[1]);
		}
	      else
		{
		  printf( "copying src loc for pixel %d %d at %d; using %d %d\n",
			  ix, iy, level, ix, iy );
		  bestLoc = Point2( ix, iy ); // ??? how can this work? what if the dest is bigger than the example? ?? ???

		  // I believe this case can only happen at (0,0) in the coarsest
		  // level of the pyramid in pure synthesis when random start
		  // is unchecked.  -AH

		  bestLoc.ImageID() = 0;

		  assert(ix ==0 && iy ==0);
		} 
	    }
    
	  assert( bestLoc.ImageID() >= 0 );
	  assert(m_filteredExamplePyramid[bestLoc.ImageID()][ level ].inside(bestLoc[0], 
									     bestLoc[1]));
	  
	  result.copyPixel(ix,iy,m_filteredExamplePyramid[bestLoc.ImageID()][level],
			   bestLoc[0],bestLoc[1]);
	  resultMask.Pixel( ix, iy ) = resultMask.maxVal();
	  sources.Pixel(ix,iy,0) = bestLoc[0];
	  sources.Pixel(ix,iy,1) = bestLoc[1];
	  sources.Pixel(ix,iy,2) = bestLoc.ImageID();

	  if (m_useFilterModeMask && !m_useTargetModeMask)
	    m_modeMaskPyramid[level].Pixel(ix,iy,0) = 
	      m_exampleModeMaskPyramid[level].Pixel(bestLoc[0],
						    bestLoc[1], 0);
	  
	  if( m_createSrcLocHisto )
	    {
	      ColorImage *img = sourceLocHistoImages[bestLoc.ImageID()];
	      img->Pixel( bestLoc[0], bestLoc[1], 0 ) += 1;
	      maxLocHistoValue = max( maxLocHistoValue, 
				      img->Pixel( bestLoc[0], bestLoc[1], 0));
	    }
	}
    }
  
  // write histo images out and destroy them
  if( m_createSrcLocHisto )
  {
    printf( "Saving source location histogram images\n" );
    for( int index = 0; index < sourceLocHistoImages.size(); index++ )
    {
      ColorImage *img = sourceLocHistoImages[index];
      for( int iy = 0; iy < img->height(); iy++ )
	for( int ix = 0; ix < img->width(); ix++ )
	{
	  float val = img->Pixel( ix, iy, 0 );
	  if( val > 0 )
	    img->Pixel( ix, iy, 0 ) = (10.0/255.0) + (245.0/255.0)*(val / maxLocHistoValue);
	}
      char histoImgName[1000];
      sprintf( histoImgName, "SourceHisto.%d.%d.png", index, level );
      img->savePNG( histoImgName );
      delete img;
    }
    
    sourceLocHistoImages.erase( sourceLocHistoImages.begin(), sourceLocHistoImages.end() );
  }
  
  SignalRedraw();
  
  if( searchEnvironment )
    delete searchEnvironment;
  
  if( searchEnvironmentMM )
    delete searchEnvironmentMM;

  if (pointsGathered && usingPoints)
    {
      vector<Neigh*>::iterator it;

      for(it=neighborhoods.begin(); it != neighborhoods.end(); ++it)
	delete *it;
      
      for(it=neighborhoodsMM.begin(); it != neighborhoodsMM.end(); ++it)
	delete *it;
    }

  sprintf(imgname,"filtered%d-pass%d.png",level,m_pass);
  result.savePNG(imgname);

  //  printf("Sampling percentages: ");
  //  m_sampler->printStats();

  if( m_searchType == ASHIKHMIN_SEARCH || (level==0 && m_ashikhminLastLevel))
  {
    m_avgNumCandidates /= m_numPixelsSynthesized;
    printf( "avg # candidates: %f (of %d)\n", m_avgNumCandidates, m_numPixelsSynthesized );
  }
}


template < class TSource, class TFilter> 
Point2 
CFilterLearner<TSource, TFilter>::
ApplyPrediction(Point2 targetLoc, int currLevel, Point2 bestLoc,
		float coherenceFac)
{
  assert(m_coherenceEps != 0);
  
  // check to see if any of the prediction candidates do better than bestLoc
  set<Point2> candidates;
  bool hitValidN = GetPredictions(targetLoc,currLevel,false,candidates, 
				  m_pass==0);
  
  if (!hitValidN || candidates.find(bestLoc) != candidates.end())
    return bestLoc;
  
  bool isValid = false;

  double bestDist = 
    GetNeighborhoodDist(targetLoc,bestLoc,currLevel,isValid,
			m_neighborhoodWidth) * coherenceFac;
  
  assert(isValid);
      
  for(set<Point2>::iterator it=candidates.begin();it!=candidates.end();++it)
    {
      isValid = false;
      double d2 = GetNeighborhoodDist(targetLoc,*it,currLevel,isValid,
				      m_neighborhoodWidth,bestDist);
      
      if (d2 < bestDist && isValid)
	{
	  bestDist = d2;
	  bestLoc = *it;
	}
    }

  return bestLoc;
}

template < class TSource, class TFilter >
void
CFilterLearner<TSource,TFilter>::
GatherTrainingData(int currLevel,vector<Neigh*> & points,
		   vector<CVector<double> *> * pixels,
		   bool ignoreUnmaskedPixels)
{
  printf("Collecting training data\n");

  assert( m_filteredExamplePyramid.size() > 0 );
  if( m_useSourceImagesNow )
    assert( m_filteredExamplePyramid.size() == m_sourceExamplePyramid.size() );
  
  for( int imageID = 0; imageID < m_filteredExamplePyramid.size(); imageID++ )
  {
    Pyramid<TFiltImage> * img = & m_filteredExamplePyramid[imageID];
    Pyramid<TSrcImage> * imgProto = m_useSourceImagesNow ? 
      &m_sourceExamplePyramid[imageID] : NULL;

    int height = (*img)[currLevel].height();
    int width = (*img)[currLevel].width();
    
    const int nw = m_neighborhoodWidth;
    const int radius = (nw-1)/2;
    
    int maxLevel = min( currLevel+m_numLevels-1, img->NLevels()-1 );
    int num = maxLevel - currLevel;
    
    for( int iy = radius; iy < height-radius-num; iy++ )
      for( int ix = radius; ix < width-radius-num; ix++ )
      {
	// skip unmasked pixels
	if (m_useFilterModeMask && ignoreUnmaskedPixels &&
	    !m_exampleModeMaskPyramid[currLevel].Pixel(ix,iy))
	  continue;

	Neigh * n = new Neigh( GetNeighborhood( Point2(ix,iy, imageID), 
						currLevel, false,
						img, imgProto ));
	points.push_back(n);
	
	if (pixels != NULL)
	{
	    int dim = (*img)[currLevel].dim();
	    assert(dim > 0);
	    CVector<double> * p = new CVector<double>(dim);
	    if (p == NULL)
	    {
	      printf("out of memory allocating vector in Gather\n");
	      exit(1);
	    }
	    assert(p->Length() > 0);
	    for(int d=0;d<dim;d++)
	      (*p)[d] = (*img)[currLevel].Pixel(ix,iy,d);
	    
	    pixels->push_back(p);
	    assert(pixels->back()->Length() > 0);
	}
      }	    
  }
  /*
#ifndef NDEBUG
  if(!pixels->empty())
    assert(pixels->front()->Length() > 0);
#endif
  */
}

#ifdef LAPACK
template < class TSource, class TFilter >
CMatrix<double>
CFilterLearner<TSource,TFilter>::PCA(vector<Neigh*> & points, Neigh & mean, 
				     float minRatio)
     // this ought to be a member of CMatrix instead
{
  printf("Computing PCA.\n");

  assert(points.size() > 0);
  int dim = points[0]->Length();
	
  mean = TSVQ<Neigh>::centroid(points);

  // create a scatter matrix ( this code could be hand-tuned)
  int i,j;
  CMatrix<double> scatter(dim,dim);
  scatter.clear();
  //  for(i=0;i<dim;i++)
  //    for(j=0;j<dim;j++)
  //      scatter.get(i,j) = 0;

  vector<Neigh*>::iterator it;
  for(it = points.begin();it != points.end();++it)
    {
      Neigh msub = **it - mean;
      for(i=0;i<dim;i++)
	{
	  scatter.get(i,i) += msub[i]*msub[i];
	  for(j=i+1;j<dim;j++)
	    scatter.get(i,j) += msub[i]*msub[j];
	}
    }

  // symmettrize
  for(i=0;i<dim;i++)
    for(j=0;j<i;j++)
      {
	assert(scatter.get(i,j) == 0);
	//	assert(scatter.get(j,i) == 0);// note: this could legitimately fail for 
	// independent components
	scatter.get(i,j) = scatter.get(j,i);
      }
  
  //cov /= points.size();
  
  // find out the eigenvalues
  CVector<double> eigenvals = scatter.symmetricEigenvalues();

  /*
  printf("evals = ");
  for(i=0;i<eigenvals.Length();i++)
    printf("%f ",eigenvals[i]);
  printf("\n");
  */  

  // decide how many eigenvectors to keep
  
  int nVecs;
  for(nVecs=2;nVecs<=dim;nVecs++)
    {
      assert(eigenvals[dim - nVecs] <= eigenvals[dim-1]);
      float ratio = eigenvals[dim-nVecs]/eigenvals[dim-1];
      if (ratio < minRatio && -ratio < minRatio)
	break;
    }
  nVecs --;

  // compute eigenvectors
  CMatrix<double> evT(dim,nVecs);
  CVector<double> eigenvals2 = 
    scatter.symmetricEigenvectors(evT,dim,dim-nVecs+1);

  CMatrix<double> eigenvectors = evT.transpose(); 
  // project data into reduced space
  for(it=points.begin();it!=points.end();++it)
  {
    Neigh * pt = *it;
    *it = new Neigh(eigenvectors * (*pt - mean));
    (*it)->loc = pt->loc;
    delete pt;		
  }

  printf("Kept %d of %d dimensions.  First %d eigenvalues are: ",nVecs,dim,nVecs+1);
  for(i=0;i<nVecs;i++)
    printf("%f ",eigenvals2[i]);
  printf("\n");


  /*
#ifndef NDEBUG
  printf("testing eigenmatrix: diff = \n");
  for(int i=0;i<nVecs;i++)
    {
      CVector<double> v = eigenvectors.getRow(i);
      CVector<double> diff = scatter*v - v*eigenvals2[i];

      printf("%f ",diff.L2());
    }
  printf("\n");
#endif
  */

  // also ineffecient to copy this
  return eigenvectors;
}
#endif

template < class TSource, class TFilter >
Neigh
CFilterLearner<TSource,TFilter>::
GetNeighborhood(Point2 loc, int currLevel,bool computeWeightVector,
		Pyramid<TFiltImage> * img,Pyramid<TSrcImage> * imgProto)  
{
  bool isFirstPixel = true;

  //  assert( loc.ImageID() >= 0 );
  //  assert( loc.ImageID() < m_filteredExamplePyramid.size() );

  int i;

  bool useSource = (imgProto != NULL);
  //  bool useCoarse = (currLevel < img->NLevels()-1);
  // bool useFilteredCoarse = false;

  int maxLevel = min( currLevel+m_numLevels-1, 
		      img->NLevels()-1 );
		      //		      m_filteredExamplePyramid[loc.ImageID()].NLevels()-1 );
  
  assert(maxLevel <= 20);

  int radius = (m_neighborhoodWidth-1)/2;
  
  int nsize = 0;

  int dim = img->FineImage().dim();
  //  int dim = m_filteredExamplePyramid[loc.ImageID()].FineImage().dim();
  //  int dim = m_grayscaleMode ? 1 : m_filteredExamplePyramid.FineImage().dim();

  // assume that the neighborhood is causal if the pixel has not been
  // synthesized already
  //  bool causal = bool(m_destMaskPyramid[ currLevel ].Pixel(loc[0],loc[1]) == 0);
  bool causal = bool(m_pass == 0);

  // determine the vector size
  for(i=currLevel;i<=maxLevel && radius>=1;i++)
    {
      int width = 2*radius+1;
      if (i == currLevel && causal)
	nsize += dim*(width*radius+radius);
      else
	nsize += dim*width*width;
      if (useSource && !m_onePixelSource)
	nsize += dim*width*width;
      radius >>= 1;  // divide by 2 and round down
    }
  if (!causal)
    nsize -= dim; // subtract center pixel
  
  if (useSource && m_onePixelSource)
    nsize += dim;

  Neigh n(nsize,loc);
  n.constSrc = true;
  
  Point2 p = loc;

  i = 0;

  float levelWeight = 1;
  int level;
  double totalWeight;
  //  double totalTotalWeight = 0;

  for(level = currLevel,radius = (m_neighborhoodWidth-1)/2;
      level <= maxLevel && radius >= 1 && levelWeight != 0; 
      level ++)
    {
      int width = 2*radius+1;
      int levelStart = i;

      TFiltImage & imcur = (*img)[level];
      
      TFilter maxVal = imcur.maxVal();

      totalWeight = 0;
      
      bool levelCausal = causal && level == currLevel;

      for(int iy = 0;iy <= radius || (iy < width && !levelCausal);++iy)
	for(int ix = 0;ix < width && (ix < radius || iy < radius ||
				      !levelCausal); ++ ix)
	  {
	    // skip center pixel
	    if (iy == radius && ix == radius && level == currLevel)
	      continue;

	    double fac = m_kernel->get(width,iy)*m_kernel->get(width,ix)/
	      maxVal;  
	    if (computeWeightVector)
	      for(int d=0;d<dim;d++)
		n[i++] = fac;
	    else
	      for(int d=0;d<dim;d++)
		n[i++] = fac*imcur.Pixel(p[0]+ix-radius,p[1]+iy-radius,d);
	    totalWeight += fac;
	  }
      
      if (useSource && !m_onePixelSource)
	{
 	  TSrcImage & impcur = (*imgProto)[level];
	  TSource maxVS = impcur.maxVal();

	  int sdim = impcur.dim();
	  //	  int sdim = m_grayscaleMode ? 1 : impcur.dim();
	  for(int iy = 0;iy < width;++iy)
	    for(int ix = 0;ix < width; ++ ix)
	      {
		double fac = m_sourceFacNow*m_kernel->get(width,iy)*
		  m_kernel->get(width,ix)/maxVS;
		if (computeWeightVector)
		  for(int d=0;d<sdim;d++)
		    n[i++] = fac;
		else
		  for(int d=0;d<sdim;d++)
		    n[i++] = fac*impcur.Pixel(p[0]+ix-radius,p[1]+iy-radius,d);
		totalWeight += fac;

		// determining if neighborhood is constant-colored
		if( isFirstPixel )
		{
		  for( int d = 0; d < sdim; d++ )
		  {
		    n.constSrcColor[d] = impcur.Pixel(p[0]+ix-radius,p[1]+iy-radius,d);
		  }

		  n.srcPixelDim = sdim;
		  isFirstPixel = false;
		}
		else
		{
		  for( int d = 0; d < sdim; d++ )
		    if( n.constSrcColor[d] != impcur.Pixel(p[0]+ix-radius,p[1]+iy-radius,d) )
		      n.constSrc = false;
		}
		
	      }
	  
	  
	}
      else
	if (useSource && m_onePixelSource && level == currLevel)
	  {
	    n.constSrc = true;
	    TSrcImage & impcur = (*imgProto)[currLevel];
	    int sdim = impcur.dim();

	    if (computeWeightVector)
	      for(int d=0;d<sdim;d++)
		n[i++] = 1;
	    else
	      for(int d=0;d<sdim;d++)
		n[i++] = m_sourceFacNow*impcur.Pixel(p[0],p[1],d);
	    totalWeight += m_sourceFacNow;
	  }
      

      //      printf("level = %d, totalWeight = %f\n",level,totalWeight);
	     

      for(int q=levelStart;q<i;q++)
	n[q] *= (levelWeight/totalWeight);

      p[0] /= 2; 
      p[1] /= 2;
      radius >>= 1;

      levelWeight *= m_levelWeighting;

      //      totalTotalWeight += levelWeight*(i-levelStart);
    }

  assert (i == n.Length());
  assert(n.loc[0] >=0 && n.loc[1] >=0);

  /*
  printf("n = [");
  for(i=0;i<n.Length();i++)
    printf("%f ",n[i]);
  printf("]  tw = %f   lw = %f\n",totalWeight,levelWeight);
  */
  /*
  // mean subtract
  if (m_useBias && !m_useGain)
    {
      n.weightedMean = 0;
      for(i=0;i<n.Length();i++)
	n.weightedMean += n[i];
      n.weightedMean /= totalTotalWeight;
      for(i=0;i<n.Length();i++)
	n[i] -= n.weightedMean;
    }
  */

  return n;
}


template < class TSource, class TFilter >
bool CFilterLearner<TSource,TFilter>::
ApplyMLP(MLP * mlp, Point2 targetLoc, int currLevel, Point2 & bestLoc)
{
#ifndef BFGS
  printf("Can't use neural network\n");
  exit(1);
  return false;
  
#else
  const int nw = m_neighborhoodWidth;
  const int radius = (nw-1)/2;

  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;

  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );
  int num = maxLevel - currLevel;
  if (targetLoc[0] < radius || 
      targetLoc[0] + radius + num >= (*img)[currLevel].width() ||
      targetLoc[1] < radius || 
      targetLoc[1] + radius + num >= (*img)[currLevel].height() )
    {
      bestLoc = FindBestMatchLocation(targetLoc,currLevel);
      return false;
    }

  Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt );
    
  Neigh n;
  if (m_usePCA)
    n = m_eigenvectors*(nb-m_mean);
  else
    n = nb;
  
  CVector<double> pix = mlp->apply(n);

  assert(pix.Length() == (*img)[currLevel].dim());

  for(int d=0;d<pix.Length();d++)
    (*img)[currLevel].Pixel(targetLoc[0], targetLoc[1], d) = pix[d];
  
  return true;
#endif
}

template < class TSource, class TFilter >
bool CFilterLearner<TSource,TFilter>::
ApplyTSVQR(TSVQR * rtree, Point2 targetLoc, int currLevel, Point2 & bestLoc)
{
#ifndef LAPACK
  printf("Can't use TSVQR\n");
  exit(1);
  return false;
  
#else
  const int nw = m_neighborhoodWidth;
  const int radius = (nw-1)/2;

  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;

  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );
  int num = maxLevel - currLevel;
  if (targetLoc[0] < radius || 
      targetLoc[0] +radius+num >= (*img)[currLevel].width() ||
      targetLoc[1] < radius || 
      targetLoc[1] +radius+num >= (*img)[currLevel].height() )
    {
      bestLoc = FindBestMatchLocation(targetLoc,currLevel);
      return false;
    }

  Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt );
    
  Neigh n;
  if (m_usePCA)
    n = m_eigenvectors*(nb-m_mean);
  else
    n = nb;
  
  //  bestLoc = rtree->apply(n,m_numComparisons,m_TSVQbacktracks);

  //  return false;


  CVector<double> pix = rtree->apply(n,m_numComparisons,m_TSVQbacktracks);

  assert(pix.Length() == (*img)[currLevel].dim());

  for(int d=0;d<pix.Length();d++)
    (*img)[currLevel].Pixel(targetLoc[0], targetLoc[1], d) = pix[d];
  
  return true;

#endif
}

template < class TSource, class TFilter >
Point2
CFilterLearner<TSource,TFilter>::
FindBestMatchLocationHeuristic( Point2 targetLoc, int currLevel,
				CSearchEnvironment *searchEnvironment )
{
  // use brute-force method if need be
  if( searchEnvironment == NULL || currLevel >= m_filteredPyramid.NLevels() )
    return FindBestMatchLocation( targetLoc, currLevel );

  // #### currently, this procedure only looks at the first example pair

  assert( searchEnvironment != NULL );
  assert( m_filteredExamplePyramid.size() > 0 );

  if( m_filteredExamplePyramid.size() > 1 )
    fprintf( stderr, "WARNING! currently using only one example pair for heuristic matching\n" );

  const int nw = m_neighborhoodWidth;
  const int radius = (nw-1)/2;

  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;

  // cheesy thing -- using # levels from first example pyramid. oh well, these had better all be the same, anyway
  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );
  int num = maxLevel - currLevel;
  if (targetLoc[0] < radius ||
      targetLoc[0] + radius +num>= (*img)[currLevel].width() ||
      targetLoc[1] < radius || 
      targetLoc[1] + radius +num>= (*img)[currLevel].height() )
    return FindBestMatchLocation( targetLoc, currLevel, searchEnvironment );
    //    return FindBestMatchLocation(targetLoc,currLevel);

  /*
  Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt );
  Neigh n;
  if (m_usePCA)
    n = m_eigenvectors*(nb-m_mean);
  else
    n = nb;

  Point2 resultLoc = searchEnvironment->FindBestMatchLocation( n );
*/

  Point2 bestMatch(-1,-1);
  float bestDist = FLT_MAX;

  // assume that the neighborhood is causal if the pixel has not been
  // synthesized already
  //  bool causal = bool(m_destMaskPyramid[ currLevel ].Pixel(targetLoc[0],
  //targetLoc[1]) == 0);

  bool causal = bool(m_pass == 0);

  deque<Point2> ready;
  set<Point2> predPts;
  set<Point2> searchPts;

  int exampleWidth = m_filteredExamplePyramid[0][currLevel].width();
  int exampleHeight = m_filteredExamplePyramid[0][currLevel].height();

  m_flags.clearAllFlags();

  //  ready.push_back( resultLoc );
  //    m_flags.setFlag( resultLoc[0], resultLoc[1], currLevel );

  Image<srcCoord> & sources = m_sourcesPyramid[currLevel];

  if (targetLoc[0] > 0)
    {
      int srcx = sources.Pixel(targetLoc[0]-1,targetLoc[1],0);
      int srcy = sources.Pixel(targetLoc[0]-1,targetLoc[1],1);
      if (srcx < exampleWidth-1 && !m_flags.testAndSet(srcx+1,srcy,currLevel))
      {
	ready.push_back(Point2(srcx+1,srcy));
	if( m_keepSearchStats )
	{
	  predPts.insert(Point2(srcx+1,srcy));
	}
      }
    }

  if (targetLoc[1] > 0)
    {
      int srcx = sources.Pixel(targetLoc[0],targetLoc[1]-1,0);
      int srcy = sources.Pixel(targetLoc[0],targetLoc[1]-1,1);
      //      stack.insert(Point2(srcx,srcy));
      if (srcy < exampleHeight-1 && !m_flags.testAndSet(srcx,srcy+1,currLevel))
      {
	ready.push_back(Point2(srcx,srcy+1));
	if( m_keepSearchStats )
	{
	  predPts.insert(Point2(srcx,srcy+1));
	}
      }
      //	stack.insert(Point2(srcx,srcy+1));
    }
      
  if (currLevel < m_sourcesPyramid.NLevels()-1)
    {
      int srcx = m_sourcesPyramid[currLevel+1].
	Pixel(targetLoc[0]/2,targetLoc[1]/2,0);
      int srcy = m_sourcesPyramid[currLevel+1].
	Pixel(targetLoc[0]/2,targetLoc[1]/2,1);
      if (!m_flags.testAndSet(2*srcx,2*srcy,currLevel))
      {
	ready.push_back(Point2(2*srcx,2*srcy));
	if (m_keepSearchStats)
	{
	  predPts.insert(Point2(2*srcx,2*srcy));
	}
      }
    }

  if (!causal)
    {
      int srcx = sources.Pixel(targetLoc[0],targetLoc[1],0);
      int srcy = sources.Pixel(targetLoc[0],targetLoc[1],1);
      if (!m_flags.testAndSet(srcx,srcy,currLevel))
      {
	ready.push_back(Point2(srcx,srcy));
	if (m_keepSearchStats)
	{
	  predPts.insert(Point2(srcx,srcy));
	}
      }

      if (targetLoc[0] < exampleWidth-1)
	{
	  int srcx = sources.Pixel(targetLoc[0]+1,targetLoc[1],0);
	  int srcy = sources.Pixel(targetLoc[0]+1,targetLoc[1],1);
	  if (srcx > 0 && !m_flags.testAndSet(srcx-1,srcy,currLevel))
	  {
	    ready.push_back(Point2(srcx-1,srcy));
	    if (m_keepSearchStats)
	    {
	      predPts.insert(Point2(srcx-1,srcy));
	    }
	    
	  }
	}

      if (targetLoc[1] < exampleHeight-1)
	{
	  int srcx = sources.Pixel(targetLoc[0],targetLoc[1]+1,0);
	  int srcy = sources.Pixel(targetLoc[0],targetLoc[1]+1,1);
	  if (srcy > 0 && !m_flags.testAndSet(srcx,srcy-1,currLevel))
	  {
	    ready.push_back(Point2(srcx,srcy-1));
	    if (m_keepSearchStats)
	    {
	      predPts.insert(Point2(srcx,srcy-1));
	    }
	  }
	}
    }


  //        printf("searchin'\n");

  m_sampler->eraseSamples();

  bool usedTree = false; 
  int numTested = -ready.size();

  Point2 treeResultLoc;

  //  while (!ready.empty())
  while (!ready.empty() || (!usedTree && bestDist > m_annEpsilon))
    {
      /*
      printf( "size: %d, used tree: %s, bestDist: %f, eps: %f\n", 
	      ready.size(), usedTree ? "yes" : "no", bestDist, m_annEpsilon );
      */

      if (ready.empty())
      {
	usedTree = true;
	Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt );

	/*
	// TODO: this?
	Neigh n;
	if (m_usePCA)
	n = m_eigenvectors*(nb-m_mean);
	else
	n = nb;
	annResultLoc = searchEnvironment->FindBestMatchLocation( n );
	*/

	treeResultLoc = searchEnvironment->FindBestMatchLocation( nb );
	ready.push_back( treeResultLoc );
	m_flags.setFlag( treeResultLoc[0], treeResultLoc[1], currLevel );
      }
      
      Point2 ptnew = ready.front();
      //      assert( ptnew.ImageID() >= 0 );
      ptnew.ImageID() = 0; // the heuristic search only works for 1 example pair
      ready.pop_front();
      
      numTested++;

      assert( m_filteredExamplePyramid[0][currLevel].inside(ptnew[0],ptnew[1]) );
      
      bool isValid = false;
      float dist = GetNeighborhoodDist(targetLoc, ptnew, currLevel, 
				       isValid, m_neighborhoodWidth,
				       m_sampler->threshold() );

      //      m_sampler->addElement(dist,ptnew);

      if( isValid )
	m_sampler->addElement(dist,ptnew);

      //            printf("loc = %d %d, dist = %f; bestDist = %f\n",ptnew[0],ptnew[1],dist,
      //                  	     bestDist);
      if (isValid && dist < bestDist)
	{

	  //	  printf("\tbetter\n");
	  bestDist = dist;
	  bestMatch = ptnew;

	  if (ptnew[0] > 0 && 
	      !m_flags.testAndSet(ptnew[0]-1,ptnew[1],currLevel))
	  {
	    ready.push_back(Point2(ptnew[0]-1,ptnew[1]));
	    if (m_keepSearchStats)
	      searchPts.insert(Point2(ptnew[0]-1,ptnew[1]));
	  }
	  
	  if (ptnew[1] > 0 && 
	      !m_flags.testAndSet(ptnew[0],ptnew[1]-1,currLevel))
	  {
	    ready.push_back(Point2(ptnew[0],ptnew[1]-1));
	    if (m_keepSearchStats)
	      searchPts.insert(Point2(ptnew[0],ptnew[1]-1));
	  }
	  
	  if (ptnew[0] < exampleWidth -1 &&
	      !m_flags.testAndSet(ptnew[0]+1,ptnew[1],currLevel))
	  {
	    ready.push_back(Point2(ptnew[0]+1,ptnew[1]));
	    if (m_keepSearchStats)
	      searchPts.insert(Point2(ptnew[0]+1,ptnew[1]));
	  }

	  if (ptnew[1] < exampleHeight -1 &&
	      !m_flags.testAndSet(ptnew[0],ptnew[1]+1,currLevel))
	  {
	    ready.push_back(Point2(ptnew[0],ptnew[1]+1));
	    if (m_keepSearchStats)
	      searchPts.insert(Point2(ptnew[0],ptnew[1]+1));
	  }
	}
    }
  
   /*
     printf("numTested = %d; usedAnn = %s; bestDist = %f, eps = %f\n",numTested,usedAnn ? "yes" : "no",
   	 bestDist,m_annEpsilon);
   */
 
   //  return  m_sampler->UniformSample().second;
   Point2 finalResult =  m_sampler->BestSample().second;
   assert( finalResult.ImageID() >= 0 );
   //   finalResult.imageID = 0; // ???
   //   return result;

   //  return m_sampler->UniformSample().second;
   
   if (m_keepSearchStats)
     {
       /*
       printf("finalResult = [%d %d]\n",finalResult[0],finalResult[1]);
       printf("\tannResultLoc = [%d %d]\n",annResultLoc[0],annResultLoc[1]);
       printf("\tpredPoints = ");
       for(set<Point2>::iterator it=predPts.begin();it != predPts.end();++it)
 	printf("[%d %d] ",(*it)[0],(*it)[1]);
       printf("\n");
       */
       m_numSearches ++;
       m_totalSearchDist += numTested;
 
       bool treeHit = bool (usedTree && finalResult[0] == treeResultLoc[0] && 
			    finalResult[1] == treeResultLoc[1]);
 
       if (treeHit)
	 m_numTreeHits ++;
       
       //      printf("pred ct = %d\n",predPts.count(Point2(finalResult[0],finalResult[1])));
 
       bool predHit = false;
 
       for(set<Point2>::iterator it2=predPts.begin();it2!=predPts.end();++it2)
 	if ( (*it2)[0] == finalResult[0] && (*it2)[1]==finalResult[1])
 	  {
 	    predHit = true;
 	    m_numPredHits ++;
 
 	    if (!treeHit)
 	      m_numPredOnlyHits ++;
 	    break;
 	  }
       for(set<Point2>::iterator it3=searchPts.begin();it3!=searchPts.end();++it3)
 	if ( (*it3)[0] == finalResult[0] && (*it3)[1]==finalResult[1])
 	  {
 	    m_numSearchHits ++;
 	    break;
 	  }
 
       /*
       if (predPts.find(Point2(finalResult[0],finalResult[1])) != 
       	  predPts.end())
       	m_numPredHits ++;
       
       if (searchPts.find(Point2(finalResult[0],finalResult[1])) != 
 	  searchPts.end())
 	m_numSearchHits ++;
       */
     }

   return finalResult;
}

/*
template <class TSource, class TFilter >
Point2
CFilterLearner<TSource,TFilter>::
FindBestMatchLocationPrediction(Point2 targetLoc, int currLevel,
				ANNkd_tree *annTree, int annDim,
				ANNpoint &annQueryPoint,
				const vector<Point2> &neighborhoodLocs)
{

}
*/

template < class TSource, class TFilter >
void 
CFilterLearner<TSource,TFilter>::
optimalBias(const Neigh & target,const Neigh & source,
	    const Neigh & weights,double * dist, double * bias)
{
  double totalWeights = weights.sum();
  double targetWmean = target.sum()/(totalWeights + m_biasPenalty);
  double sourceWmean = source.sum()/(totalWeights + m_biasPenalty);
  
  //  printf("ts = %f, ss = %f, ws = %f\n",target.sum(),source.sum(),
  //	 weights.sum());
  double b = targetWmean - sourceWmean;

  if (bias != NULL)
    *bias = b;

  if (dist != NULL)
    *dist = (target - weights*targetWmean).dist2(source-weights*sourceWmean) +
      b*b*m_biasPenalty/2;
}

template < class TSource, class TFilter >
void 
CFilterLearner<TSource,TFilter>::
optimalGain(const Neigh &target,const Neigh & source, 
	    const Neigh &weights, double * dist, double * gain)
{
  Neigh unweightedSource(source.Length(),Point2(-1,-1));
  for(int i=0;i<source.Length();i++)
    unweightedSource[i] =source[i] / weights[i];

  double denom = unweightedSource.dot(source) + m_gainPenalty;
  double g;

  if (denom < 1e-8 && denom > -1e-8)
    g = 1;
  else
    g = (unweightedSource.dot(target) + m_gainPenalty)/denom;

  if (gain != NULL)
    *gain = g;

  if (dist != NULL)
    {
      double gsub = g-1;
      *dist = target.dist2(source*g) + gsub*gsub*m_gainPenalty/2;
    }
}

template < class TSource, class TFilter >
void 
CFilterLearner<TSource,TFilter>::
optimalBiasGain(const Neigh & target,const Neigh & source,
		const Neigh & weights, double * dist, double * bias,
		double * gain)
{
  double totalWeights = weights.sum();
  double targetWmean = target.sum()/(totalWeights+m_biasPenalty);
  double sourceWmean = source.sum()/(totalWeights+m_biasPenalty);
  Neigh wtm = weights * targetWmean;
  Neigh wsm = weights * sourceWmean;
  Neigh unweightedSource(source.Length(),Point2(-1,-1));
  for(int i=0;i<source.Length();i++)
    unweightedSource[i] =source[i] / weights[i];

  double denom = unweightedSource.dot(source - wsm) + m_gainPenalty;
  double b,g;

  if (denom < 1e-8 && denom > -1e-8)
    g = 1;
  else
    g = (unweightedSource.dot(target - wtm)+m_gainPenalty)/denom;
  
  b = targetWmean - g * sourceWmean;

  if (bias != NULL)
    *bias = b;
  if (gain != NULL)
    *gain = g;
  if (dist != NULL)
    *dist = (target-wtm).dist2((source-wsm)*g);
}

template < class TSource, class TFilter >
bool
CFilterLearner<TSource,TFilter>::
FindBestMatchLocationSearchPoints( Point2 targetLoc, int currLevel,
				   const vector<Neigh*> & points, 
				   Point2 &bestLoc)
{
  assert(points.size() > 0);
  assert( m_filteredExamplePyramid.size() > 0 );
  
  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;

  // cheesy thing -- using # levels from first example pyramid. oh well, these had better all be the same, anyway
  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );

  int num = maxLevel - currLevel;
  int radius = (m_neighborhoodWidth-1)/2;
  if (targetLoc[0] < radius || 
      targetLoc[0] + radius+num >= (*img)[currLevel].width() ||
      targetLoc[1] < radius || 
      targetLoc[1] + radius+num >= (*img)[currLevel].height() )
    {
      bestLoc = FindBestMatchLocation(targetLoc,currLevel);
      return false;
    }

  Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt);
  Neigh * weights = NULL;
  if (m_useBias || m_useGain)
    weights = new Neigh(GetNeighborhood(targetLoc,currLevel,true,img,imgFilt));

  //  assert(m_eigenvectors.NCols() > 0);
  Neigh n;
  if (m_usePCA)
    n = m_eigenvectors*(nb-m_mean);
  else
    n = nb;

  // linear search
  m_sampler->eraseSamples();
  int best = 0;
  //  double dist = n.dist(*points[0]);
  //  bool foundsrc= false;
  for(int i=0;i<points.size();i++)
    {
      double d;

      if (!m_useBias && !m_useGain)
	{
	  d = n.dist2(*points[i]);

#ifndef NDEBUG
	  // check if GetNeighborhood and GNDist give the same result
	  bool isValid = true;
	  double d2 = GetNeighborhoodDist(targetLoc,points[i]->loc,
					  currLevel,isValid,
					  m_neighborhoodWidth);

	  assert(m_usePCA || !isValid || abs(d-d2) < 1e-8);
#endif
	}
      else
	if (m_useBias && !m_useGain)
	  optimalBias(n,*points[i],*weights,&d,NULL);
	else
	  if (m_useBias && m_useGain)
	    optimalBiasGain(n,*points[i],*weights,&d,NULL,NULL);
	  else
	    optimalGain(n,*points[i],*weights,&d,NULL);

      //      if (points[i]->loc == targetLoc)
      //	foundsrc = true;
      //      assert(points[i]->loc != targetLoc || d == 0);

      /*
	bool isValid;
	double dist2 = GetNeighborhoodDist( targetLoc, points[i]->loc,
	currLevel, maxLevel,
	isValid, m_neighborhoodWidth );
      */

      //      m_sampler->addElement(d, points[i]->loc);
      m_sampler->addElement(d, Point2(i,-1));
      /*
	
	if (d < dist)
	{
	dist = d;
	best = i;
	}
      */
    }

  //  assert(foundsrc);

  int index = m_sampler->UniformSample().second[0];

  bestLoc = points[index]->loc;

  double bias = 0, gain = 1;

  if (!m_useBias && !m_useGain)
    {
      return false;
    }
  else
    if (m_useBias && !m_useGain)
      optimalBias(n,*points[index],*weights,NULL,&bias);
    else
      if (m_useBias && m_useGain)
	optimalBiasGain(n,*points[index],*weights,NULL,&bias,&gain);
      else
	optimalGain(n,*points[index],*weights,NULL,&gain);
  
  if (weights != NULL)
    delete weights;

  for(int d=0;d<(*img)[currLevel].dim();d++)
    {
      TFilter src = m_filteredExamplePyramid[bestLoc.ImageID()][currLevel].
	Pixel(bestLoc[0],bestLoc[1],d);
      TFilter val = gain*src + bias;
      (*img)[currLevel].Pixel(targetLoc[0], targetLoc[1], d) = val;

      m_sourcesPyramid[currLevel].Pixel(targetLoc[0],targetLoc[1],0) = 
	bestLoc[0];
      m_sourcesPyramid[currLevel].Pixel(targetLoc[0],targetLoc[1],1) = 
	bestLoc[1];
    }

  return true;
  
  /*
  Neigh *match = points[best];

  assert(points[best]->loc[0] >= 0 && points[best]->loc[1] >= 0);
    
  return match->loc;*/
}

template <class TSource, class TFilter >
bool
CFilterLearner<TSource, TFilter>::
GetPredictions(Point2 targetLoc, int currLevel, bool insertRandom,
	       set<Point2> & candidates, bool causal, int inWidth)
{
  //  TFiltImage &feImage = m_filteredExamplePyramid[0][ currLevel ];

  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;
  Image<srcCoord> & sources = m_sourcesPyramid[currLevel];
  Image<GLubyte> &targetMask = m_destMaskPyramid[ currLevel ];

  const int width = inWidth < 0 ? m_neighborhoodWidth : inWidth;
  //  const int width = m_neighborhoodWidth;
  const int radius = (width-1)/2;

  const int exampleWidth = m_filteredExamplePyramid[0][currLevel].width();
  const int exampleHeight = m_filteredExamplePyramid[0][currLevel].height();

  bool hitValidN = false;

  //  bool causal = bool(m_pass == 0);

  // #### is this right?
  for(int iy = 0;iy <= radius || (!causal && iy < width);++iy)
    for(int ix = 0;ix < width && (ix < radius || iy < radius || !causal); 
	++ ix)
      {
	// skip center pixel
	if (iy == radius && ix == radius)
	  continue;

	int srcX = targetLoc[0]+ix - radius;
	int srcY = targetLoc[1]+iy - radius;

	if (!sources.inside(srcX,srcY))
	  {
	    //	    printf("\tsource is outside\n");
	    if (insertRandom)
	      candidates.insert(Point2(RandInt(exampleWidth),
				       RandInt(exampleHeight),
				       RandInt(m_filteredExamplePyramid.
					       size())));
	    continue;
	  }

	assert(targetMask.Pixel(srcX,srcY));

	int newX = sources.Pixel(srcX,srcY,0) + radius - ix;
	int newY = sources.Pixel(srcX,srcY,1) + radius - iy;
	int newImageID = 0; // ought to set to sources.Pixel(...,2)?
	
	if (newX >=radius && newY >= radius && newY+radius<exampleWidth &&
	    newY+radius<exampleHeight)
	  // can't compare distances at image boundary
	  {
	    hitValidN = true;
	    candidates.insert(Point2(newX,newY,newImageID));
	  }
	else
	  {
	    //	    printf("\tcandidate is outside\n");
	    //	    candidates.insert(Point2(RandInt(exampleWidth),
	    //				     RandInt(exampleHeight),0));
	  }
      }

  return hitValidN;
}

template <class TSource, class TFilter >
bool
CFilterLearner<TSource, TFilter>::
GetPredictionsTwoLevel(Point2 targetLoc, int currLevel, bool insertRandom,
		       set<Point2> & candidates, bool causal)
{
  Point2 prevLoc = targetLoc;
  prevLoc[0] /= 2;
  prevLoc[1] /= 2;
  set<Point2> prevCandidates;

  // crap. we want to use the smaller neighborhood for the coarser level of the pyramid... how to do this?
  // also, we probably should decouble the neighborhood size from the Ashikhmin candidate neighborhood size...

  bool prevHitValid = GetPredictions( prevLoc, currLevel+1, insertRandom, prevCandidates, false, m_neighborhoodWidth-2 ); //
  //  bool prevHitValid = GetPredictions( prevLoc, currLevel+1, insertRandom, prevCandidates, false );

  // foreach thing in prevCandidates, add its upsampled loc to candidates
  if( prevHitValid )
  {
    for(set<Point2>::iterator it=prevCandidates.begin();it != prevCandidates.end();++it)
    {
      Point2 prevLoc = *it;
      candidates.insert( Point2( 2*prevLoc.x(),   2*prevLoc.y(),   prevLoc.ImageID() ) );
      candidates.insert( Point2( 2*prevLoc.x()+1, 2*prevLoc.y(),   prevLoc.ImageID() ) );
      candidates.insert( Point2( 2*prevLoc.x(),   2*prevLoc.y()+1, prevLoc.ImageID() ) );
      candidates.insert( Point2( 2*prevLoc.x()+1, 2*prevLoc.y()+1, prevLoc.ImageID() ) );
    }
  }

  // TODO: have an option to not look at the current level for candidates (?)
  // (since they'll be considered anyway if coherenceEps > 0 )
  bool hitValid = GetPredictions( targetLoc, currLevel, insertRandom, candidates, causal );
  return hitValid || prevHitValid;
}

template <class TSource, class TFilter >
Point2
CFilterLearner<TSource, TFilter>::
FindBestMatchLocationAshikhmin( Point2 targetLoc, int currLevel, bool twoLevel)
{
  set<Point2> candidates;
  
  bool hitValidN;
  if( twoLevel )
    hitValidN = GetPredictionsTwoLevel(targetLoc,currLevel, true, candidates, m_pass==0);
  else
    hitValidN = GetPredictions(targetLoc,currLevel, true, candidates, m_pass==0);

  Point2 bestMatch(-1,-1);
  float bestDist = FLT_MAX;

  if (hitValidN)
    for(set<Point2>::iterator it=candidates.begin();it != candidates.end();++it)
      {
	//      printf("\ttesting candidate: %d %d\n",(*it)[0], (*it)[1]);
	
	bool isValid = false;
	float dist2 = GetNeighborhoodDist(targetLoc,*it,currLevel,isValid,
					  m_neighborhoodWidth, bestDist);
	
	//      printf("isValid = %d, dist = %f\n",isValid ? 1:0, dist2);
	
	if (isValid && dist2 < bestDist)
	  {
	    bestDist = dist2;
	    bestMatch = *it;
	  }
      }
  else
  //  assert(bestDist < FLT_MAX);
    {
  // fall back on brute force search
  if (bestDist >= FLT_MAX)
    {
      // reverting to brute force
      bestMatch = FindBestMatchLocation( targetLoc, currLevel );
    }
    }

  //  printf("best match is %d %d\n",bestMatch[0],bestMatch[1]);

  m_numPixelsSynthesized++;
  m_avgNumCandidates += candidates.size();

  return bestMatch;
}

template < class TSource, class TFilter >
Point2
CFilterLearner<TSource,TFilter>::
FindBestMatchLocation( Point2 targetLoc, int currLevel )
{
  assert( m_filteredExamplePyramid.size() > 0 );

  m_sampler->eraseSamples();

  for( int imageID = 0; imageID < m_filteredExamplePyramid.size(); imageID++ )
  {
    // search over src image, looking for similar neighborhoods
    int maxLevel = min( currLevel+m_numLevels-1, m_filteredExamplePyramid[imageID].NLevels()-1 );
    
    TFiltImage &feImage = m_filteredExamplePyramid[imageID][ currLevel ];
    
    for( int iy = 0; iy < feImage.height(); iy++ )
      for( int ix = 0; ix < feImage.width(); ix++ )
      {
	bool isValid = false;
	Point2 srcLoc( ix, iy, imageID );

	double dist = GetNeighborhoodDist( targetLoc, srcLoc, currLevel, 
					   isValid, m_neighborhoodWidth,
					   m_sampler->threshold());
	
	if (isValid && dist >= 0)
	  m_sampler->addElement(dist, srcLoc );
      }
    /*
      if( isValid && dist >= 0 && (bestDist < 0 || dist < bestDist ) )
      {
      bestDist = dist;
      bestLoc = Point2( ix, iy );
      }
      }
    */
    //  assert(m_useRandom || bestDist == -1 ||(targetLoc.x() == bestLoc.x() && targetLoc.y() == bestLoc.y()));
    
    //  assert(m_useRandom ||
    //	     !(targetLoc.x() > 5 && targetLoc.y() > 5 && targetLoc.x() < 59 && targetLoc.y() < 59) ||
    //		 bestDist == 0);
    
    //printf("bestDist = %f\n",bestDist);
    // return bestLoc;
  }
  
  if(m_sampler->size() == 0)
    return Point2(-1,-1);

   //  const pair<float,Point2> result = sampler->GaussianSample();

 // printf("pt = (%d,%d; %d), dist = %f, src=(%d,%d)\n",targetLoc.x(),targetLoc.y(),
//	  currLevel, result.first,result.second.x(), result.second.y());
//  assert(result.first == 0);

  //if (!(result.second.x() == targetLoc.x() && result.second.y() == targetLoc.y()))
	//  sampler.print();
  //assert(result.second.x() == targetLoc.x() && result.second.y() == targetLoc.y());

// if (result.first == 0)
//	  return targetLoc;

   // m_sample->print();
  
  return m_sampler->UniformSample().second;
}

template < class TSource, class TFilter >
Point2
CFilterLearner<TSource,TFilter>::
FindBestMatchLocation( Point2 targetLoc, int currLevel, 
		       CSearchEnvironment *searchEnvironment )
{
  if( searchEnvironment == NULL || 
      currLevel >= m_filteredExamplePyramid[0].NLevels() )
    return FindBestMatchLocation( targetLoc, currLevel );

  assert( searchEnvironment != NULL );
  assert( m_filteredExamplePyramid.size() > 0 );

  const int nw = m_neighborhoodWidth;
  const int radius = (nw-1)/2;

  Pyramid<TFiltImage> * img = &m_filteredPyramid;
  Pyramid<TSrcImage> * imgFilt = m_useSourceImagesNow ? &m_sourcePyramid : NULL;

  // cheesy thing -- using # levels from first example pyramid. oh well, these had better all be the same, anyway
  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );

  int num = maxLevel - currLevel;
  if (targetLoc[0] < radius || 
      targetLoc[0] +radius+num >= (*img)[currLevel].width() ||
      targetLoc[1] < radius || 
      targetLoc[1] +radius+num >= (*img)[currLevel].height() )
    {
      //      if( useBruteForceBoundary )
	return FindBestMatchLocation(targetLoc,currLevel);
    }
    
  Neigh nb = GetNeighborhood(targetLoc,currLevel,false,img,imgFilt );

  Neigh n;
  if (m_usePCA)
    n = m_eigenvectors*(nb-m_mean);
  else
    n = nb;

  return searchEnvironment->FindBestMatchLocation( n );
}

template < class T>
inline double
PixelDistSqr(const Image<T> & img1, int x1, int y1,
	     const Image<T> & img2, int x2, int y2)
{
  assert(img1.dim() == img2.dim() );
  double dist = 0;
  for(int i=0;i<img1.dim();i++)
    {
      double dp = double(img1.Pixel(x1,y1,i) - img2.Pixel(x2,y2,i))/
	img1.maxVal();
      dist += ANN_POW(dp);
      //dist += dp*dp;
    }

  //  printf("PixelDist = %f ",dist);

  return dist;
}

template < class TSource, class TFilter >
double
CFilterLearner<TSource,TFilter>::
GetNeighborhoodDist( Point2 targetLoc, Point2 srcLoc, int currLevel, 
		     bool &isValid, int neighborhoodWidth, 
		     double distThresh)
{
  bool hitValidPixel = false;

  double dist = 0;

  isValid = true;

  m_numComparisons ++;

  int maxLevel = min( currLevel+m_numLevels-1, 
		      m_filteredExamplePyramid[0].NLevels()-1 );
  
  assert(maxLevel <= 20);

  Point2 tar = targetLoc;
  Point2 src = srcLoc;
  double levelWeight = 1;
  for(int level = currLevel, nw = neighborhoodWidth; 
      level <= maxLevel && nw >= 3 && levelWeight != 0; 
      level ++, 
	tar[0]/=2, tar[1] /=2,
	src[0]/=2, src[1]/=2,
	nw = (nw>>1)+1,
	levelWeight *= m_levelWeighting*m_levelWeighting)
    {

      double d2 = GetNeighborhoodDistForLevel( tar, src, level, 
					       isValid, hitValidPixel,
					       nw, bool(level == currLevel));

      if (!isValid)
	return -1;

      if (d2 >= 0 && !isnan(d2))
	{
	  d2 *= levelWeight;
	  dist += d2 * levelWeight;

	  if (distThresh > 0 && dist > distThresh)
	    {
	      isValid = false;
	      return -1;
	    }
	}
      else
	printf( "warning: coarser level gives invalid score %d\n",d2);
    }

  /*
  if( m_useSourceImagesNow && m_onePixelSource)
    {
      //	TSource maxVS = targetProtoImage->maxVal();
	
	int imageID = srcLoc.ImageID();
	
	double pdist = 
	  PixelDistSqr<TSource>(m_sourceExamplePyramid[imageID][currLevel], 
				srcLoc.x(), srcLoc.y(), 
				m_sourcePyramid[currLevel],
				targetLoc.x(), targetLoc.y());

	double fac = m_sourceFacNow * m_sourceFacNow;
	totalWeight += m_sourceFacNow;
	dist += fac * pdist;
	
	//		  validPixels++;
	hitValidPixel = true;
	
	assert(!isnan(dist) && !isnan(totalWeight));
    }
  */

  isValid = isValid && hitValidPixel;
  
  //  assert( dist >= 0 );
  return dist;
}


template < class TSource, class TFilter >
double
CFilterLearner<TSource,TFilter>::
GetNeighborhoodDistForLevel( Point2 targetLoc, Point2 srcLoc,
			     int currLevel, bool &isValid, 
			     bool & hitValidPixel,
			     int neighborhoodWidth, bool skipCenterPixel)
{
  assert( m_filteredExamplePyramid.size() > 0 );
  int imageID = srcLoc.ImageID();
  assert( imageID >= 0 );

  hitValidPixel = false;

  TFiltImage &feImage = m_filteredExamplePyramid[imageID][ currLevel ];
  TFiltImage &targetImage = m_filteredPyramid[ currLevel ];
  Image<GLubyte> &targetMask = m_destMaskPyramid[ currLevel ];
  
  assert( targetMask.width() == targetImage.width() );
  assert( targetMask.height() == targetImage.height() );
  
  // the filtered images
  TSrcImage *srcProtoImage = NULL;
  TSrcImage *targetProtoImage = NULL;
  
  if (m_useSourceImagesNow)
  {
    srcProtoImage = &m_sourceExamplePyramid[imageID][ currLevel ];
    targetProtoImage = &m_sourcePyramid[currLevel];
  }
  
  double dist = 0;
  double totalWeight = 0;
  int offset = neighborhoodWidth/2;
  
  TFilter maxVal = targetImage.maxVal();
  
  for( int iiy = 0; iiy < neighborhoodWidth; iiy++ )
  {
    int srcY = srcLoc.y()+iiy-offset;
    int targetY = targetLoc.y()+iiy-offset;
    
    for( int iix = 0; iix < neighborhoodWidth; iix++ )
    {
      int targetX = targetLoc.x()+iix-offset;
      int srcX = srcLoc.x()+iix-offset;
      
      double weight = m_kernel->get(neighborhoodWidth,iix)*
	m_kernel->get(neighborhoodWidth,iiy);
      
      if (isnan(weight))
	printf("weight = %f, m_sourceFac = %f,iix = %d,iiy = %d,k[iix]=%f,k[iiy]=%f\n",weight,m_sourceFacNow,iix,iiy,m_kernel->get(neighborhoodWidth,iix),m_kernel->get(neighborhoodWidth,iiy));
      
      assert(weight > 0);
      
      // ignore invalid pixels
      if (!targetImage.inside(targetX, targetY))
	continue;

      // for target, ignore pixels that haven't been created, and ignore
      // central pixel
      
      //      if (targetX == 0 && targetY == 0 && !skipCenterPixel)
      //	printf("A");

      if (targetMask.Pixel(targetX, targetY) && 
	  !(2*iix+1 == neighborhoodWidth && 2*iiy+1 == neighborhoodWidth && 
	    skipCenterPixel))
      {
	if( feImage.inside(srcX, srcY) )
	{
	  //	  if (targetX == 0 && targetY == 0 && !skipCenterPixel)
	  //	    printf("B\n");

	  double pdist = 
	    PixelDistSqr<TFilter>( feImage, srcX, srcY,
				   targetImage, targetX, targetY);
	  dist += weight*weight * pdist;

	  totalWeight += weight;
	  //		  validPixels++;
	  hitValidPixel = true;

	  assert(!isnan(dist) && !isnan(totalWeight));
	}
	else // really penalize stuff that goes off the edge
	{
	  isValid = false;
	  return -1;

	  //	  dist += weight * 300.0;
	  //	  totalWeight += weight;
	}
      }
      //      else
      //      if (targetX == 0 && targetY == 0 && !skipCenterPixel)
      //	printf("\n");
      
      if( m_useSourceImagesNow && 
	  (!m_onePixelSource || 
	   (skipCenterPixel &&
	    2*iix+1 == neighborhoodWidth && 2*iiy+1 == neighborhoodWidth)))

      {	
	//	TSource maxVS = targetProtoImage->maxVal();
	
	bool protoPixelValid =  feImage.inside(srcX, srcY);
	//		targetImage.inside(targetX, targetY);
	
	assert(protoPixelValid || !m_onePixelSource);

	if( protoPixelValid )
	{
	  double pdist = 
	    PixelDistSqr<TSource>(*srcProtoImage, srcX, 
				  srcY, *targetProtoImage,
				  targetX, targetY);

	  double fac = m_sourceFacNow * m_sourceFacNow;
	  if (!m_onePixelSource)
	    fac *= weight * weight;

	  if (!m_onePixelSource)
	    totalWeight += m_sourceFacNow * weight;
	  else
	    totalWeight += m_sourceFacNow;

	  dist += fac * pdist;

	  //		  validPixels++;
	  hitValidPixel = true;

	  assert(!isnan(dist) && !isnan(totalWeight));
	}
	else // really penalize stuff that goes off the edge
	{
	  isValid = false;
	  return -1;


	  //  dist += m_sourceFac * m_sourceFac * weight * weight * 3.0;
	  //  totalWeight += m_sourceFac * weight;
	}
      }
      
    }
  }
  
  //  printf("dist = %f, totalWeight = %f",dist,totalWeight);

  double d1 = dist;  // for debugging

  if( totalWeight > 0 )
    dist /= (totalWeight*totalWeight);

  assert(!isnan(dist));
  assert(dist >= 0);

  //  printf(", final dist = %f\n",dist);

  //  isValid = hitValidPixel;
  isValid = true;

  /*
  if (hitValidPixel && dist < 0)
    {
      printf("Warning: valid = 1 and dist < 0 (totalWeight = %f, d1 = %f, targetLoc = %d, %d, srcLoc = %d, %d)\n", totalWeight,d1,targetLoc[0],targetLoc[1],srcLoc[0],srcLoc[1]);
      }*/

  return dist;
}

template < class TSource, class TFilter >
Point2
CFilterLearner<TSource,TFilter>::
FindBestMatchLocation2( Point2 targetLoc, int level )
{
  // search over src image, looking for similar neighborhoods
  int maxLevel = min( level+m_numLevels-1, 
		      m_filteredExamplePyramid.NLevels()-1 );

  m_sampler->eraseSamples();

  float bestDist = -1;
  Point2 bestLoc( -1, -1 );
  int width = m_filteredExamplePyramid[ maxLevel ].width();
  int height = m_filteredExamplePyramid[ maxLevel ].height();
  for( int iy = 0; iy < height; iy++ )
    for( int ix = 0; ix < width; ix++ )
      {
	// find best of the neighbors
	bool isValid = false;
	FindBestMatchLocParent( targetLoc, Point2( ix, iy ), 
				level, maxLevel, -1,
				bestDist, bestLoc, isValid );
      }

  return bestLoc;
}

// note that targetLoc is in minLevel's coord system, whereas srcLoc is in maxLevel's (which is the current level)
template < class TSource, class TFilter >
void
CFilterLearner<TSource,TFilter>::
FindBestMatchLocParent( Point2 targetLoc, Point2 srcLoc,
			int minLevel, int maxLevel, float cumulDist,
			float &bestDist, Point2 &bestLoc, bool &isValid )
{
  assert( minLevel <= maxLevel );

  float divisor = pow2( maxLevel - minLevel );
  Point2 newTargetLoc = Point2( targetLoc[0] / divisor, targetLoc[1] / divisor );

  bool thisValid = false;
  float dist = GetNeighborhoodDistForLevel( newTargetLoc, 
					    srcLoc,
					    maxLevel, thisValid );
  isValid = isValid || thisValid;

  if( cumulDist >= 0 && dist >= 0 )
    dist += cumulDist;
  else if( dist < 0 )
    dist = cumulDist;

  if( minLevel == maxLevel ) // leaf node -- we've got the final score
    {
      // dist is the final score for this pixel
      if( isValid && dist >= 0 && (bestDist < 0 || dist <= bestDist ) )
	{
	  bestDist = dist;
	  bestLoc = srcLoc;
	}
    }
  else
    {
      if( dist >= 0 )
	dist *= m_levelWeighting;

      FindBestMatchLocParent( targetLoc, Point2( 2*srcLoc[0],   2*srcLoc[1] ),
			      minLevel, maxLevel-1, dist, bestDist, bestLoc, isValid );
      FindBestMatchLocParent( targetLoc, Point2( 2*srcLoc[0], 2*srcLoc[1]+1 ),
			      minLevel, maxLevel-1, dist, bestDist, bestLoc, isValid );
      FindBestMatchLocParent( targetLoc, Point2( 2*srcLoc[0]+1, 2*srcLoc[1] ),
			      minLevel, maxLevel-1, dist, bestDist, bestLoc, isValid );
      FindBestMatchLocParent( targetLoc, Point2( 2*srcLoc[0]+1, 2*srcLoc[1]+1 ),
			      minLevel, maxLevel-1, dist, bestDist, bestLoc, isValid );
    }
}

extern int mainWindow;
void display( void );

template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::SignalRedraw( void )
{
  if (m_redisplay)
    {
      glutSetWindow( mainWindow );
      display();
    }
}

template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::SignalUpdateUI( void )
{
}


template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
savePointsToFile(const vector<Neigh*> & points,char *filename1,
		 char *filename2)
{
  vector<Neigh*>::const_iterator it;
  FILE * fp = fopen(filename1,"wt");
  
  if (fp == NULL)
    {
      printf("Unable to open output file %s\n",filename1,filename2);

      return;
    }

  printf("Writing points to \"%s\" and \"%s\".\n",filename1,filename2);

  //  for(vector<Neigh*>::const_iterator it = points.begin();it != points.end();
  for(it = points.begin();it != points.end();
      ++it)
    {
      (*it)->printFloat(fp);
      fprintf(fp,"\n");
    }
      
  fclose(fp);

  fp = fopen(filename2,"wt");
  if (fp == NULL)
    {
      printf("Unable to open output filename %s\n",filename2);
      
      return;
    }

  ColorImage & outImg = filteredExamplePyramid().FineImage();

  //  for(vector<Neigh*>::const_iterator it = points.begin();it != points.end(); ++it)
  for(it = points.begin();it != points.end(); ++it)
    {
      Point2 loc = (*it)->loc;
      for(int d=0;d<outImg.dim();d++)
	fprintf(fp,"%f ",double(outImg.Pixel(loc[0],loc[1],d)));
      fprintf(fp,"\n");
    }

  fclose(fp);

  printf("Done.\n");
}

template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
SetImages(vector< TFiltImage * > filteredExampleImages, int levels)
{
  assert( filteredExampleImages.size() == 1 );
  assert(levels > 0);

  m_useSourceImages = false;
  m_useFilterModeMask = false;

  if (m_destImage != NULL)
    delete m_destImage;
  
  int width = filteredExampleImages[0]->width();
  int height = filteredExampleImages[0]->height();

  m_destImage = new TFiltImage(width,height,filteredExampleImages[0]->dim(),0);
  if (m_destImage == NULL)
    {
      printf("error allocating dest image\n");
      exit(1);
    }

  m_filteredExamplePyramid.erase( m_filteredExamplePyramid.begin(), m_filteredExamplePyramid.end() );
  Pyramid<TFiltImage> filteredExamplePyramid;
  //debug
#ifdef STEERDEBUG
  printf("calling makePyramid for filteredExamplePyramid\n");
#endif
  filteredExamplePyramid.makePyramid(levels, filteredExampleImages[0],
				     m_filteredPyramidType);
  m_filteredExamplePyramid.push_back( filteredExamplePyramid );
  
  //debug
#ifdef STEERDEBUG
  printf("calling makePyramid for m_filteredPyramid\n");
#endif
  m_filteredPyramid.makePyramid(levels, m_destImage, m_filteredPyramidType);
  //debug
#ifdef STEERDEBUG
  printf("calling makePyramid for m_destMaskPyramid\n");
#endif
  m_destMaskPyramid.makePyramid( levels, width, height, 1, GAUSSIAN_PYRAMID);
  m_destMaskPyramid.clear();
  //debug
#ifdef STEERDEBUG
  printf("calling makePyramid for m_sourcesPyramid\n");
#endif
  m_sourcesPyramid.makePyramid(levels, width, height, 3, SOURCES_PYRAMID);
  for(int level=0;level<m_sourcesPyramid.NLevels();level++)
    m_sourcesPyramid[level].set(-1);

  m_flags.make(levels, width, height);
  
  assert( m_destMaskPyramid.NLevels() > 0 );
}

template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
SetImages( vector< TSrcImage * > sourceExampleImages,
	   vector< TFiltImage * > filteredExampleImages,
	   TSrcImage * sourceImage, int levels, 
	   TFiltImage * modeMask, TFiltImage * targetModeMask)
{
  int index;

  assert(levels > 0);
  assert(sourceImage->width() >=0 && sourceImage->height() >= 0);
  assert(sourceExampleImages.size() == filteredExampleImages.size());

  int numExamples = sourceExampleImages.size();
  for( index = 0; index < numExamples; index++ )
  {
    assert(sourceExampleImages[index]->width() == filteredExampleImages[index]->width() &&
	   sourceExampleImages[index]->height() == filteredExampleImages[index]->height());
    
    assert(filteredExampleImages[index]->width() >= 0 && 
	   filteredExampleImages[index]->height() >= 0);
  }

  // ensure all filtered images have the same dimension:
  int filterDim = filteredExampleImages[0]->dim();
  for( index = 1; index < numExamples; index++ )
    assert( filteredExampleImages[index]->dim() == filterDim );
	    
  m_useSourceImages = true;
  m_useFilterModeMask = bool(modeMask != NULL);
  m_useTargetModeMask = bool(targetModeMask != NULL);

  if (m_destImage != NULL)
    delete m_destImage;

  m_destImage = new TSrcImage(sourceImage->width(),sourceImage->height(),
			      filterDim,0);
  if (m_destImage == NULL)
    {
      printf("error allocating dest image\n");
      exit(1);
    }

  m_filteredExamplePyramid.erase( m_filteredExamplePyramid.begin(),
				  m_filteredExamplePyramid.end() );
  m_sourceExamplePyramid.erase( m_sourceExamplePyramid.begin(),
				m_sourceExamplePyramid.end() );
  for( index = 0; index < numExamples; index++ )
  {
    Pyramid<TFiltImage> filteredExamplePyramid;
#ifdef STEERDEBUG
    //debug
    printf("Calling makePyramid for filteredExamplePyramid\n");
#endif
    filteredExamplePyramid.makePyramid(levels, filteredExampleImages[index],
				       m_filteredPyramidType);
    m_filteredExamplePyramid.push_back( filteredExamplePyramid );

    Pyramid<TSrcImage> sourceExamplePyramid;
#ifdef STEERDEBUG
    //debug
    printf("Calling makePyramid for sourceExamplePyramid\n");
#endif
    sourceExamplePyramid.makePyramid(levels, sourceExampleImages[index],
				     m_sourcePyramidType);
    m_sourceExamplePyramid.push_back( sourceExamplePyramid );
  }
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_filteredPyramid\n");
#endif
  m_filteredPyramid.makePyramid(levels, m_destImage, m_filteredPyramidType);  
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_sourcePyramid\n");
#endif
  m_sourcePyramid.makePyramid(levels, sourceImage, m_sourcePyramidType);  
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_destMaskPyramid\n");
#endif
  m_destMaskPyramid.makePyramid( m_filteredPyramid.NLevels(),
				 m_filteredPyramid.FineImage().width(), 
				 m_filteredPyramid.FineImage().height(),
				 1,
				 GAUSSIAN_PYRAMID);  
  m_destMaskPyramid.clear();
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_sourcesPyramid\n");
#endif
  m_sourcesPyramid.makePyramid(m_filteredPyramid.NLevels(), 
			       m_filteredPyramid.FineImage().width(), 
			       m_filteredPyramid.FineImage().height(),3,
			       SOURCES_PYRAMID);

  // TODO: feh, this may be a problem with multiple input pairs:
  m_flags.make(m_filteredExamplePyramid[0].NLevels(), 
	       m_filteredExamplePyramid[0].FineImage().width(), 
	       m_filteredExamplePyramid[0].FineImage().height());
  for(int level=0;level<m_sourcesPyramid.NLevels();level++)
    m_sourcesPyramid[level].set(-1);

  if (m_useTargetModeMask)
    m_modeMaskPyramid.makePyramid(levels, targetModeMask, GAUSSIAN_PYRAMID);

  if (m_useFilterModeMask)
    {
      assert(modeMask != NULL);

      m_exampleModeMaskPyramid.makePyramid(levels, modeMask, 
					   GAUSSIAN_PYRAMID);
      if (!m_useTargetModeMask)
	{
	  m_modeMaskPyramid.makePyramid( m_filteredPyramid.NLevels(),
					 m_filteredPyramid.FineImage().width(),
					 m_filteredPyramid.FineImage().height(),
					 1,
					 GAUSSIAN_PYRAMID);
	  m_modeMaskPyramid.clear();
	}
    }
}

/*
template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
SetImages( vector< TSrcImage * > sourceExampleImages,
	   vector< TFiltImage * > filteredExampleImages,
	   TSrcImage * sourceImage, int levels, 
	   TFiltImage * modeMask, TFiltImage * targetModeMask,
	   PFILTER pf)
{
  int index;

  assert(levels > 0);
  assert(sourceImage->width() >=0 && sourceImage->height() >= 0);
  assert( sourceExampleImages.size() == filteredExampleImages.size() );

  int numExamples = sourceExampleImages.size();
  for( index = 0; index < numExamples; index++ )
  {
    assert(sourceExampleImages[index]->width() == filteredExampleImages[index]->width() &&
	   sourceExampleImages[index]->height() == filteredExampleImages[index]->height());
    
    assert(filteredExampleImages[index]->width() >= 0 && 
	   filteredExampleImages[index]->height() >= 0);
  }

  // ensure all filtered images have the same dimension:
  int filterDim = filteredExampleImages[0]->dim();
  for( index = 1; index < numExamples; index++ )
    assert( filteredExampleImages[index]->dim() == filterDim );
	    
  m_useSourceImages = true;
  m_useFilterModeMask = bool(modeMask != NULL);
  m_useTargetModeMask = bool(targetModeMask != NULL);

  if (m_destImage != NULL)
    delete m_destImage;
#ifdef STEERDEBUG
  //debug
  printf("Inside FilterLearnerInline:SetImages for Steerable\n");
#endif
  m_destImage = new TSrcImage(sourceImage->width(),sourceImage->height(),
			      filterDim,0);
  if (m_destImage == NULL)
    {
      printf("error allocating dest image\n");
      exit(1);
    }

  m_filteredExamplePyramid.erase( m_filteredExamplePyramid.begin(),
				  m_filteredExamplePyramid.end() );
  m_sourceExamplePyramid.erase( m_sourceExamplePyramid.begin(),
				m_sourceExamplePyramid.end() );
#ifdef STEERDEBUG
  //debug
  printf("NumExamples %d\n",numExamples);
#endif
  for( index = 0; index < numExamples; index++ )
  {
    Pyramid<TFiltImage> filteredExamplePyramid;
#ifdef STEERDEBUG
    //debug
    printf("Calling makePyramid for filteredExamplePyramid\n");
#endif
    filteredExamplePyramid.makePyramid(levels, filteredExampleImages[index],
				       m_filteredPyramidType);
    m_filteredExamplePyramid.push_back( filteredExamplePyramid );

    Pyramid<TSrcImage> sourceExamplePyramid;
#ifdef STEERDEBUG
    //debug
    printf("Calling makePyramid for sourceExamplePyramid type %d\n", m_sourcePyramidType);
#endif
    sourceExamplePyramid.makePyramid(levels, sourceExampleImages[index],
				     m_sourcePyramidType, pf);
    m_sourceExamplePyramid.push_back( sourceExamplePyramid );
  }
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_filteredPyramid\n");
#endif
  m_filteredPyramid.makePyramid(levels, m_destImage, m_filteredPyramidType);  

  //if steerable filter convert the sourceImage to graylevel
  if (m_sourcePyramidType == STEERABLE_PYRAMID) {
    if (sourceImage->dim()==3) {
#ifdef STEERDEBUG
      //debug
      printf("Converting sources to GrayScale before calling steerable pyramid function\n");
#endif
      TSrcImage sourceImageGrayScale(sourceImage->width(),sourceImage->height(),1,0);	
      sourceImage->RGBtoGrayScale(sourceImageGrayScale);
#ifdef STEERDEBUG
    //debug
      printf("Calling makePyramid for steerable with source image dim = 3\n");
#endif
      m_sourcePyramid.makePyramid(levels, (TSrcImage *)&sourceImageGrayScale, m_sourcePyramidType,pf);
    } else {
#ifdef STEERDEBUG
      //debug
      printf("Calling makePyramid for m_sourcePyramid steerable\n");
#endif
      m_sourcePyramid.makePyramid(levels, sourceImage, m_sourcePyramidType,pf);  
    }
  } else {
    m_sourcePyramid.makePyramid(levels, sourceImage, m_sourcePyramidType);  
  }
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_destMaskPyramid\n");
#endif
  m_destMaskPyramid.makePyramid( m_filteredPyramid.NLevels(),
				 m_filteredPyramid.FineImage().width(), 
				 m_filteredPyramid.FineImage().height(),
				 1,
				 GAUSSIAN_PYRAMID);  
  m_destMaskPyramid.clear();
#ifdef STEERDEBUG
  //debug
  printf("Calling makePyramid for m_sourcesPyramid\n");
#endif
  m_sourcesPyramid.makePyramid(m_filteredPyramid.NLevels(), 
			       m_filteredPyramid.FineImage().width(), 
			       m_filteredPyramid.FineImage().height(),3,
			       SOURCES_PYRAMID);

  // TODO: feh, this may be a problem with multiple input pairs:
  m_flags.make(m_filteredExamplePyramid[0].NLevels(), 
	       m_filteredExamplePyramid[0].FineImage().width(), 
	       m_filteredExamplePyramid[0].FineImage().height());
  for(int level=0;level<m_sourcesPyramid.NLevels();level++)
    m_sourcesPyramid[level].set(-1);

  if (m_useTargetModeMask)
    m_modeMaskPyramid.makePyramid(levels, targetModeMask, GAUSSIAN_PYRAMID);

  if (m_useFilterModeMask)
    {
      assert(modeMask != NULL);

      m_exampleModeMaskPyramid.makePyramid(levels, modeMask, 
					   GAUSSIAN_PYRAMID);
      if (!m_useTargetModeMask){
	m_modeMaskPyramid.makePyramid( m_filteredPyramid.NLevels(),
				       m_filteredPyramid.FineImage().width(),
				       m_filteredPyramid.FineImage().height(),
				       1,
				       GAUSSIAN_PYRAMID);
	m_modeMaskPyramid.clear();
      }
    }

  //debug 
  for (int i=0;i<m_sourcesPyramid.NLevels();i++) {
#ifdef STEERDEBUG
    printf("level %d\n",i);
    printf("sources.w=%d, source.w=%d\n",
	   m_sourcesPyramid[i].width(),m_sourcePyramid[i].width());
    printf("sources.h=%d, source.h=%d\n",
	   m_sourcesPyramid[i].height(),m_sourcePyramid[i].height());
#endif
    assert(m_sourcesPyramid[i].width()==m_sourcePyramid[i].width());
    assert(m_sourcesPyramid[i].height()==m_sourcePyramid[i].height());
  }
}
*/


template < class TSource, class TFilter >
void CFilterLearner<TSource,TFilter>::
AddExamplePair( TSrcImage * sourceExampleImage, 
		TFiltImage * filteredExampleImage )
{
  int levels = m_filteredPyramid.NLevels();
  assert(levels > 0);

  assert(sourceExampleImage->width() == filteredExampleImage->width() &&
	 sourceExampleImage->height() == filteredExampleImage->height());
		   
  assert(filteredExampleImage->width() >= 0 && 
	 filteredExampleImage->height() >= 0);

  if( sourceExampleImage->width() != filteredExampleImage->width() ||
      sourceExampleImage->height() != filteredExampleImage->height() )
  {
    fprintf( stderr, "warning: Adding example pair with different sizes -- ignored\n" );
    return;
  }
  
  m_useSourceImages = true; // ???

  Pyramid<TFiltImage> filteredExamplePyramid;
  filteredExamplePyramid.makePyramid(levels, filteredExampleImage,
				     m_filteredPyramidType);
  m_filteredExamplePyramid.push_back( filteredExamplePyramid );


  Pyramid<TSrcImage> sourceExamplePyramid;
  sourceExamplePyramid.makePyramid(levels, sourceExampleImage,
				   m_sourcePyramidType);
  m_sourceExamplePyramid.push_back( sourceExamplePyramid );

  /*
  // TODO: feh, this may be a problem with multiple input pairs:
  m_flags.make(m_filteredExamplePyramid[0].NLevels(), 
	       m_filteredExamplePyramid[0].FineImage().width(), 
	       m_filteredExamplePyramid[0].FineImage().height());

  for(int level=0;level<m_sourcesPyramid.NLevels();level++)
    m_sourcesPyramid[level].set(-1);
  */
}


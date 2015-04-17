// FilterLearner.h : header file
//

#if !defined(FILTER_LEARNER_H)
#define FILTER_LEARNER_H

#include <vector>
#include <set>
#include "compat.h"
//#include "TSVQ.h"
#include "TSVQR.h"
#include "Image.h"
#include "CVector.h" // includes def'n of Neigh
#include "CMatrix.h"
#include "sampler.h"
#include "Pyramid.h"
#include "FlagPyramid.h"
extern "C" {
//#include "spyramid.h"
}

/////////////////////////////////////////////////////////////////////////////
// CLearnTextureDlg dialog

class CSearchEnvironment;

template < class TSource, class TFilter>
class CFilterLearner 
{
  // Construction
public:
  typedef Image<TSource> TSrcImage;
  typedef Image<TFilter> TFiltImage;

  typedef GLshort srcCoord;

  CFilterLearner( void );
  ~CFilterLearner();

  void SetImages(vector< TFiltImage * > filteredExampleImages, int levels);
  void SetImages(vector< TSrcImage * > sourceExampleImages, 
		 vector< TFiltImage * > filteredExampleImages,
		 TSrcImage * sourceImage, int levels, 
		 TFiltImage * modeMask, TFiltImage * targetModeMask);
/*  void SetImages(vector< TSrcImage * > sourceExampleImages, 
		 vector< TFiltImage * > filteredExampleImages,
		 TSrcImage * sourceImage, int levels, 
		 TFiltImage * modeMask, TFiltImage * targetModeMask,
		 PFILTER pf);*/

  void AddExamplePair( TSrcImage * sourceExampleImage, 
		      TFiltImage * filteredExampleImage );
//  void AddExamplePair( TSrcImage * sourceExampleImage, 
//		      TFiltImage * filteredExampleImage, PFILTER pf );

  // other routines to do actual work
  void Synthesize( void );
  void SynthesizePyramidLevel( int level );
  void GatherTrainingData(int level,vector<Neigh*> & points,
			  vector<CVector<double> *> * pixels=NULL,
			  bool gatherMMdata = false);
#ifdef LAPACK
  static CMatrix<double> PCA(vector<Neigh*> & points, Neigh & mean,
			     float minRatio=.001);
#endif
  Neigh GetNeighborhood(Point2 loc, int currLevel, bool computeWeightVector,
			Pyramid<TFiltImage> * img, 
			Pyramid<TSrcImage> * imgProto = NULL);  
  Point2 FindBestMatchLocation( Point2 targetLoc, int level );
  Point2 FindBestMatchLocation2( Point2 targetLoc, int level );

  // these 2 methods do TSVQ or ANN search, depending on what type of
  // search environment is passed in
  Point2 FindBestMatchLocationAshikhmin(Point2 targetLoc, int level, bool twoLevel = false);

  Point2 FindBestMatchLocation( Point2 targetLoc, int level, 
				CSearchEnvironment *searchEnvironment );
  Point2 FindBestMatchLocationHeuristic( Point2 targetLoc, int level, 
					 CSearchEnvironment *tsvqEnvironment );
  
  bool FindBestMatchLocationSearchPoints( Point2 targetLoc, int level,
					  const vector<Neigh*> & points,
					  Point2 & bestLoc);

  void FindBestMatchLocParent( Point2 targetLoc, Point2 srcLoc,
			       int minLevel, int maxLevel, float cumulDist,
			       float &bestDist, Point2 &bestLoc, 
			       bool &isValid );
  bool ApplyMLP( MLP * mlp, Point2 targetLoc, int level, Point2 & bestLoc);
  bool ApplyTSVQR( TSVQR * mlp, Point2 targetLoc, int level, Point2 & bestLoc);

  bool GetPredictions(Point2 targetLoc, int currLevel, bool insertRandom,
		      set<Point2> & candidates, bool causal, int inWidth = -1);
  bool GetPredictionsTwoLevel(Point2 targetLoc, int currLevel, bool insertRandom,
			      set<Point2> & candidates, bool causal);
  
  Point2 ApplyPrediction(Point2 targetLoc, int currLevel, Point2 bestLoc,
			 float coherenceFac);

  double GetNeighborhoodDist( Point2 targetLoc, Point2 srcLoc, int currLevel,
			      bool &isValid, int neighborhoodWidth,
			      double distThresh = -1);
  double GetNeighborhoodDistForLevel( Point2 targetLoc, Point2 srcLoc,
				      int currLevel, bool &isValid,
				      bool &hitValidPixel,
				      int neighborhoodWidth, 
				      bool skipCenterPixel);

  void savePointsToFile(const vector<Neigh*> & points,
			char *filenameIn="neigh.txt",
			char *filenameOut="pixel.txt");
  
  void SignalRedraw( void );
  void SignalUpdateUI( void );

  void optimalBias(const Neigh & target,const Neigh & source,
		   const Neigh & weights,double * dist, double * bias);

  void optimalGain(const Neigh &target,const Neigh & source, 
		   const Neigh &weights, double * dist, double * gain);
  
  void optimalBiasGain(const Neigh & target,const Neigh & source,
		       const Neigh & weights, double * dist, double * bias,
		       double * gain);

  //  static float PixelDist(const TSrcImage & img1, int x1, int y1,
  //			 const TSrcImage & img2, int x2, int y2);

  /////////// ACCESSORS ////////////////////////////

  int NumExamplePairs( void ) { return m_filteredExamplePyramid.size(); }
  Pyramid<TSrcImage> & sourceExamplePyramid( int num = 0 ) { return m_sourceExamplePyramid[num]; }
  const Pyramid<TSrcImage> & sourceExamplePyramid( int num = 0 ) const { return m_sourceExamplePyramid[num]; }

  Pyramid<TSrcImage> & sourcePyramid() { return m_sourcePyramid; }
  const Pyramid<TSrcImage> & sourcePyramid() const { return m_sourcePyramid; }

  Pyramid<TFiltImage> & filteredExamplePyramid( int num = 0 ) { return m_filteredExamplePyramid[num]; }
  const Pyramid<TFiltImage> & filteredExamplePyramid( int num = 0 ) const { return m_filteredExamplePyramid[num]; }

  Pyramid<TFiltImage> & filteredPyramid() { return m_filteredPyramid; }
  const Pyramid<TFiltImage> & filteredPyramid() const { return m_filteredPyramid; }
  
  Pyramid<Image<GLubyte> > & validPyramid() { return m_destMaskPyramid; }
  const Pyramid<Image<GLubyte> > & validPyramid() const { return m_destMaskPyramid; }

  Pyramid<Image<srcCoord> > & sourcesPyramid() { return m_sourcesPyramid; }
  Pyramid<Image<srcCoord> > sourcesPyramid() const { return m_sourcesPyramid; }

  Pyramid<TFiltImage> & modeMaskPyramid() { return m_modeMaskPyramid; }
  Pyramid<TFiltImage> modeMaskPyramid() const { return m_modeMaskPyramid; }
  
  Pyramid<TFiltImage> & exampleModeMaskPyramid() { return m_exampleModeMaskPyramid; }
  Pyramid<TFiltImage> exampleModeMaskPyramid() const { return m_exampleModeMaskPyramid; }

  int numLevels() const { return m_numLevels; }
  int & numLevels() { return m_numLevels; }

  bool useSourceImages() const { return m_useSourceImages; }
  bool & useSourceImages() { return m_useSourceImages; }

  bool useSourceImagesAfterFirstPass() const { return m_useSourceImagesAfterFirstPass; }
  bool & useSourceImagesAfterFirstPass() { return m_useSourceImagesAfterFirstPass; }

  bool usePCA() const { return m_usePCA; }
  bool & usePCA() { return m_usePCA; }

  float levelWeighting() const { return m_levelWeighting; }
  float & levelWeighting() { return m_levelWeighting; }

  float finalSourceFac() const { return m_finalSourceFac; }
  float & finalSourceFac() { return m_finalSourceFac; }

  double sourceFac() const { return m_sourceFac; }
  double & sourceFac() { return m_sourceFac; }
  
  bool useRandom() const { return m_useRandom; }
  bool & useRandom() { return m_useRandom; }

  float samplerEpsilon() const { return m_samplerEpsilon; }
  float & samplerEpsilon()  { return m_samplerEpsilon; }

  int neighborhoodWidth() const { return m_neighborhoodWidth; }
  bool useSplineWeights() const { return m_useSplineWeights; }

  void SetNeighborhood(int neighborhoodWidth,bool useSplineWeights);

  enum SearchType { IMAGE_SEARCH=0, 
		    VECTOR_SEARCH=1, TSVQ_SEARCH=2, HEURISTIC_SEARCH=3,
		    MLP_SEARCH=4, TSVQR_SEARCH=5, ANN_SEARCH=6,
		    HEURISTIC_ANN_SEARCH=7, ASHIKHMIN_SEARCH=8}; 

  SearchType searchType() const { return m_searchType; }
  SearchType & searchType()  { return m_searchType; }

  PyramidType sourcePyramidType() const { return m_sourcePyramidType; }
  PyramidType & sourcePyramidType() { return m_sourcePyramidType; }

  PyramidType filteredPyramidType() const { return m_filteredPyramidType; }
  PyramidType & filteredPyramidType() { return m_filteredPyramidType; }

  SynthProcedureType filteredBaseProcedure() const 
    { return m_filteredBaseProcedure; } 
  SynthProcedureType & filteredBaseProcedure() 
    { return m_filteredBaseProcedure; }

  bool savePointsToFile() const { return m_savePointsToFile; }
  bool & savePointsToFile() { return m_savePointsToFile; }

  int maxTSVQdepth() const { return m_maxTSVQdepth; }
  int & maxTSVQdepth() { return m_maxTSVQdepth; }
  
  float maxTSVQerror() const { return m_maxTSVQerror; }
  float & maxTSVQerror() { return m_maxTSVQerror; }
  
  int TSVQbacktracks() const { return m_TSVQbacktracks; }
  int & TSVQbacktracks() { return m_TSVQbacktracks; }
  
  int xstep() const { return m_xstep; }
  int & xstep() { return m_xstep; }

  bool redisplay() const { return m_redisplay; }
  bool & redisplay() { return m_redisplay; }

  int MLPnumHidden() const { return m_MLPnumHidden(); }
  int & MLPnumHidden() { return m_MLPnumHidden; }

  double MLPhiddenDecayWeight() const { return m_MLPhiddenDecayWeight; }
  double & MLPhiddenDecayWeight() { return m_MLPhiddenDecayWeight; }
  
  double MLPoutputDecayWeight() const { return m_MLPoutputDecayWeight; }
  double & MLPoutputDecayWeight()  { return m_MLPoutputDecayWeight; }

  bool MLPsigmoidal() const { return m_MLPsigmoidal; }
  bool & MLPsigmoidal() { return m_MLPsigmoidal; }

  float annEpsilon() const { return m_annEpsilon; }
  float & annEpsilon() { return m_annEpsilon; }

  int numPasses() const { return m_numPasses; }
  int & numPasses() { return m_numPasses; }
  
  bool useBias() const { return m_useBias; }
  bool & useBias() { return m_useBias; }

  bool useGain() const { return m_useGain; }
  bool & useGain()  { return m_useGain; }

  double biasPenalty() const { return m_biasPenalty; }
  double & biasPenalty() { return m_biasPenalty; }

  double gainPenalty() const { return m_gainPenalty; }
  double & gainPenalty() { return m_gainPenalty; }

  bool createSrcLocHisto( void ) const { return m_createSrcLocHisto; }
  bool & createSrcLocHisto( void ) { return m_createSrcLocHisto; }

  float modeMaskWeight() const { return m_modeMaskWeight; }
  float & modeMaskWeight() { return m_modeMaskWeight; }

  float coherenceEps() const { return m_coherenceEps; }
  float & coherenceEps() { return m_coherenceEps; }

  float coherencePow() const { return m_coherencePow; }
  float & coherencePow() { return m_coherencePow; }

  bool cheesyBoundaries() const { return m_cheesyBoundaries; }
  bool & cheesyBoundaries() { return m_cheesyBoundaries; }

  bool oneway() const { return m_oneway; }
  bool & oneway() { return m_oneway; }
  
  bool ashikhminLastLevel() const { return m_ashikhminLastLevel; }
  bool & ashikhminLastLevel() { return m_ashikhminLastLevel; }

  bool onePixelSource() const { return m_onePixelSource; }
  bool & onePixelSource() { return m_onePixelSource; }

 protected:
  vector< Pyramid<TFiltImage> > m_filteredExamplePyramid;
  Pyramid<TFiltImage> m_filteredPyramid;

  vector< Pyramid<TSrcImage> > m_sourceExamplePyramid;
  Pyramid<TSrcImage> m_sourcePyramid;

  TFiltImage * m_destImage;

  Pyramid<Image<GLubyte> > m_destMaskPyramid;
  Pyramid<Image<srcCoord> > m_sourcesPyramid;
  FlagPyramid m_flags;  // for the heuristic search

  Pyramid<TFiltImage> m_exampleModeMaskPyramid;
  Pyramid<TFiltImage> m_modeMaskPyramid;

  int m_pass; // current pass number during computation
  bool m_useSourceImagesNow; // use source images for this pass?

  int m_numLevels;  // # of levels of the hierarchy to use in matching
  float m_levelWeighting; // ugh
  float m_finalSourceFac; // ugh
  bool m_useSourceImages;
  bool m_useSourceImagesAfterFirstPass;
  bool m_usePCA;
  double m_sourceFac;
  double m_sourceFacNow;
  bool m_useMMnow;
  bool m_useRandom;
  float m_samplerEpsilon;
  SearchType m_searchType;
  Sampler<Point2> * m_sampler;
  bool m_useSplineWeights;
  PyramidType m_sourcePyramidType;
  PyramidType m_filteredPyramidType;
  SynthProcedureType m_filteredBaseProcedure;
  bool m_savePointsToFile;
  int m_TSVQbacktracks;
  float m_maxTSVQerror;
  int m_maxTSVQdepth;
  int m_xstep;
  bool m_redisplay;
  float m_annEpsilon;
  int m_numPasses;
  bool m_useGain;
  bool m_useBias;
  double m_biasPenalty;
  double m_gainPenalty;
  bool m_createSrcLocHisto;
  float m_coherenceEps;
  float m_coherencePow;

  int m_MLPnumHidden;
  double m_MLPhiddenDecayWeight, m_MLPoutputDecayWeight;
  bool m_MLPsigmoidal;    

  bool m_useFilterModeMask;
  bool m_useTargetModeMask;
  bool m_ignoreModeMaskPass;
  float m_modeMaskWeight;

  bool m_oneway;

  CMatrix<double> m_eigenvectors; // for PCA
  Neigh m_mean;
  int m_neighborhoodWidth;
  CMatrix<double> * m_kernel;

  unsigned int m_numComparisons;  // statistic for comparing efficiency

  bool m_cheesyBoundaries;
  
  // statistics used to evaluate the heuristic and such
  int m_numSearches;
  int m_numTreeHits;
  int m_numPredHits;
  int m_numSearchHits;
  int m_numPredOnlyHits;
  long int m_totalSearchDist;
  bool m_keepSearchStats;

  // chuck's ashikhmin stuff, to be cleaned up:
  bool m_ashikhminLastLevel;
  int m_numPixelsSynthesized;
  double m_avgNumCandidates;

  bool m_onePixelSource;
};

#include "FilterLearnerInline.h"

#endif // !defined(FILTER_LEARNER_H


#ifndef SEARCHENVIRONMENT_H
#define SEARCHENVIRONMENT_H

#include "CVector.h"
#include "sampler.h"
#include "ANN/ANN.h"
#include "TSVQ.h"

//template<class T> class Sampler;

////////////////////////////////////
// search environment classes
////////////////////////////////////
class CSearchEnvironment
{
public:
  CSearchEnvironment( Sampler<Point2> *sampler ) : m_sampler( sampler ) {}
  virtual ~CSearchEnvironment( void ) {}

  void SetSampler( Sampler<Point2> * sampler ) { m_sampler = sampler; }

  // the big important method:
  virtual Point2 FindBestMatchLocation( Neigh &neighborhood ) = 0;

protected:
  Sampler<Point2> *m_sampler;

};

class CANNSearchEnvironment : public CSearchEnvironment
{
public:
  CANNSearchEnvironment( vector<Neigh*> &neighborhoods,
			 Sampler<Point2> *sampler );
  virtual ~CANNSearchEnvironment( void );

  // setting things
  void SetANNEpsilon( float epsilon ) { m_annEpsilon = epsilon; }
  
  virtual Point2 FindBestMatchLocation( Neigh &neighborhood );

  // accessor methods
  /*
  ANNkd_tree *ANNTree( void ) { return m_annTree; }
  int Dimension( void ) { return m_annDim; }
  int NumPoints( void ) { return m_numPoints; }
  ANNpointArray SearchPoints( void ) { return m_annPoints; }
  */
  
protected:
  ANNkd_tree *m_annTree;
  int m_numPoints;
  ANNpointArray m_annPoints;

  int m_annDim;
  ANNpoint m_annQueryPoint;

  vector<Point2> m_neighborhoodLocations;

  float m_annEpsilon;
};


class CTSVQSearchEnvironment : public CSearchEnvironment
{
public:
  CTSVQSearchEnvironment( vector<Neigh*> &neighborhoods,
			  Sampler<Point2> *sampler,
			  float maxError, int maxDepth );
  virtual ~CTSVQSearchEnvironment( void );

  // setting things
  void SetNumComparisons( int n ) { m_numComparisons = n; }
  void SetBacktracks( int n ) { m_TSVQbacktracks = n; }
  
  virtual Point2 FindBestMatchLocation( Neigh &neighborhood );
  
protected:
  TSVQ<Neigh> *m_tsvqTree;
  float m_maxTSVQerror;
  int m_maxTSVQdepth;

  int m_TSVQbacktracks;
  unsigned int m_numComparisons;
};




#endif SEARCHENVIRONMENT_H

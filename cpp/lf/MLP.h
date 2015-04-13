#ifndef __MLP_H__
#define __MLP_H__


#include <math.h>
#include <assert.h>
#include <vector>

#include "CVector.h"
#include "CMatrix.h"

using namespace std;

class MLP
{
 public:
  MLP(const vector<Neigh*> & inputData,  
      const vector<CVector<double>*> & targetData,
      int num_hidden, double hiddenDecayWeight, 
      double outputDecayWeight, bool sigmoidal=false);

  // note: the first argument is really CVector<double>; we use Neigh for
  // compatibility 

  CVector<double> apply(const CVector<double> & input) const;

 private:

  void computeEnergy(double * weights, double & energy, double * gradient,
		     const vector<Neigh*> & inputData, 
		     const vector<CVector<double>*> & targetData,
		     double hiddenDecayWeight, 
		     double outputDecayWeight) const;

  int _numInputs;
  int _numHidden;
  int _numOutputs;

  CMatrix<double> * _hiddenWeights;
  CVector<double> * _hiddenBiases;

  CMatrix<double> * _outputWeights;
  CVector<double> * _outputBiases;

  bool _sigmoidal;
};  


#endif


#ifdef BFGS



#include "MLP.h"
#include "BFGSwrap.h"

MLP::MLP(const vector<Neigh*> & inputData, 
	 const vector<CVector<double>*> & targetData,
	 int num_hidden, double hiddenDecayWeight, 
	 double outputDecayWeight, bool sigmoidal) 
{
  assert(!inputData.empty() && inputData.size() == targetData.size());

  _numInputs = (*inputData.begin())->Length();
  _numHidden = num_hidden;
  _numOutputs = (*targetData.begin())->Length();
  _sigmoidal = sigmoidal;

  assert(_numHidden > 0 && _numOutputs > 0);

  // create a parameter vector with random initialization

  int numWeights = _numHidden * (_numInputs + 1) +
    _numOutputs * (_numHidden + 1);
  double * weights = new double[numWeights];

  int i;

  for(i=0;i<numWeights;i++)
    //    weights[i] = i;
    weights[i] = drand48();
  Domain dom(numWeights);  // create a domain without constraints
  for(i=0;i<numWeights;i++)
    dom.setBound(i,DBL_MIN,DBL_MAX,Domain::None);

  // initialize optimizer
  BFGSoptimizer opt(numWeights, weights, &dom, 5 );
  opt.setPrintLevel(1); 
  
  opt.setAccuracyFactor(.001);
  opt.setGradientAccuracy(.001);

  printf("Training multi-layer perceptron\n");

  double energy;
  double * gradient = new double[numWeights];







  /*

  computeEnergy(weights,energy,gradient,inputData,targetData,
		hiddenDecayWeight,outputDecayWeight);


  printf("Initialization:\nWeights = ");
  for(i=0;i<numWeights;i++)
    printf("%f ",weights[i]);

  printf("\nEnergy = %f\n",energy);

  printf("Gradient = ");
  for(i=0;i<numWeights;i++)
    printf("%f ",gradient[i]);
  printf("\n");
  
  exit(1);


  */




  BFGSoptimizer::Results res;
  do
    {
      // it is ok to call it first time without computing f and g: 
      // it will return EvaluationRequest  
      res  = opt.nextIteration(energy,gradient);
      if( res == BFGSoptimizer::EvaluationRequest ) { 
	computeEnergy(weights,energy,gradient,inputData,targetData,
		      hiddenDecayWeight,outputDecayWeight);
      } 
    } while( ( res == BFGSoptimizer::IterationCompleted )|| 
	     ( res == BFGSoptimizer::EvaluationRequest));

  // copy weights into matrices
  _hiddenWeights = new CMatrix<double>(_numHidden, _numInputs);
  _hiddenBiases = new CVector<double>(_numHidden);

  _outputWeights = new CMatrix<double>(_numOutputs, _numHidden);
  _outputBiases = new CVector<double>(_numOutputs);

  int offset = 0;
  _hiddenWeights->copy(weights+offset);   offset += (_numHidden*_numInputs);
  _hiddenBiases->copy(weights+offset);    offset += _numHidden;
  _outputWeights->copy(weights+offset);   offset += (_numOutputs*_numHidden);
  _outputBiases->copy(weights+offset);    offset += _numOutputs;
  assert(offset == numWeights);
  
  delete [] weights;
  delete [] gradient;
}

CVector<double> 
MLP::apply(const CVector<double> & input) const
{
  CVector<double> a_hidden = (*_hiddenWeights) * input + (*_hiddenBiases);
  CVector<double> z_hidden;

  if (_sigmoidal)
    z_hidden = a_hidden.sigmoid();
  else
    z_hidden = a_hidden.tanh();
  
  return (*_outputWeights) * z_hidden + (*_outputBiases);
}

void
MLP::computeEnergy(double * weights, double & energy, double * gradient,
		   const vector<Neigh*> & inputData, 
		   const vector<CVector<double>*> & targetData,
		   double hiddenDecayWeight, 
		   double outputDecayWeight) const
{
  int numWeights = _numHidden * (_numInputs + 1) +
    _numOutputs * (_numHidden + 1);

  // put weights and gradients into matrix form
  int offset = 0;
  CMatrix<double> hiddenWeights(_numHidden,_numInputs,weights+offset,false);
  CMatrix<double> d_hiddenWeights(_numHidden,_numInputs,
				  gradient+offset,false);
  offset += (_numHidden * _numInputs);

  CVector<double> hiddenBiases(_numHidden,weights + offset,false);
  CVector<double> d_hiddenBiases(_numHidden,gradient+offset,false);
  offset += _numHidden;

  CMatrix<double> outputWeights(_numOutputs,_numHidden,weights+offset,false);
  CMatrix<double> d_outputWeights(_numOutputs,_numHidden,
				  gradient+offset,false);
  offset += (_numOutputs*_numHidden);

  CVector<double> outputBiases(_numOutputs,weights + offset,false);
  CVector<double> d_outputBiases(_numOutputs,gradient+offset,false);
  offset += _numOutputs;

  assert(offset == numWeights);

  // initialize gradients to 0
  d_hiddenWeights.clear(); d_hiddenBiases.clear();
  d_outputWeights.clear(); d_outputBiases.clear();

  // initialize the energy and gradient
  energy = 0;
  int i;
  //  for(int i=0;i<numWeights;i++)
  //    gradient[i] = 0;

  // =-----  add energy and gradient terms for each data point --------
  CVector<double> a_hidden(_numHidden);
  CVector<double> z_hidden(_numHidden);

  CVector<double> d_z_hidden(_numHidden);
  CVector<double> delta_hidden(_numHidden);
  CVector<double> delta_out(_numOutputs);

  for(i=0;i<inputData.size();i++)
  {
    // ------ energy computation ---------

    Neigh & x = *( inputData[i]);
    CVector<double> & y = *(targetData[i]);

    // apply the MLP to the input point
    a_hidden = hiddenWeights * x + hiddenBiases;

    if (_sigmoidal)
      z_hidden = a_hidden.sigmoid();
    else
      z_hidden = a_hidden.tanh();


    CVector<double> y_out = outputWeights * z_hidden + outputBiases;

    // energy is the L2^2 distance between the target and the actual output
    CVector<double> delta_out = y_out - y;

    energy += delta_out.L2sqr()/2;

    // ------- gradient computation (back-propagation) --------
    if (_sigmoidal)
      for(int j=0;j<_numHidden;j++)
	d_z_hidden[j] = z_hidden[j] * (1-z_hidden[j]);
    else
      for(int j=0;j<_numHidden;j++)
	{
	  double e1 = exp(a_hidden[j]);
	  double e2 = exp(-a_hidden[j]);
	  double denom = 2/(e1 + e2);
	  d_z_hidden[j] = denom*denom;
	}
    
    delta_hidden = d_z_hidden.mult(outputWeights.transpose() * delta_out);

    d_hiddenWeights += CMatrix<double>::outer(delta_hidden,x);
    d_hiddenBiases += delta_hidden;

    d_outputWeights += CMatrix<double>::outer(delta_out,z_hidden);
    d_outputBiases += delta_out;



    /*    
    printf("Input: ");
    x.printFloat();
    printf("\nTarget: ");
    y.printFloat();
    printf("\na: ");
    a_hidden.printFloat();
    printf("\nz: ");
    z_hidden.printFloat();
    printf("\ny_out: ");
    y_out.printFloat();

    printf("\ndelta_out: ");
    delta_out.printFloat();


    printf("\ndelta_out.L2sqr()/2 = %f\n",delta_out.L2sqr()/2);

    printf("d_z_hidden = ");
    d_z_hidden.printFloat();

    printf("\ndelta_hidden = ");
    delta_hidden.printFloat();

    printf("\nd_hiddenWeights = ");
    d_hiddenWeights.printFloat();

    printf("\n");
    */
  }

  // ----------------- add weight decay energy and gradient ---------------
  if (hiddenDecayWeight != 0)
    {
      energy += hiddenWeights.sumsqr()*hiddenDecayWeight/2;
      for(int i=0;i<hiddenWeights.rows();i++)
	for(int j=0;j<hiddenWeights.columns();j++)
	  d_hiddenWeights.get(i,j) += 
	    (hiddenWeights.get(i,j)*hiddenDecayWeight);
    }

  if (outputDecayWeight != 0)
    {
      energy += outputWeights.sumsqr()*outputDecayWeight/2;
      for(int i=0;i<outputWeights.rows();i++)
	for(int j=0;j<outputWeights.columns();j++)
	  d_outputWeights.get(i,j) += 
	    (outputWeights.get(i,j)*outputDecayWeight);
    }
}
#else


#include "MLP.h"

MLP::MLP(const vector<Neigh*> & , 
	 const vector<CVector<double>*> & ,
	 int , double, 
	 double , bool ) 
{

}

CVector<double> MLP::apply(const CVector<double> & input) const
{
  printf("MLP support not compiled in\n");

  return CVector<double>();
}




#endif

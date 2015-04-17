#ifndef __HISTOGRAM_HH__
#define __HISTOGRAM_HH__

#include <stdio.h>
#include <assert.h>
#include "Image.h"
#include "CMatrix.h"

struct Cluster
{
  double mean;
  double variance;
  double count;
};

class Histogram
{
private:
  int * _bins;
  int _numBins;
  double _maxVal;
  double _binSize;
  
  int _totalNumCounts;
  int _numICDFBins;
  double * _CDF;
  double * _inverseCDF;
  double _icdfBinSize;

  int _numClusters;
  Cluster * _clusters;
  CMatrix<double> * _labels;
  
public:
  Histogram(double binSize, int largestPossibleValue)
  {
    _maxVal = largestPossibleValue;
    _binSize = binSize;

    _numBins = (int)ceil(_maxVal / _binSize);
    
    _bins = new int[_numBins];
    if (_bins == NULL)
      {
	printf("error\n");
	exit(1);
      }
    memset(_bins,0,_numBins*sizeof(int));
    _CDF = NULL;
    _inverseCDF = NULL;

    _numICDFBins = 1000;

    _numClusters = -1;
    _clusters = NULL;
    _labels = NULL;
  } 

  ~Histogram()
  {
    delete [] _bins;
    if (_CDF != NULL) delete [] _CDF;
    if (_inverseCDF != NULL) delete [] _inverseCDF;
  }

  Histogram(const Image<float> & im,double binSize=1.0/256)
  {
    assert(im.dim() == 1);

    _maxVal = im.maxVal();
    _binSize = binSize;

    _numBins = (int)ceil(_maxVal / _binSize);
    
    _bins = new int[_numBins];
    if (_bins == NULL)
      {
	printf("error\n");
	exit(1);
      }
    memset(_bins,0,_numBins*sizeof(int));
    _CDF = NULL;
    _inverseCDF = NULL;
    _numICDFBins = 1000;

    for(int x=0;x<im.width();x++)
      for(int y=0;y<im.height();y++)
	insert(im.Pixel(x,y,0));

  }
  
  int valueToBin(double value) const
  {
    int bin = (int)floor(value / _binSize);
      
    assert(bin >=0 && bin < _numBins);

    return bin;
  }

  int getBin(int index) const
  {
    assert(index >=0 && index < _numBins);

    return _bins[index];
  }

  int & getBin(int index) 
  {
    assert(index >=0 && index < _numBins);

    return _bins[index];
  }

  void insert(double value)
  {
    int index = valueToBin(value);
    assert(index>=0 && index < _numBins);
    _bins[index] ++;

  }

  double binValue(int index) const
  {
    return _binSize*index;
  }
  
  void makeCDF()
  {
    if (_CDF == NULL)
      {
	_CDF = new double[_numBins];
	if (_CDF == NULL)
	  {
	    printf("error\n");
	    exit(1);
	  }
      }

    int totalCounts = 0;
    int i;
    for(i=0;i<_numBins;i++)
      totalCounts += _bins[i];

    _CDF[0] = _bins[0];
    for(i=1;i<_numBins;i++)
	{
	  _CDF[i] = _CDF[i-1] + _bins[i];
	  assert(_CDF[i] <= totalCounts);
	}
    for(i=0;i<_numBins;i++)
      {
	_CDF[i] /= double(totalCounts);
	assert(_CDF[i] <= 1);
      }
  }

  void makeInverseCDF()
  {
    if (_CDF == NULL)
      makeCDF();
    if(_inverseCDF == NULL)
      {
	_inverseCDF = new double[_numICDFBins];
	if (_inverseCDF == NULL)
	  {
	    printf("error\n");
	    exit(1);
	  }
      }

    _icdfBinSize = 1.0/(_numICDFBins-1);
    double targetBV = 0;
    int j = 0;
    for(int i=0;i<_numICDFBins;i++,targetBV += _icdfBinSize)
      {
	while (_CDF[j] < targetBV)
	  j++;
	assert(j<_numBins);
	assert(j==0 || _CDF[j] >= _CDF[j-1]);
	assert(j==0 || _CDF[j-1]<targetBV);
	if (j==0)
	  _inverseCDF[i] = lerp(0,_CDF[j],0,_binSize,targetBV);
	else
	  _inverseCDF[i] = lerp(_CDF[j-1],_CDF[j],binValue(j-1),binValue(j),
				targetBV);
      }      
  }

  static double lerp(double t1, double t2, double v1, double v2, double t) 
  {
    assert(t >= t1 && t <= t2);
    if (t == t1)
      return v1;
    if (t == t2)
      return v2;

    double alpha = (t-t1)/(t2 - t1);
    return (1-alpha) * v1 + alpha * v2;
  }
  /*
  static double lerp(const double * table, double t, double binsize)
  {
    int t1 = (int)floor(t/binsize);
    lerp(t1,t1+1,table[t1],table[t1+1],t);
  }
  */
  double lookupCDF(double value)
  {
    assert(value >=0 && value <= _maxVal);

    if (_CDF == NULL)
      makeCDF();

    int bin = (int)floor(value / _binSize);
      
    double result;

    if (bin+1 < _numBins)
      {
	assert(bin >=0 && bin+1< _numBins);
	result = lerp(bin,bin+1,_CDF[bin],_CDF[bin+1],value/_binSize);
      }
    else
      {
	assert(bin >=0 && bin < _numBins);
	result =  _CDF[bin];
      }

    assert(result >= 0 && result <= 1);
    return result;

  }

  double lookupInverseCDF(double value)
  {
    assert(value >=0 && value <= _maxVal);

    if (_inverseCDF == NULL)
      makeInverseCDF();

    int bin = (int)floor(value / _icdfBinSize);

      
    if (bin+1 < _numBins)
      {
	assert(bin >=0 && bin+1 < _numICDFBins);
	return lerp(bin,bin+1,_inverseCDF[bin],_inverseCDF[bin+1],
		    value/_icdfBinSize);
      }
    else
      {
	assert(bin >=0 && bin < _numICDFBins);
	return _inverseCDF[bin];
      }
  }

  double applyHistMatchFunc(double val, Histogram & target) 
    // given a value in this drawn from the distribution that generated
    // this histogram, transform it to match the target histogram
  {
    return target.lookupInverseCDF(lookupCDF(val))*_maxVal;
  }

  double applyHistEqFunc(double val)
    {
      return lookupCDF(val)*_maxVal;
    }

  void print(FILE * fp = stdout) const
    {
      for(int i=0;i<_numBins;i++)
	{
	  fprintf(fp,"%d ",_bins[i]);
	}
    }
  /*
  void fitMixtureModel(int numClusters, bool softLabeling)
    {
      _numClusters = numClusters;

      if (_clusters != NULL)
	delete [] _clusters;

      if (_labels != NULL)
	delete [] _labels;

      _clusters = new Cluster[numClusters];
      if (_clusters == NULL)
	{
	  printf("error\n");
	  exit(1);
	}

      _labels = new CMatrix(_numClusters,_numBins);
      if (_labels == NULL)
	{
	  printf("error\n");
	  exit(1);
	}


      
      
      
    }
*/
};
#endif

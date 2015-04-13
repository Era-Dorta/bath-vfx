#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include <vector>

#ifndef _WIN32
#include <multimap.h>
#else
#include <map>
#endif

#include "compat.h"



using namespace std;

template<class T>
class Sampler
{
 private:
  multimap<float,T> _data;
  vector<unsigned int> _stats;
  float _epsilon;
  float _threshold;
  
 public:
  Sampler(float epsilon=.1) : _epsilon(epsilon), _threshold(-1) {}
  
  float epsilon() const { return _epsilon; }
  void setEpsilon(float val) { _epsilon = val;
  if (size() > 0) _threshold = BestSample().first*(1+_epsilon); }
  float threshold() const { return _threshold; }

  void resetStats() { stats.erase(stats.begin(),stats.end()); }

  void eraseSamples() { _data.erase(_data.begin(),_data.end()); _threshold=-1;}
  
  void printStats()
    {
      int total = 0, i=0;
      for(i=0;i<_stats.size();i++)
	total += _stats[i];
      
      if (total == 0)
	{
	  printf("no samples (size = %d)\n",_stats.size());
	  return;
	}
      for(i=0;i<_stats[i];i++)
	printf("%f(%d) ",_stats[i]/float(total),_stats[i]);
      printf("(total: %d)\n",total);
    }

  void addElement(const float & distance, const T & newElement)
    {
      assert(distance >= 0);

      if (_data.empty())
	{
	  _data.insert(pair<float,T>(distance,newElement));

	  _threshold = distance*(1+_epsilon);

	  return;
	}

      if (distance > _threshold)
	return;

      _data.insert(pair<float,T>(distance,newElement));
      _threshold = BestSample().first*(1+_epsilon);
    }

  int size() const { return _data.size(); }

  pair<float,T> BestSample()
    {
      assert(_data.size() > 0);

      return *_data.begin();
    }

  pair<float,T> UniformSample() 
    {
      assert(_data.size() > 0);

      unsigned int zero = 0;

      // remove extra elements
      multimap<float,T>::iterator it = _data.upper_bound(_threshold);
      if (it != _data.end())
	_data.erase(it,_data.end());
      
      if (_data.size() == 1)
	{
	  if (_stats.empty())
	    _stats.push_back(zero);
	  _stats[0] ++;

	  return *_data.begin();
	}

      int i=(int)(drand48()*_data.size())%_data.size();

      assert(i < _data.size());

      it = _data.begin();
      for(int j=0;j<i;j++)
	++it;

      assert(it != _data.end());
      //      printf("\npicked sample %d of %d: %d %d\n",i,_data.size(),(*it).second.x(), (*it).second.y());

      while (i >= _stats.size())
	_stats.push_back(zero);

      _stats[i] ++;

      return *it;
    }

  void print() const 
    {
      printf("samples: ");
      for(multimap<float,T>::const_iterator it2=_data.begin(); it2 != _data.end(); ++it2)
	printf("%f (%d %d)",(*it2).first,(*it2).second.x(), (*it2).second.y());
      printf("; epsilon = %f\n",_epsilon);


    }
  
};


#endif

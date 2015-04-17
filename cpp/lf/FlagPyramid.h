#ifndef __FLAG_PYRAMID_H__
#define __FLAG_PYRAMID_H__

#include "Pyramid.h"

class FlagPyramid :  Pyramid<Image<unsigned int> >
{
 private: 
  unsigned int _counter;
  
 public:
  FlagPyramid() : Pyramid<Image<unsigned int> >(),_counter(1) { }
  //  FlagPyramid(int levels, int width, int height) : 
  //    Pyramid<Image<unsigned int> >(levels,width,height,1),_counter(1) { clear(); }
  void make(int levels,int width,int height)
    { makePyramid(levels,width,height,1,FLAGS_PYRAMID); }
  void clearAllFlags() { _counter ++; }
  bool isFlagSet(int x,int y,int level)
    { return (*this)[level].Pixel(x,y) == _counter; }
  void setFlag(int x,int y,int level) 
    { (*this)[level].Pixel(x,y) = _counter; }
  void clearFlag(int x,int y,int level)
    {(*this)[level].Pixel(x,y) = _counter-1; }
  bool testAndSet(int x,int y,int level)
    { if (isFlagSet(x,y,level)) return true;
    else setFlag(x,y,level); return false; 
    }
};


#endif

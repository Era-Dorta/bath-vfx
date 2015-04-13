#ifndef __PYRAMID_H__
#define __PYRAMID_H__

#include "Image.h"
extern "C" {
//#include "spyramid.h"
}

template <class TImage>
class Pyramid
{
 private:
  TImage ** _imgs;
  int _levels;
  bool _ownsFine;
  PyramidType _myType;
//  PYRAMID  _steerPyr;

public:
  Pyramid() : _levels(-1),_imgs(NULL), _ownsFine(false) { }
  Pyramid(const Pyramid & p);
  Pyramid( int levels, int width, int height, int dim );
  //  Pyramid(int height,TImage * FineImage);
  void makePyramid(int height,TImage * FineImage,PyramidType ptype);
  void makePyramid(int levels,int imWidth,int imHeight,int imDim,
  		   PyramidType ptype);
//  void makePyramid(int levels,int imWidth,int imHeight,int imDim,
//		   PyramidType ptype, PFILTER pf = NULL);
//  void makePyramid(int height,TImage * FineImage,PyramidType ptype,
//		   PFILTER pf);
  void clear();
  ~Pyramid();
  TImage & operator[](int i) { assert(i>=0&&i<_levels); return *_imgs[i]; }
  const TImage & operator[](int i) const { assert(i>=0&&i<_levels); return *_imgs[i]; }
  TImage & FineImage( void ) { assert(_levels > 0); return *_imgs[0]; }
  const TImage & FineImage( void ) const { assert(_levels > 0); return *_imgs[0]; }
  void drawColumn(int x,int y,bool rescale=false) const;
  void drawSourcesColumn(int x,int y,int rmax,int gmax) const;
  int NLevels() const { return _levels; }
  void reconstruct();
  bool winxyToImagexy(int wx,int wy,int & ix, int &iy, int & level);
  PyramidType type() const { return _myType; }
  int displayWidth() const { return FineImage().width()+5; }
  int displayHeight() const;

  COLORSPACE Colorspace( void );
  void SetColorspace( COLORSPACE c );
  void ConvertToColorspace( COLORSPACE c );
};
  
#include "PyramidInline.h"

#endif

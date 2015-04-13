#ifndef __IMAGE_HH__
#define __IMAGE_HH__

#include <assert.h>
#include <string.h>
#include <vector>
#include <map>

#include "compat.h"
#include "color.h"
#include "BmpReader.h"
#include "CMatrix.h"
extern "C" {
//#include "matrix.h"
}

using namespace std;

#define ASCII_PGM  2
#define RAW_PGM  5

#define ASCII_PPM 3
#define RAW_PPM 6

enum SynthProcedureType { FILTER_SYNTH=0, COPY_SYNTH=1, 
			  FILTER_NO_SOURCE_SYNTH =2, ZERO_SYNTH=3};
enum PyramidType { GAUSSIAN_PYRAMID=0, LAPLACIAN_PYRAMID=1,
                   SOURCES_PYRAMID=4, FLAGS_PYRAMID=3, 
		   STEERABLE_PYRAMID=2 };
enum FeatureType { RAW_RGB_FEATURE=0, DIFFERENCE_FEATURE=1,
		   LUV_FEATURE=2 };
		 



template <class T>
class Image
{
 private:
  T * _pixels;
  int _width;
  int _height;
  int _dim;

  int _winx, _winy;  // coordinates of the upper-left corner in the
                     // display window

  COLORSPACE _colorspace;

 public:
  Image(int w,int h,int d);
  Image(const char *filename);
  Image(int w,int h,int d,T initialValue);
  Image(const Image & img);
//  Image(MATRIX &m);
  virtual ~Image() { delete [] _pixels; }

  int width() const { return _width; }
  int height() const { return _height; }
  int dim() const { return _dim; }
  int myformat() const;
  int mytype() const;
  T maxVal() const;

  int winx() const { return _winx; }
  int & winx() { return _winx; }
  int winy() const { return _winy; }
  int & winy() { return _winy; }
  bool winxyToImagexy(int wx,int wy, int &ix, int &iy);
  void imagexyToWinxy(int ix,int iy, int &wx, int &wy);

  T Pixel(int x,int y,int n=0) const;
  T & Pixel(int x,int y,int n=0) ;
  void setPixel(int x,int y, T val);
  void setPixel(int x,int y, T val1, T val2);
  void setPixel(int x,int y, T val1, T val2, T val3);
  void setPixel(int x,int y, T val1, T val2, T val3, T val4);
  void copyPixel(int x,int y,const Image & src,int srcX,int srcY);

  bool inside(int i) const;
  bool inside(int x,int y) const;
  bool inside(int x,int y,int n) const;

  void RGBtoYIQ(Image & img);
  void RGBtoGrayScale(Image<T> &img);
/*  void RGBtoMatrix(MATRIX & m);
  void GraytoMatrix(MATRIX & m);
  void MatrixtoGray(MATRIX & m);
  //sets a specific band (dimension) of the image with
  //the data in MATRIX
  void setImgBand(MATRIX &m, int band, int rescale=0);
  //saves one band as PNG
  void saveBandPNG(const char *filename, int band);*/
 
  void copy(const Image & img);
  void draw(int x=0,int y=0);
  void read(int x=0,int y=0);
  void scaleDraw(int x=0,int y=0,float bias=.5,float scale=.5);
  void sourcesDraw(int x,int y,int sourceWidth,int sourceHeight);
  void set(T val);
  void set() { set(maxVal()); }
  void clear() { memset(_pixels,0,_width*_height*_dim*sizeof(T)); }
  void insert(Image & im, int x0,int y0,bool rescale=false);
  void linearMap(int x,int y,const CMatrix<double> & gain, 
		 const CVector<double> & bias);

  
  Image * downsample();
  Image * upsample();
  Image * luminance();

  bool loadBMP( const char *filename );
  void loadPNM( const char *filename );
  void loadPNG( const char *filename );
  void savePNG( const char *filename ); 
  bool loadData( const char *filename);
  bool saveData( const char *filename);

  void meanVariance( T & mean, T & variance);
  void meanCovariance(CVector<double> & mean, CMatrix<double> & covariance);

  // colorspace conversion routines
  COLORSPACE Colorspace( void ) { return _colorspace; }
  void SetColorspace( COLORSPACE c );
  void ConvertToColorspace( COLORSPACE c );

  void PrintExtrema( void );

  void applyGainBias(float gain,float bias);
  void applyGainBias(const CMatrix<double>& gain,const CVector<double> & bias);
  void applyGainBias(int x,int y,const CMatrix<double> & gain, 
		     const CVector<double> & bias);
};

typedef Image<GLfloat> FloatImage;
typedef Image<GLfloat> ColorImage;

#include "ImageInline.h"

#endif



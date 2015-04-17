#ifndef __COLOR_HH__
#define __COLOR_HH__

#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

#include "compat.h"

//#include <stdio.h>


enum COLORSPACE { RGB_SPACE, XYZ_SPACE, YIQ_SPACE, LAB_SPACE, LUV_SPACE };

#define IMAGE_MODE GL_RGB
#define RED_OFFSET 0
#define GREEN_OFFSET 1
#define BLUE_OFFSET 2
//#define ALPHA_OFFSET -100
//#define COLOR_CHANNELS 3


#if 0


#ifdef _WIN32
#define IMAGE_MODE   GL_RGBA
#define RED_OFFSET 0
#define GREEN_OFFSET 1
#define BLUE_OFFSET 2
#define ALPHA_OFFSET 3
#define COLOR_CHANNELS 4
#else
#define  IMAGE_MODE  GL_ABGR_EXT

#define RED_OFFSET 3
#define GREEN_OFFSET 2
#define BLUE_OFFSET 1
#define ALPHA_OFFSET 0
#define COLOR_CHANNELS 4
#endif

#endif






float lookupSqrt(float);


inline float Max(float v1,float v2)
{
  return v1 > v2 ? v1 : v2;
}

inline float Min(float v1,float v2)
{
  return v1 < v2 ? v1 : v2;
}

inline GLubyte byteClamp(float v)
{
  return GLubyte(Max(Min(v,255),0));
}


inline GLubyte getRed(GLuint color) 
{ 
    return (color >> ((3-RED_OFFSET) * 8)) & 0xFF;
}

inline GLubyte getGreen(GLuint color) 
{ 
    return (color >> ((3-GREEN_OFFSET) * 8)) & 0xFF;
}

inline GLubyte getBlue(GLuint color) 
{ 
    return (color >> ((3-BLUE_OFFSET) * 8)) & 0xFF;
}

#ifdef ALPHA_OFFSET
inline GLubyte getAlpha(GLuint color) 
{ 
    return (color >> ((3-ALPHA_OFFSET) * 8)) & 0xFF;
}

inline float getAlphaf(GLuint color)
{
  return getAlpha(color)/255.0;
}

inline GLuint makeColor(GLubyte red,GLubyte green, GLubyte blue, GLubyte alpha)
{
  return 
    (GLuint(red) << ((3-RED_OFFSET)*8)) | 
    (GLuint(green) << ((3-GREEN_OFFSET)*8)) |
    (GLuint(blue) << ((3-BLUE_OFFSET)*8)) | 
    (GLuint(alpha) << ((3-ALPHA_OFFSET)*8));
}

inline GLuint makeColor(GLubyte red,GLubyte green, GLubyte blue, float alpha)
{
  return makeColor(red,green,blue,GLubyte(alpha*255));
}

inline GLuint makeColor(GLubyte red,GLubyte green,GLubyte blue)
{
  return makeColor(red,green,blue,GLubyte(0xFF));
}

inline GLuint makeColorf(float red,float green,float blue,float alpha)
{
  return makeColor(GLubyte(red*255),GLubyte(green*255),GLubyte(blue*255),
		   GLubyte(alpha*255));
}

inline GLuint makeColorf(float red,float green,float blue)
{
  return makeColorf(red,green,blue,1);
}

inline GLuint makeColor(GLubyte luminance)
{
  return makeColor(luminance,luminance,luminance);
}

inline GLuint makeColorf(float luminance)
{
  return makeColorf(luminance,luminance,luminance);
}
#endif



GLuint alphaBlend(GLuint color1,float alpha,GLuint color2);

GLuint alphaBlend(GLuint color1,GLuint color2);

inline float luminance(GLubyte red,GLubyte green,GLubyte blue)
{
  //  printf("r = %d, g= %d, b=%d  l=%f",red,green,blue,
  //	 (30*red + 59*green + 11*blue)/(0xFF*100.0));


  return (30*red + 59*green + 11*blue)/(0xFF*100.0);
}

inline float luminance(GLuint color)
{
  return luminance(getRed(color),getGreen(color),getBlue(color));
}

inline double luminance(double red, double green, double blue)
{
  return .299*red + .587*green + .114*blue;

  // TODO: change this to the contemporary:
  return .2125*red + .7154*green + .0721*blue;
}

inline void RGBtoYIQ(double red, double green, double blue,
		      double &luminance, double &I, double &Q)
{
    // from Foley-Van Dam: NTSC RGB
  luminance = .299*red + .587*green + .114*blue;
  I =         .596*red  - .275*green - .321*blue;
  Q =         .212*red  - .523*green + .311*blue;
}

inline void YIQtoRGB(double luminance, double I, double Q,
		     double & red, double & green, double &blue)
{
  red = luminance + 0.95568806036116*I + 0.61985809445637*Q;
  green = luminance -0.27158179694406*I -0.64687381613840*Q;
  blue = luminance -1.10817732668266*I + 1.70506455991918*Q;
}


inline float colorDifference(GLuint color1,GLuint color2)
{
    // GLbyte is 8 bits, GLshort is 16 bits, GLint is 32 bits

  GLint dr = GLshort(getRed(color1)) - getRed(color2);
  GLint dg = GLshort(getGreen(color1)) - getGreen(color2);
  GLint db = GLshort(getBlue(color1)) - getBlue(color2);
  
  //  return dr*dr + dg*dg + db*db;
  return sqrtf(dr*dr + dg*dg + db*db);
}

inline GLint colorDifferenceSqr(GLuint color1,GLuint color2)
{
    // GLbyte is 8 bits, GLshort is 16 bits, GLint is 32 bits

  GLint dr = GLshort(getRed(color1)) - getRed(color2);
  GLint dg = GLshort(getGreen(color1)) - getGreen(color2);
  GLint db = GLshort(getBlue(color1)) - getBlue(color2);
  
  //  return dr*dr + dg*dg + db*db;
  return dr*dr + dg*dg + db*db;
}

const float maxColorDifference = 255*sqrt(3);

#ifdef ALPHA_OFFSET
inline void RGBtoLUV(GLuint color,float &L,float &u,float &v)
{
    // from Foley-Van Dam: NTSC RGB
    const float xr = .67;
    const float xg = .21;
    const float xb = .14;
    const float yr = .33;
    const float yg = .71;
    const float yb = .08;
    const float Yr = .299;
    const float Yg = .587;
    const float Yb = .114;

    const float Xn = 98.1012658228;
    const float Yn = 100;
    const float Zn = 118.35443038;

    const float upn = 4*Xn/(Xn+15*Yn+3*Zn);
    const float vpn = 9*Yn/(Xn+15*Yn+3*Zn);

    GLubyte r = getRed(color);
    GLubyte g = getGreen(color);
    GLubyte b = getBlue(color);

    if (r == 0 && g == 0 && b == 0)
	{
	    L = 0; u = 0; v = 0; return;
	}
   
    float x = xr*r+xg*g+xb*b;
    float y = yr*r+yg*g+yb*b;

    float Y = Yr*r+Yg*g+Yb*b;
    float X = x*Y/y;
    float Z = (1-x-y)*Y/y;

    float denom = X+15*Y+3*Z;
    float up = 4*X/denom;
    float vp = 9*Y/denom;

    L = 116 * pow(Y/Yn,1.0/3)-16;
    if (L < 0)
	L = 0;

    u = 13 * L * (up-upn);
    v = 13 * L * (vp-vpn);
}

inline void RGB_To_HSV(GLuint color,float &h,float &s, float &v)
{
  float r = getRed(color)/255.0;
  float g = getGreen(color)/255.0;
  float b = getBlue(color)/255.0;

  float max = Max(r,Max(g,b));
  float min = Min(r,Min(g,b));

  v = max;
  
  s = (max != 0) ? ((max-min)/max) : 0;

  if (s != 0)
    {
      double delta = max-min;
      if (r==max)
	h=(g-b)/delta;
      else
	if (g==max)
	  h=2+(b-r)/delta;
	else
	  if (b==max)
	    h=4+(r-g)/delta;

      h /= 6;

      if (h < 0)
	h += 1;
    }
}
      
inline GLuint HSV_To_RGB(float h,float s,float v)
{
    // from Foley-Van Dam: NTSC RGB
  if (s == 0)
      return makeColor(v*255,v*255,v*255);

  double f,p,q,t,r,g,b;
  int i;

  if (h == 1)
    h = 0;

  h *= 6;

  i = floor(h);

  f = h-i;
  p = v*(1-s);
  q = v*(1-s*f);
  t = v*(1-(s*(1-f)));

  switch(i)
    {
    case 0: r=v; g=t; b=p; break;
    case 1: r=q; g=v; b=p; break;
    case 2: r=p; g=v; b=t; break;
    case 3: r=p; g=q; b=v; break;
    case 4: r=t; g=p; b=v; break;
    case 5: r=v; g=p; b=q; break;
    }

  return makeColor(r*255,g*255,b*255);
}
#endif


GLuint adjustColor(GLuint color);



// XYZ, RGB, R'G'B', L*u*v*, L*a*b*

template <class PixType>
void XYZtoRGB( PixType X, PixType Y, PixType Z,
	       PixType &R, PixType &G, PixType &B )
{
  R =  3.240479 * X  - 1.537150 * Y  - 0.498535 * Z;
  G = -0.969256 * X  + 1.875992 * Y  + 0.041556 * Z;
  B =  0.055648 * X  - 0.204043 * Y  + 1.057311 * Z;

  R /= 100.0;
  G /= 100.0;
  B /= 100.0;
}

template <class PixType>
void XYZtoRGB( PixType &X, PixType &Y, PixType &Z )
{
  XYZtoRGB( X, Y, Z, X, Y, Z );
}


template <class PixType>
void RGBtoXYZ( PixType R, PixType G, PixType B,
	  PixType &X, PixType &Y, PixType &Z )
{
  X = 0.412453 * R  + 0.357580 * G  + 0.180423 * B;
  Y = 0.212671 * R  + 0.715160 * G  + 0.072169 * B;
  Z = 0.019334 * R  + 0.119193 * G  + 0.950227 * B;

  X *= 100;
  Y *= 100;
  Z *= 100;
}

template <class PixType>
void RGBtoXYZ( PixType &R, PixType &G, PixType &B )
{
  RGBtoXYZ( R, G, B, R, G, B );
}

template <class PixType>
void XYZtoLuv( PixType X, PixType Y, PixType Z,
	       PixType &L, PixType &u, PixType &v )
{
  PixType Xn = 95.0456;
  PixType Yn = 100.0;
  PixType Zn = 108.8754;

  PixType uPrime = 4*X / (X + 15*Y + 3*Z);
  PixType vPrime = 9*Y / (X + 15*Y + 3*Z);

  // chromaticity of white point:
  PixType unPrime = 4*Xn / (Xn + 15*Yn + 3*Zn);
  PixType vnPrime = 9*Yn / (Xn + 15*Yn + 3*Zn);

  // this is for Y/Yn > 0.008856
  L = max( (PixType)0.0, (PixType)(116*pow(Y/Yn, 1.0/3.0) - 16));

  // the following are for X/Xn, Y/Yn, Z/Zn > 0.01
  u = 13*L*(uPrime-unPrime);
  v = 13*L*(vPrime-vnPrime);
}

template <class PixType>
void XYZtoLuv( PixType &X, PixType &Y, PixType &Z )
{
  XYZtoLuv( X, Y, Z, X, Y, Z );
}

template <class PixType>
void LuvtoXYZ( PixType L, PixType u, PixType v,
	       PixType &X, PixType &Y, PixType &Z )
{
  PixType Xn = 95.0456;
  PixType Yn = 100.0;
  PixType Zn = 108.8754;

  // check all this, etc.
  Y = Yn * pow( (L+16.0) / 116.0, 3.0 );
  PixType fY = pow( Y/Yn, 1.0/3.0 );

  // chromaticity of white point:
  PixType unPrime = 4*Xn / (Xn + 15*Yn + 3*Zn);
  PixType vnPrime = 9*Yn / (Xn + 15*Yn + 3*Zn);

  PixType uPrime = u / (13*L) + unPrime;
  PixType vPrime = v / (13*L) + vnPrime;

  X = (9*Y*uPrime) / (4*vPrime);
  Z = (1.0/3.0) * ((4*X)/uPrime - X - 15*Y);
}

template <class PixType>
void LuvtoXYZ( PixType &L, PixType &u, PixType &v )
{
  LuvtoXYZ( L, u, v, L, u, v );
}

template <class PixType>
void XYZtoLab( PixType X, PixType Y, PixType Z,
	  PixType &L, PixType &a, PixType &b )
{
  PixType Xn = 95.0456;
  PixType Yn = 100.0;
  PixType Zn = 108.8754;

  // this is only valid if Y/Yn > 0.008856
  L = max( (PixType)0.0, (PixType)(116*pow(Y/Yn, 1.0/3.0) - 16) );
  
  // the following are really only valid for X/Xn, Y/Yn, Z/Zn > 0.01
  a = 500*(pow(X/Xn, 1.0/3.0) - pow(Y/Yn, 1.0/3.0));
  b = 200*(pow(Y/Yn, 1.0/3.0) - pow(Z/Zn, 1.0/3.0));
}

template <class PixType>
void XYZtoLab( PixType &X, PixType &Y, PixType &Z )
{
  XYZtoLab( X, Y, Z, X, Y, Z );
}

template <class PixType>
void LabtoXYZ( PixType L, PixType a, PixType b,
	       PixType &X, PixType &Y, PixType &Z )
{
  PixType Xn = 95.0456;
  PixType Yn = 100.0;
  PixType Zn = 108.8754;

  // check all this, etc.
  Y = Yn * pow( (L+16.0) / 116.0, 3.0 );
  PixType fY = pow( Y/Yn, 1.0/3.0 );

  X = Xn*pow((a / 500.0) + fY, 3.0 );
  Z = Zn * pow( fY - (b/200.0), 3.0 );
}


template <class PixType>
void LabtoXYZ( PixType &L, PixType &a, PixType &b )
{
  LabtoXYZ( L, a, b, L, a, b );
}

/*
template <class PixType>
void LabtoRGB( PixType X, PixType Y, PixType Z,
	       Pixtype &R, PixType &G, PixType &B )
{
  Labto
}
*/

#endif

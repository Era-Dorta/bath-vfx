#include <math.h>

#include "color.h"

 float hjit=0,sjit=0,vjit=0,RGBjit=0;
float hfac=1,sfac=1,vfac=1;

#ifdef ALPHA_OFFSET

GLuint alphaBlend(GLuint color1,float alpha,GLuint color2)
{
  // common cases
  if (alpha == 1)
    return color1;

  if (alpha == 0)
    return color2;

  GLubyte r = getRed(color1)*alpha + getRed(color2)*(1-alpha);
  GLubyte g = getGreen(color1)*alpha + getGreen(color2)*(1-alpha);
  GLubyte b = getBlue(color1)*alpha + getBlue(color2)*(1-alpha);

  return makeColor(r,g,b,GLubyte(0xFF));
}

GLuint alphaBlend(GLuint color1,GLuint color2)
{
  int alpha = getAlpha(color1);

  if (alpha == 0xFF)
    return color1;

  if (alpha == 0)
    return color2;

  GLubyte r = (getRed(color1)*alpha + getRed(color2)*(0xFF-alpha))/0xFF;
  GLubyte g = (getGreen(color1)*alpha + getGreen(color2)*(0xFF-alpha))/0xFF;
  GLubyte b = (getBlue(color1)*alpha + getBlue(color2)*(0xFF-alpha))/0xFF;
  
  return makeColor(r,g,b,GLubyte(0xFF));
}

GLuint adjustColor(GLuint color)
{
  if (hjit != 0 || sjit != 0 || vjit != 0)
    {
	float h,s,v;
	
	RGB_To_HSV(color,h,s,v);

	h = hfac*(h + hjit*(drand48()-.5));
	if (h>1)
	    h-= 1;
	else
	    if (h<0)
		h+=1;
  
	s = sfac*(s + sjit*(drand48()-.5));
	s = Min(Max(s,0),1);
	
	v = vfac*(v + vjit*(drand48()-.5));
	v = Min(Max(v,0),1);

	color = HSV_To_RGB(h,s,v);
    }

  if (RGBjit != 0)
    {
	GLubyte red = getRed(color);
	GLubyte green = getGreen(color);
	GLubyte blue = getBlue(color);
	
	red += RGBjit * (drand48() - .5);
	green += RGBjit * (drand48() - .5);
	blue += RGBjit * (drand48() - .5);

	color = makeColor(red,green,blue);
    }

  return color;
}
#endif

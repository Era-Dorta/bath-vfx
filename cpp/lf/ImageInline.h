enum ImageFileFormat { UNKNOWN_FORMAT, BMP_FORMAT, PNM_FORMAT, PNG_FORMAT,
		       RAW_FORMAT };
// image formats we can read

inline int Image<GLubyte>::mytype() const { return GL_UNSIGNED_BYTE; }

inline int Image<GLfloat>::mytype() const { return GL_FLOAT; }

inline int Image<GLshort>::mytype() const { return GL_SHORT; }
inline int Image<GLint>::mytype() const { return GL_INT; }

inline GLubyte Image<GLubyte>::maxVal() const { return 255; }

inline GLfloat Image<GLfloat>::maxVal() const { return 1; }

inline GLint Image<GLint>::maxVal() const { return 1; }


inline
ImageFileFormat getFileFormat( const char *filename )
{
  const char *ext = strrchr( filename, '.' );
  if( !ext )
    return UNKNOWN_FORMAT;
  else if( strcmp( ext, ".bmp" ) == 0 )
    return BMP_FORMAT;
  else if( strcmp( ext, ".ppm" ) == 0 )
    return PNM_FORMAT;
  else if( strcmp( ext, ".pgm" ) == 0 )
    return PNM_FORMAT;
  else if( strcmp( ext, ".png" ) == 0 )
    return PNG_FORMAT;
  else if (strcmp( ext, ".data" ) == 0)
    return RAW_FORMAT;
  else
    return UNKNOWN_FORMAT;
}

template <class T>
Image<T>::Image(int w,int h,int d) : 
  _width(w), _height(h), _dim(d), _winx(-1), _winy(-1), _colorspace(RGB_SPACE)
{
  assert(_dim > 0);
  _pixels = new T[_width*_height*_dim];
  if (_pixels == NULL)
    {
      printf("Can't allocate image\n");
      exit(1);
    }
}

template <class T>
Image<T>::Image(int w,int h,int d,T initialValue) : 
  _width(w), _height(h), _dim(d), _winx(-1), _winy(-1), _colorspace(RGB_SPACE)
{
  assert(_dim > 0);
  _pixels = new T[_width*_height*_dim];
  if (_pixels == NULL)
    {
      printf("Can't allocate image\n");
      exit(1);
    }
  for(int i=0;i<_width*_height*_dim;i++)
    _pixels[i] = initialValue;
}

template <class T>
Image<T>::Image(const Image & img) : 
  _width(img._width),_height(img._height), _dim(img._dim),
  _winx(img._winx), _winy(img._winy), _colorspace(img._colorspace)
{
  assert(_dim > 0);
  _pixels = new T[_width*_height*_dim];
  if (_pixels == NULL)
    {
      printf("Can't allocate image\n");
      exit(1);
    }
  memcpy(_pixels,img._pixels,_width*_height*_dim*sizeof(T));
}
/*
template <class T>
Image<T>::Image(MATRIX & m) : 
  _width(m->columns),_height(m->rows), _dim(1),
  _winx(-1), _winy(-1)
{
  assert(_dim > 0);
  _pixels = new T[_width*_height*_dim];
  if (_pixels == NULL)
    {
      printf("Can't allocate image\n");
      exit(1);
    }
  this->MatrixtoGray(m);
}

//converts the image into a BW image by applying the 
//equation: m=.3r + .59g + .11b -->takes the luminance
template <class T>
void Image<T>::RGBtoMatrix(MATRIX &m)
{
  //it assumes the image is RGB
  assert(_dim == 3); 
  assert( width() == m->columns && height() == m->rows);
  float c=0;
  int   mr = m->rows;
  int   mcol = m->columns;
  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)      
      m->values[y*mcol + x] = .3 * (float) Pixel(x,y,0)+.59*(float)Pixel(x,y,1)+.11*(float)Pixel(x,y,2);	
}

//converts a grayscale image to MATRIX
template <class T>
void Image<T>::GraytoMatrix(MATRIX &m)
{
  assert(_dim==1);
  assert(width() == m->columns && height() == m->rows);
  float c=0;
  int   mr = m->rows;
  int   mcol = m->columns;
  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)      
      m->values[y*mcol + x] = (float)Pixel(x,y,0);
}

//converts a MATRIX into a grayscale image
template <class T>
void Image<T>::MatrixtoGray(MATRIX &m)
{
  assert(_dim==1);
  assert(width() == m->columns && height() == m->rows);
  float c=0;
  int   mr = m->rows;
  int   mcol = m->columns;
  float max_val, min_val, new_val, scale, pedestal;
  max_val = min_val = m->values[0];
  for (int i=0; i<mr*mcol; i++) {
    new_val = m->values[i];
    if (max_val < new_val) max_val = new_val; 
    if (min_val > new_val) min_val = new_val; 
  }
  if (max_val != min_val) {
    scale = maxVal()/(max_val-min_val);
  } else {
    scale = 0.0;
  }
  pedestal = -scale*min_val;
  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)      
      Pixel(x,y,0) = m->values[y*mcol + x]*scale+pedestal;
}


//converts a MATRIX into a grayscale image
template <class T>
void Image<T>::setImgBand(MATRIX &m, int band, int rescale)
{
  assert(_dim>=band);

  float c=0;
  int   mr = m->rows;
  int   mcol = m->columns;
  float max_val, min_val, new_val, scale=1.0, pedestal=0.0, auxv;
  if (rescale) {
    max_val = min_val = m->values[0];
    for (int i=0; i<mr*mcol; i++) {
      new_val = m->values[i];
      if (max_val < new_val) max_val = new_val; 
      if (min_val > new_val) min_val = new_val; 
    }
    if (max_val != min_val) {
      scale = maxVal()/(max_val-min_val);
    } else {
      scale = 1.0;
    }
    pedestal = -scale*min_val;
  } 
  //debug
  printf("scale %f, pedestal %f\n",scale,pedestal);
  fflush(stdout);
  //copy the matrix and then fill in if they are 
  //greater
  int maxX=(mcol>width()) ? width() : mcol;
  int maxY=(mr>height()) ? height() : mr;
  for(int x=0;x<maxX;x++)
    for(int y=0;y<maxY;y++) {      
      auxv=(float)m->values[y*mcol + x]*scale+pedestal;
      if (auxv < 0) auxv = 0;
      else if (auxv > maxVal()) auxv = maxVal();
      Pixel(x,y,band) = auxv; 
    }

  //if the image is bigger, copy pixels
  for (int x=maxX; x<width();x++)
    for (int y=maxY;y<height();y++) {
      Pixel(x,y,band) = Pixel(2*maxX-x-1,2*maxY-y-1,band);
    }
}


inline
void ColorImage::saveBandPNG(const char * filename, int band)
{
  int mdim=1;
  int nbands=_dim;
  int d=band;
#ifndef USE_LIBPNG
  printf("PNG support not compiled in!\n");
#else
  if (band > nbands)
    {
      printf("saving PNG band with band number greater than the image dimension!!\n");
      return;
    }

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL)
    {
      printf("Can't open output PNG file %s\n",filename);
      return;
    }

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
						NULL, NULL, NULL);
  if (!png_ptr)
    {
      printf("can't allocate PNG structure\n");
      fclose(fp);
      return;
    }
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr,
			       (png_infopp)NULL);
      printf("can't allocate PNG structure\n");
      fclose(fp);
      return;
    }
  
  if (setjmp(png_ptr->jmpbuf))
    {
      printf("error saving PNG file %s (detected at setjmp)\n",filename);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      fclose(fp);
      return;
    }

  png_init_io(png_ptr, fp);

  png_set_filter(png_ptr, 0, PNG_FILTER_NONE | PNG_FILTER_SUB | 
		 PNG_FILTER_PAETH);
  
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  
  //dimension is for sure 1
  png_set_IHDR(png_ptr, info_ptr, _width, _height, 8, 
	       mdim == 1 ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	       PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  png_bytep *row_pointers = new png_bytep[_height];
	  
  if (row_pointers == NULL)
    {
      printf("Error\n");
      exit(1);
    }
  GLubyte * temp_pixels = new GLubyte[mdim*_width*_height];

  // copy the pixels

  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++){      
      float val = Pixel(x,y,d)*0xFF/maxVal();
      if (val < 0) val = 0;
      else if (val > 255) val = 255;
      temp_pixels[mdim*(x+(_height-y-1)*_width)] = (GLubyte)val;
    }

  for(int i=0;i<_height;i++)
    row_pointers[i] = &temp_pixels[mdim*i*_width];

  // save the image
  
  png_write_image(png_ptr, row_pointers);

  // finish the file
  png_write_end(png_ptr, info_ptr);
  
  // free up memory
  png_destroy_write_struct(&png_ptr, &info_ptr);
  
  delete [] row_pointers;
  delete [] temp_pixels;
  
  fclose(fp);
  
  printf("Saved %s \n",filename);

#endif
}

*/
template <class T>
void Image<T>::copy(const Image & img) 
{
  /*
    if (_width != img._width || _height != img._height)
    {
    delete [] _pixels;

    _width = img._width;
    _height = img._height;
      
    _pixels = new T[_width*_height*_dim];
    if (_pixels == NULL)
    {
    printf("Can't allocate image\n");
    exit(1);
    }
    }
  */
  assert(_width == img._width && _height == img._height);
  assert(_dim == img._dim);
  memcpy(_pixels,img._pixels,_width*_height*_dim*sizeof(T));
  _colorspace = img._colorspace;
}

template <class T>
void Image<T>::set(T val)
{
  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)
      for(int d=0;d<dim();d++)
	Pixel(x,y,d) = val;
}

template <class T>
T Image<T>::Pixel(int x,int y,int n) const
{ 
  assert(inside(x,y,n)); 
  return _pixels[_dim*(x+y*_width)+n];
}

template <class T>
T & Image<T>::Pixel(int x,int y,int n) 
{
  assert(inside(x,y,n)); 
  return _pixels[_dim*(x+y*_width)+n];
}

template <class T>
void Image<T>::setPixel(int x,int y, T val)
{
  assert(_dim == 1);
  Pixel(x,y,0) = val;
}

template <class T>
void Image<T>::setPixel(int x,int y, T val1, T val2)
{
  assert(_dim == 2);
  Pixel(x,y,0) = val1;
  Pixel(x,y,1) = val2;
}

template <class T>
void Image<T>::setPixel(int x,int y, T val1, T val2, T val3)
{
  assert(_dim == 3);
  Pixel(x,y,0) = val1;
  Pixel(x,y,1) = val2;
  Pixel(x,y,2) = val3;
}

template <class T>
void Image<T>::setPixel(int x,int y, T val1, T val2, T val3, T val4)
{
  assert(_dim == 4);
  Pixel(x,y,0) = val1;
  Pixel(x,y,1) = val2;
  Pixel(x,y,2) = val3;
  Pixel(x,y,3) = val4;
}

template <class T>
void Image<T>::copyPixel(int x,int y,const Image & src,int srcX,int srcY)
{
  for(int d=0;d<_dim;d++)
    Pixel(x,y,d) = src.Pixel(srcX,srcY,d);
}

template <class T>
bool Image<T>::inside(int i) const
{
  return i>=0 && i<_width*_height*_dim;
}

template <class T>
bool Image<T>::inside(int x,int y) const
{
  return x>=0 && x<_width && y>=0 && y<_height;
}

template <class T>
bool Image<T>::inside(int x,int y,int n) const
{
  return inside(x,y) && n>=0 && n < _dim;
}

template <class T>
void Image<T>::draw(int x,int y)
{
  _winx = x;
  _winy = y;
  glRasterPos2i(x,y);
  if (_dim == 4)
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  else
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glDrawPixels(_width,_height,(GLenum)myformat(),(GLenum)mytype(),_pixels);
}

template <class T>
void Image<T>::read(int x,int y)
{
  if (_dim == 4)
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
  else
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glReadPixels(x,y,_width,_height,(GLenum)myformat(),(GLenum)mytype(),_pixels);
}  

template <class T>
void Image<T>::scaleDraw(int x,int y,float bias, float scale)
{
  _winx = x;
  _winy = y;
  glRasterPos2i(x,y);
 
  glPixelTransferf(GL_RED_SCALE,scale);
  glPixelTransferf(GL_GREEN_SCALE,scale);
  glPixelTransferf(GL_BLUE_SCALE,scale);
  glPixelTransferf(GL_RED_BIAS,bias);
  glPixelTransferf(GL_GREEN_BIAS,bias);
  glPixelTransferf(GL_BLUE_BIAS,bias);

  glDrawPixels(_width,_height,(GLenum)myformat(),(GLenum)mytype(),_pixels);

  glPixelTransferf(GL_RED_SCALE,1);
  glPixelTransferf(GL_GREEN_SCALE,1);
  glPixelTransferf(GL_BLUE_SCALE,1);
  glPixelTransferf(GL_RED_BIAS,0);
  glPixelTransferf(GL_GREEN_BIAS,0);
  glPixelTransferf(GL_BLUE_BIAS,0);
}

template <class T>
void Image<T>::sourcesDraw(int x,int y,int sourceWidth,
			   int sourceHeight)
{
  _winx = x;
  _winy = y;
  glRasterPos2i(x,y);

  glPixelTransferf(GL_RED_SCALE,MAX_SHORT_VALUE/sourceWidth);
  glPixelTransferf(GL_GREEN_SCALE,MAX_SHORT_VALUE/sourceHeight);

  glDrawPixels(_width,_height,(GLenum)myformat(),(GLenum)mytype(),_pixels);

  glPixelTransferf(GL_RED_SCALE,1);
  glPixelTransferf(GL_GREEN_SCALE,1);
}

template <class T>
int Image<T>::myformat() const
{
  assert(_dim >= 1 && _dim <= 4);
  switch(_dim)
    {
    case 1: return GL_LUMINANCE;
    case 2: return GL_LUMINANCE_ALPHA;
    case 3: return GL_RGB;
    case 4: return IMAGE_MODE;
    default: 
      printf("error unknown format (dim = %d)\n",_dim); exit(1);
      return -1;
    }
}
	  
template <class T>
Image<T> * Image<T>::luminance()
{
  assert(_dim == 3);

  Image * img = new Image(_width,_height,1);

  if (img == NULL)
    {
      printf("Out of memory computing luminance image\n");
      exit(1);
    }
  
  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++)
      {
	double r = Pixel(x,y,0)/maxVal();
	double g = Pixel(x,y,1)/maxVal();
	double b = Pixel(x,y,2)/maxVal();
	
	img->Pixel(x,y) = ::luminance(r,g,b);
      }

  return img;
}

template <class T>
Image<T> * Image<T>::downsample()
{
  Image * img = new Image(_width/2,_height/2,_dim);

  if (img == NULL)
    {
      printf("Out of memory downsampling image\n");
      exit(1);
    }

  img->SetColorspace( _colorspace );

  double kernel[3][3] = { { .0625, .1250, .0625 } ,
			  { .1250, .2500, .1250 } ,
			  { .0625, .1250, .0625 } };

  int kwidth = 3;
  int kheight = 3;
  int koriginX = 1;
  int koriginY = 1;
     
  for(int x=0;x<_width/2;x++)
    for(int y=0;y<_height/2;y++)
      for(int d=0;d<_dim;d++)
	{
	  T val = 0; 
	  float weight = 0;
	     
	  for(int i=0;i<kwidth;i++)
	    for(int j=0;j<kheight;j++)
	      {
		int nx = 2*x + i - koriginX; 
		int ny = 2*y + j - koriginY;
		   
		if (nx >= _width || ny >= _height)
		  break;
		   
		if (nx < 0 || ny < 0)
		  continue;
		   
		float w = kernel[i][j];
		val += w*Pixel(nx,ny,d);
		weight += w;
	      }
	     
	  assert(weight != 0);
	     
	  img->Pixel(x,y,d) = val/weight;
	}
     
  return img;
}

template <class T>
Image<T> * Image<T>::upsample()
{
  Image * img = new Image(_width*2+1,_height*2+1,_dim);

  if (img == NULL)
    {
      printf("Out of memory upsampling image\n");
      exit(1);
    }

  img->SetColorspace( _colorspace );
  double kernel[3][3] = { { .0625, .1250, .0625 } ,
			  { .1250, .2500, .1250 } ,
			  { .0625, .1250, .0625 } };

  int kwidth = 3;
  int kheight = 3;
  int koriginX = 1;
  int koriginY = 1;

  for(int x=0;x<_width*2+1;x++)
    for(int y=0;y<_height*2+1;y++)
      for(int d=0;d<_dim;d++)
	{
	  T val = 0; 
	  float weight = 0;
	     
	  for(int i=0;i<kwidth;i++)
	    for(int j=0;j<kheight;j++)
	      {
		int nx = x/2 + i - koriginX; 
		int ny = y/2 + j - koriginY;
		   
		if (nx >= _width || ny >= _height)
		  break;
		   
		if (nx < 0 || ny < 0)
		  continue;
		   
		float w = kernel[i][j];
		val += w*Pixel(nx,ny,d);
		weight += w;
	      }
	     
	  assert(weight != 0);
	     
	  img->Pixel(x,y,d) = val/weight;
	}
     
  return img;
}

inline
void ColorImage::loadPNM( const char * filename)
{
  FILE *fp = fopen(filename,"rt");

  if (fp == NULL)
    {
      printf("Can't open PNM %s\n",filename);
      exit(1);
    }

  int maxv,ftype;

  fscanf(fp,"P%d\n%d %d\n%d\n",&ftype,&_width,&_height,&maxv);

  if (ftype != ASCII_PPM && ftype != RAW_PPM &&
      ftype != ASCII_PGM && ftype != RAW_PGM)
    {
      printf("Unknown input file type (%s)\n",filename);
      exit(1);
    }

  _pixels = new GLfloat[3*_width*_height];

  assert(_pixels != NULL);

  float scale = maxVal()/255.0;

  int x,y;

  switch(ftype)
    {
    case ASCII_PPM:
      for(y=_height-1;y>=0;y--)
	{
	  for(x=0;x<_width;x++)
	    {
	      int r,g,b;

	      fscanf(fp,"%d %d %d",&r,&g,&b);
	      
	      Pixel(x,y,RED_OFFSET) = r*scale;
	      Pixel(x,y,GREEN_OFFSET) = g*scale;
	      Pixel(x,y,BLUE_OFFSET) = b*scale;

#ifdef ALPHA_OFFSET
	      if (_dim > 3)
		Pixel(x,y,ALPHA_OFFSET) = maxVal();
#endif
	    }
	}
      break;

    case RAW_PPM:
      for(y=_height-1;y>=0;y--)
	{
	  for(x=0;x<_width;x++)
	    {
	      GLubyte r,g,b;
	      
	      fscanf(fp,"%c%c%c",&r,&g,&b);
	      
	      Pixel(x,y,RED_OFFSET) = r*scale;
	      Pixel(x,y,GREEN_OFFSET) = g*scale;
	      Pixel(x,y,BLUE_OFFSET) = b*scale;

#ifdef ALPHA_OFFSET	      
	      if (_dim > 3)
		Pixel(x,y,ALPHA_OFFSET) = maxVal();
#endif
	    }
	}
      break;

    case ASCII_PGM:
      for(y=_height-1;y>=0;y--)
	{
	  for(x=0;x<_width;x++)
	    {
	      int g;

	      fscanf(fp,"%d ",&g);
	      
	      Pixel(x,y,RED_OFFSET) = g*scale;
	      Pixel(x,y,GREEN_OFFSET) = g*scale;
	      Pixel(x,y,BLUE_OFFSET) = g*scale;

#ifdef ALPHA_OFFSET
	      if (_dim > 3)
		Pixel(x,y,ALPHA_OFFSET) = maxVal();
#endif
	    }
	}
      break;
      
    case RAW_PGM:
      for(y=_height-1;y>=0;y--)
	{
	  for(x=0;x<_width;x++)
	    {
	      GLubyte g;
	      
	      fscanf(fp,"%c",&g);
	      
	      Pixel(x,y,RED_OFFSET) = g*scale;
	      Pixel(x,y,GREEN_OFFSET) = g*scale;
	      Pixel(x,y,BLUE_OFFSET) = g*scale;

#ifdef ALPHA_OFFSET	      
	      if (_dim > 3)
		Pixel(x,y,ALPHA_OFFSET) = maxVal();
#endif
	    }
	}
      break;

    default:
      printf("error\n");
      exit(1);
    }

  if (maxv != 255)
    printf("Warning: maximum intensity in texture != 255\n");

  printf("PNM image: %d x %d\n",_width,_height);

  fflush(stdout);

  fclose(fp);
}

inline
void ColorImage::savePNG(const char * filename)
{
#ifndef USE_LIBPNG
  printf("PNG support not compiled in!\n");
#else

  if (_dim != 1 && _dim != 3)
    {
      printf("saving PNG with image dimension other than 1 or 3 not supported right now\n");
      return;
    }

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL)
    {
      printf("Can't open output PNG file %s\n",filename);
      return;
    }

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
						NULL, NULL, NULL);
  if (!png_ptr)
    {
      printf("can't allocate PNG structure\n");
      fclose(fp);
      return;
    }
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr,
			       (png_infopp)NULL);
      printf("can't allocate PNG structure\n");
      fclose(fp);
      return;
    }

  if (setjmp(png_ptr->jmpbuf))
    {
      printf("error saving PNG file %s (detected at setjmp)\n",filename);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      fclose(fp);
      return;
    }

  png_init_io(png_ptr, fp);

  png_set_filter(png_ptr, 0, PNG_FILTER_NONE | PNG_FILTER_SUB | 
		 PNG_FILTER_PAETH);
  
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
  
  png_set_IHDR(png_ptr, info_ptr, _width, _height, 8, 
	       _dim == 1 ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
	       PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  png_bytep *row_pointers = new png_bytep[_height];
	  
  if (row_pointers == NULL)
    {
      printf("Error\n");
      exit(1);
    }

  GLubyte * temp_pixels = new GLubyte[_dim*_width*_height];

  // copy the pixels

  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++)
      for(int d=0;d<_dim;d++)
	{
	  float val = Pixel(x,y,d)*0xFF/maxVal();
	  if (val < 0) val = 0;
	  else if (val > 255) val = 255;

	  temp_pixels[_dim*(x+(_height-y-1)*_width)+d] = (GLubyte)val;
	}


  for(int i=0;i<_height;i++)
    row_pointers[i] = &temp_pixels[_dim*i*_width];

  // save the image

  png_write_image(png_ptr, row_pointers);

  // finish the file
  png_write_end(png_ptr, info_ptr);

  // free up memory
  png_destroy_write_struct(&png_ptr, &info_ptr);

  delete [] row_pointers;
  delete [] temp_pixels;

  fclose(fp);
  
  printf("Saved %s \n",filename);

#endif
}

inline
void ColorImage::loadPNG( const char * filename)
{
#ifndef USE_LIBPNG
  printf("PNG support not compiled in!\n");
#else

  FILE *fp = fopen(filename, "rb");
  if (fp == NULL)
    {
      printf("Can't open PNG file %s\n",filename);
      exit(1);
    }

  int numheaderbytes = 8;  // anything from 1 to 8

  png_bytep header = new png_byte[numheaderbytes];

  fread(header,1,numheaderbytes, fp);

  // check if this is a PNG file
  if (png_sig_cmp(header, 0, numheaderbytes))
    {
      printf("Invalid PNG file %s.  Header:\n",filename);
      for(int i=0;i<numheaderbytes;i++)
	printf("%02X ",header[i]);
      exit(1);
    }

  // allocate PNG structures
  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					       NULL, NULL, NULL);
  
  if (png_ptr == NULL)
    {
      printf("error allocating PNG structures\n");
      exit(1);
    }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (info_ptr == NULL)
    {
      printf("Error creating PNG info\n");
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      fclose(fp);
      exit(1);
    }

  //  png_infop end_info;

  if (setjmp(png_ptr->jmpbuf))
    {
      printf("error reading file %s (detected at setjmp)\n",filename);
      png_destroy_read_struct(&png_ptr, &info_ptr,NULL);
      fclose(fp);
      exit(1);
    }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, numheaderbytes);


  png_read_info(png_ptr, info_ptr);

  int color_type, bit_depth;

  png_uint_32 w, h;

  png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type,
	       NULL, NULL, NULL);  

  _width = w; _height = h;

  printf("%s: %d x %d PNG\n",filename,_width, _height);

  // convert paletted images to RGB
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);
	
  // convert grayscale to 8 bits per pixel
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
    png_set_gray_1_2_4_to_8(png_ptr);

  // convert 16 bit images to 8 bit
  if (bit_depth == 16)
    png_set_strip_16(png_ptr);
  
  // remove alpha channel
  if (color_type & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png_ptr);

  // convert grayscale to RGB
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  // prepare for interlacing
  int number_of_passes = png_set_interlace_handling(png_ptr);
  
  // update info structure
  png_read_update_info(png_ptr, info_ptr);

  // allocate space
  GLubyte * temp_pixels = new GLubyte[3*_width*_height];

  _pixels = new GLfloat[3*_width*_height];

  if (_pixels == NULL)
    {
      printf("unable to allocate %d x %d image\n",_width,_height);
      exit(1);
    }

  png_bytep * row_pointers = new png_bytep[_height];
	  
  if (row_pointers == NULL)
    {
      printf("Error\n");
      exit(1);
    }

  for(int i=0;i<_height;i++)
    row_pointers[i] = &temp_pixels[3*i*_width];

  // read the image data

  png_read_image(png_ptr, row_pointers);

  // copy the pixels

  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++)
      for(int d=0;d<3;d++)
	Pixel(x,y,d) = temp_pixels[3*(x+(_height-y-1)*_width)+d]*
	  (maxVal()/255.0);

  // free up memory
  delete [] row_pointers;
  delete [] temp_pixels;

  png_destroy_read_struct(&png_ptr, &info_ptr,NULL);

#endif
}


template <class T>
bool Image<T>::loadData( const char *filename)
{
  FILE *fp = fopen(filename,"rb");
  
  if (fp == NULL)
    {
      printf("Can't open file %s\n",filename);
      return false;
    }

  fscanf(fp,"%d %d %d\n",&_width,&_height,&_dim);

  int numBytes = _dim*_width*_height;
  _pixels = new T[ numBytes ];

  if (_pixels == NULL)
    {
      fclose(fp);
      printf("error can't allocate %d x %d x %d array\n",_width,_height,_dim);
      return false;
    }

  fread(_pixels,sizeof(T),_width*_height*_dim,fp);
  
  fclose(fp);
  return true;
}

template <class T>
bool Image<T>::saveData(const char *filename)
{
  FILE * fp = fopen(filename,"wb");
  if (fp == NULL)
    {
      printf("Can't open %s for writing\n");
      return false;
    }
  
  fprintf(fp,"%d %d %d\n",_width,_height,_dim);
  fwrite(_pixels,sizeof(T),_width*_height*_dim,fp);

  fclose(fp);

  printf("Saved %s\n",filename);

  return true;
}


template <class T>
bool Image<T>::loadBMP( const char *filename )
{
  CBmpReader reader;
  if( !reader.ReadFile( filename ) )
    {
      // TODO: set some 'invalid' bit or something
      return false;
    }

  int bytesPerPixel = reader.BytesPerPixel();
  _width = reader.Width();
  _height = reader.Height();

  assert( bytesPerPixel <= 4 );

  // allocate memory
  if( _pixels )
    delete[] _pixels;

  int numBytes = _dim*_width*_height;
  _pixels = new T[ numBytes ];
  
  
  const unsigned char *src = reader.PixelData();
  //  GLuint *dest = (GLuint *)_pixels;
  
  float scale = maxVal()/255.0;

  for( int iy = 0; iy < _height; iy++ )
    {
      for( int ix = 0; ix < _width; ix++ )
	{
	  Pixel(ix,iy,RED_OFFSET) = src[2]*scale;
	  Pixel(ix,iy,GREEN_OFFSET) = src[1]*scale;
	  Pixel(ix,iy,BLUE_OFFSET) = src[0]*scale;

#ifdef ALPHA_OFFSET      
	  if (_dim > 3)
	    Pixel(ix,iy,ALPHA_OFFSET) = (bytesPerPixel==4?src[3]:maxVal());
#endif

	  src += bytesPerPixel;
	}
    }
  
  return true;
}

ColorImage::Image(const char *filename)  : 
  _pixels(NULL), _width(-1), _height(-1), _dim(3), _winx(-1), _winy(-1), _colorspace( RGB_SPACE )
{
  //  assert(_dim == 3 || _dim == 4);

  ImageFileFormat format = getFileFormat( filename );

  switch (format)
    {
    case BMP_FORMAT:
      if (!loadBMP(filename))
	{
	  printf("Can't open file %s\n",filename);
	  exit(1);
	}
      return;

    case PNM_FORMAT:
      loadPNM(filename);
      return;
    case RAW_FORMAT:
      loadData(filename);
      return;
#ifdef USE_LIBPNG
    case PNG_FORMAT:
      loadPNG(filename);
      return;
#endif
    default:
      printf("Unknown input file type (%s)\n",filename);
      exit(1);
    }
}

template <class T>
Image<T>::Image<T>(const char * filename) : 
  _pixels(NULL), _width(-1), _height(-1), _dim(3), _winx(-1), _winy(-1), _colorspace( RGB_SPACE )
{
  //  assert(_dim == 3 || _dim == 4);

  ImageFileFormat format = getFileFormat( filename );

  switch (format)
    {
    case RAW_FORMAT:
      loadData(filename);
      return;
      /*
    case BMP_FORMAT:
      if (!loadBMP(filename))
	{
	  printf("Can't open file %s\n",filename);
	  exit(1);
	}
      return;

    case PNM_FORMAT:
      loadPNM(filename);
      return;
#ifdef USE_LIBPNG
    case PNG_FORMAT:
      loadPNG(filename);
      return;
#endif
*/
    default:
      printf("Unknown input file type for this type (%s)\n",filename);
      exit(1);
    }
}  

template <class T>
bool Image<T>::winxyToImagexy(int wx,int wy, int & ix, int &iy)
{
  if (!(_winx >= 0 && _winy >= 0))
	  return false;

  ix = wx - _winx;
  iy = wy - _winy;

  return ix >=0 && iy >= 0 && ix < _width && iy < _height;
}

template <class T>
void Image<T>::imagexyToWinxy(int ix,int iy, int & wx, int &wy)
{
  assert(_winx >= 0 && _winy >= 0);

  wx = ix + _winx;
  wy = iy + _winy;
}

template <class T>
void Image<T>::insert(Image & im, int x0,int y0,bool rescale)
{
  assert(x0>=0 &&x0+im.width() <= width());
  assert(y0>=0 &&y0+im.height() <= height());

  if (dim() == im.dim() && !rescale)
    {
      for(int y=0;y<im.height();y++)
	memcpy(&Pixel(x0,y+y0,0),&im.Pixel(0,y,0),im.width()*dim()*sizeof(T));
      return;
    }

  // clear the memory
  if (im.dim() < dim())
  {
    for(int y=0;y<im.height();y++)
      memset(&Pixel(x0,y+y0,0),0,im.width()*dim()*sizeof(T));
  }    
  
  if (rescale)
    {
      for(int y=0;y<im.height();y++)
	for(int x=0;x<im.width();x++)
	  {
	    int d;
	    for(d=0;d<dim()&&d<im.dim();d++)
	      Pixel(x+x0,y+y0,d) = (im.maxVal()+im.Pixel(x,y,d))/2;
	    for(;d<dim();d++)
	      Pixel(x+x0,y+y0,d) = (im.maxVal()+im.Pixel(x,y,0))/2;
	  }
    }
  else
    {
      for(int y=0;y<im.height();y++)
	for(int x=0;x<im.width();x++)
	  {
	    int d=0;
	    for(d=0;d<dim()&&d<im.dim();d++)
	      Pixel(x+x0,y+y0,d) = im.Pixel(x,y,d);
	    for(;d<dim();d++)
	      Pixel(x+x0,y+y0,d) = im.Pixel(x,y,0);
	  }
    }
}



template<class T>
void Image<T>::meanVariance(T & mean, T & variance)
{
  assert(dim() == 1);

  mean = 0;
  variance = 0;

  int N = width()*height();

  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)
      {
	T val = Pixel(x,y);
	mean += val/N;
	variance += val*val/N;
      }

  variance -= mean*mean;
}

template<class T>
void Image<T>::meanCovariance(CVector<double> & mean, 
			      CMatrix<double> & covar) 
{
  assert(mean.Length() == dim() && covar.columns() == dim() &&
	 covar.rows() == dim());

  int N = width()*height();

  mean.clear();
  covar.clear();

  for(int x=0;x<width();x++)
    for(int y=0;y<height();y++)
      {
	for(int d=0;d<dim();d++)
	  mean[d] += Pixel(x,y,d)/N;

	for(int i=0;i<dim();i++)
	  for(int j=0;j<dim();j++)
	    covar.get(i,j) += Pixel(x,y,i)*Pixel(x,y,j)/N;	      
      }

  for(int i=0;i<dim();i++)
    for(int j=0;j<dim();j++)
      covar.get(i,j) -= mean[i]*mean[j];
}

template<class T>
void Image<T>::linearMap(int x,int y,const CMatrix<double> & gain, 
			 const CVector<double> & bias)
{
  assert(bias.Length() == dim() && gain.rows() == dim() && 
	 gain.columns() == dim());
  
  CVector<double> val(dim());
  int d;
  for(d=0;d<dim();d++)
    val[d] = Pixel(x,y,d);

  val = gain * val + bias;

  for(d=0;d<dim();d++)
    Pixel(x,y,d) = val[d];
}

//converts image to YIQ
template <class T>
void Image<T>::RGBtoYIQ(Image<T> &img)
{
	assert(img.dim()==3 && dim()==img.dim());
	for(int x=0;x<width();x++)	
		for(int y=0;y<height();y++){
			img.Pixel(x,y,0)=.299*Pixel(x,y,0)+.587*Pixel(x,y,1)+.114*Pixel(x,y,2);
			img.Pixel(x,y,1)=.596*Pixel(x,y,0)-.274*Pixel(x,y,1)-.322*Pixel(x,y,2);
			img.Pixel(x,y,2)=.211*Pixel(x,y,0)-.523*Pixel(x,y,1)+.312*Pixel(x,y,2);
		}
}

//converts the image to RGB
template <class T>
void Image<T>::RGBtoGrayScale(Image<T> &img)
{
  assert(img.dim()==1);
  //copy if the input image is gray scale already
  if (dim()==1) {
    img.copy(*this);
	return;
 } else if (dim()==3) {
	for(int x=0;x<width();x++)
	  for(int y=0;y<height();y++){
	    img.Pixel(x,y,0)=.299*Pixel(x,y,0)+.587*Pixel(x,y,1)+.114*Pixel(x,y,2);
	  } // for
  }
}

template<class T>
void 
Image<T>::
applyGainBias(float gain,float bias)
{
  assert(dim() == 1);

  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++)
      Pixel(x,y,0) = gain*Pixel(x,y,0) + bias;
}

template<class T>
void 
Image<T>::
applyGainBias(const CMatrix<double>& gain,const CVector<double> & bias)
{
  for(int x=0;x<_width;x++)
    for(int y=0;y<_height;y++)
      applyGainBias(x,y,gain,bias);
}
  

template<class T>
void Image<T>::applyGainBias(int x,int y,
			     const CMatrix<double> & gain, 
			     const CVector<double> & bias)
{
  assert(bias.Length() == dim() && gain.rows() == dim() && 
	 gain.columns() == dim());
  
  CVector<double> val(dim());
  int d;
  for(d=0;d<dim();d++)
    val[d] = Pixel(x,y,d);

  val = gain * val + bias;

  for(d=0;d<dim();d++)
    Pixel(x,y,d) = val[d];
}

template<class T>
void Image<T>::SetColorspace( COLORSPACE c )
{
  _colorspace = c;
}

// I know, i should assign a conversion function to a pointer at the beginning, and
// loop through the pixels, calling the function for each pixel, but i don't know
// the syntax when it's a templated function. I'm sure it's obvious, though...
template<class T>
void Image<T>::ConvertToColorspace( COLORSPACE c )
{
  printf( "ConvertToColorspace() called: converting from %d to %d\n", _colorspace, c );

  if( _colorspace == c )
    return;

  // make sure image has only 3 channels
  if( _dim != 3 )
  {
    fprintf( stderr, "ConvertToColorspace() -- needs image with 3 channels, not %d\n", _dim );
    return;
  }

  for( int iy = 0; iy < height(); iy++ )
  {
    T* row = &Pixel( 0, iy, 0 );

    for( int ix = 0; ix < width(); ix++, row += _dim )
    {
      switch( _colorspace )
      {
      case RGB_SPACE:
	RGBtoXYZ( row[0], row[1], row[2] );
	break;
	
      case XYZ_SPACE:
	// nothing
	break;

      case YIQ_SPACE:
	//	YIQtoXYZ( row[0], row[1] );
	break;
	
      case LUV_SPACE:
	LuvtoXYZ( row[0], row[1], row[2] );
	break;

      case LAB_SPACE:
	LabtoXYZ( row[0], row[1], row[2] );
	break;
      }

      if( c == XYZ_SPACE )
	continue;

      switch( c )
      {
      case RGB_SPACE:
	XYZtoRGB( row[0], row[1], row[2] );
	break;
	
      case XYZ_SPACE:
	// nothing
	break;

      case YIQ_SPACE:
	//	XYZtoYIQ( row[0], row[1] );
	break;
	
      case LUV_SPACE:
	XYZtoLuv( row[0], row[1], row[2] );
	break;

      case LAB_SPACE:
	XYZtoLab( row[0], row[1], row[2] );
	break;
      }
    }
  }

  _colorspace = c;
}


template<class T>
void Image<T>::PrintExtrema( void )
{
  int id;

  T minVal[10];
  T maxVal[10];

  for( id = 0; id < _dim; id++ )
  {
    minVal[id] = Pixel(0,0,id);
    maxVal[id] = Pixel(0,0,id);
  }


  for( int iy = 0; iy < height(); iy++ )
    for( int ix = 0; ix < width(); ix++ )
      for( int id = 0; id < _dim; id++ )
      {
	minVal[id] = min( minVal[id], Pixel( ix, iy, id ) );
	maxVal[id] = max( maxVal[id], Pixel( ix, iy, id ) );
      }

  printf( "min: " );
  for( id = 0; id < _dim; id++ )
    printf( "%f ", (float)minVal[id] );

  printf( "\tmax: " );
  for( id = 0; id < _dim; id++ )
    printf( "%f ", (float)maxVal[id] );

  printf( "\n" );
}

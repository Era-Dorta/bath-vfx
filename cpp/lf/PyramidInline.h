template <class TImage>
Pyramid<TImage>::
Pyramid( int levels, int width, int height, int dim ) : 
  _imgs(NULL), _levels(levels), _ownsFine(true)
{
  makePyramid( levels, width, height, dim, GAUSSIAN_PYRAMID );
}

/*
template <class TImage>
Pyramid<TImage>::
Pyramid(int height,TImage * FineImage) : 
  _imgs(NULL), _levels(height), _ownsFine(false)
{
  makePyramid( height, FineImage );
}
*/

template <class TImage>
Pyramid<TImage>::
Pyramid(const Pyramid & p)
{
  _myType = p._myType;
  _levels = p._levels;
  _ownsFine = true;

  _imgs = new TImage*[_levels];
  for(int i=0;i<_levels;i++)
    _imgs[i] = new TImage(*(p._imgs[i]));
}


template <class TImage>
void Pyramid<TImage>::
makePyramid(int height,TImage * FineImage,PyramidType ptype) 
{
  _myType = ptype;

  if (_imgs != NULL)
    {
      if( _ownsFine )
	delete _imgs[0];
		  
      for(int i=1;i<_levels-1;i++)
	delete _imgs[i];
      delete [] _imgs;
    }
  _ownsFine = false;
  assert(FineImage->width() >= 0 && FineImage ->height() >= 0);

  //		assert(height == 1);
  _levels = height;
  _imgs = new TImage*[_levels];
  _imgs[0] = FineImage;
  
  for(int i=1;i<_levels;i++)
    _imgs[i] = _imgs[i-1]->downsample();

  if (_myType == LAPLACIAN_PYRAMID) {
    for(int i=0;i<_levels-1;i++) {
	TImage * ups = _imgs[i+1]->upsample();

	for(int x=0;x<_imgs[i]->width();x++)
	  for(int y=0;y<_imgs[i]->height();y++)
	    for(int d=0;d<_imgs[i]->dim();d++)
	      //	      _imgs[i]->Pixel(x,y,d) -= _imgs[i+1]->Pixel(x/2,y/2,d);
	      _imgs[i]->Pixel(x,y,d) -= ups->Pixel(x,y,d);
	
	delete ups;
	} //for 
  } // if
}

/*
template <class TImage>
void Pyramid<TImage>::
makePyramid(int height,TImage * FineImage,PyramidType ptype,
	    PFILTER pf) 
{
  int i, j, mcols, mrows;
  TImage *lpTImage;
  //debug-->save the coarsest level images as png
  char bandbasefname[]="steerOutputCoarsest";
  char bandfname[80];

  //It creates one image per level, with as many dimensions as 
  //subbands in the steerable filter
  _myType = ptype;
  _levels = height;
#ifdef STEERDEBUG
  //debug
  printf("Inside makePyramid : image type %d\n", _myType);
#endif
  //call makePyramid to initialize the sizes
  //this is because the DOWNSAMPLE/UPSAMPLE versions of steerable
  //filters makes the images 1 byte smaller than if we just
  //divided them by 2
  if (_myType==STEERABLE_PYRAMID) {
    assert(pf!=NULL);
    makePyramid(_levels,FineImage->width(),FineImage->height(),
		pf->num_orientations,ptype,pf);    
    // the image needs to be YIQ or RGB
    assert(FineImage->dim()==1 || FineImage->dim()==3);
    //build the steerable pyramid
    MATRIX matrixImg = NewMatrix(FineImage->height(), FineImage->width());
    //RGB
    if (FineImage->dim()==3) {
      FineImage->RGBtoMatrix(matrixImg);
    } else if (FineImage->dim()==1) {
      //Grayscale
      FineImage->GraytoMatrix(matrixImg);
    }    
    //convert the input image to MATRIX and create the pyramid
    _steerPyr = CreatePyramid(matrixImg, pf, _levels);
    
    for (i=0;i<_levels;i++) {	
      lpTImage = _imgs[i];
      for (j=0; j<pf->num_orientations;j++) {
	//rescale and then display
	_imgs[i]->setImgBand(_steerPyr->levels[i]->subband[j], j, 1);        
      }
    } //for i
    
    for (j=0;j<pf->num_orientations;j++) {
      sprintf(bandfname,"%s%d.png",bandbasefname,j);
      _imgs[3]->saveBandPNG(bandfname,j);
    }
    DeleteMatrix(matrixImg);
  } else {
    _imgs = new TImage*[_levels];
    _imgs[0] = FineImage;
	
    for(i=1;i<_levels;i++)
      _imgs[i] = _imgs[i-1]->downsample();
	
    if (_myType == LAPLACIAN_PYRAMID) {
      for(i=0;i<_levels-1;i++) {
	TImage * ups = _imgs[i+1]->upsample();
	    
	for(int x=0;x<_imgs[i]->width();x++)
	  for(int y=0;y<_imgs[i]->height();y++)
	    for(int d=0;d<_imgs[i]->dim();d++)
	      //	      _imgs[i]->Pixel(x,y,d) -= _imgs[i+1]->Pixel(x/2,y/2,d);
	      _imgs[i]->Pixel(x,y,d) -= ups->Pixel(x,y,d);
	
	delete ups;
      }
    } //if
  } // else
}
 */

template <class TImage>
void Pyramid<TImage>::reconstruct()
{
  //only works for Laplacian and Steerable
  if (_myType == LAPLACIAN_PYRAMID) {
    for(int i=_levels-2;i>=0;i--){
      TImage * ups = _imgs[i+1]->upsample();      
      for(int x=0;x<_imgs[i]->width();x++)
	for(int y=0;y<_imgs[i]->height();y++)
	  for(int d=0;d<_imgs[i]->dim();d++)
	    _imgs[i]->Pixel(x,y,d) += ups->Pixel(x,y,d);      
      delete ups;
    }
  } else if (_myType == STEERABLE_PYRAMID) {
    //reconstruct steerable
  } else {
    printf("The reconstruction function only works for Laplacian or Steerable. _myType = %d\n", _myType);
  }
  _myType = GAUSSIAN_PYRAMID;
}


template <class TImage>
void Pyramid<TImage>::
makePyramid(int levels,int imWidth,int imHeight,int imDim,PyramidType ptype) 
{
  assert(imDim > 0);
	// if steerable pyramid return because the filter is missing
  assert(ptype==STEERABLE_PYRAMID);
  _myType = ptype;
  if (_imgs != NULL)
    {
      if( _ownsFine )
	delete _imgs[0];
      for(int i=1;i<_levels-1;i++)
	delete _imgs[i];
      delete [] _imgs;
    }

  _levels = levels;
  _ownsFine = true;
  _imgs = new TImage*[_levels];
  _imgs[0] = new TImage(imWidth,imHeight,imDim);
  for( int i = 1; i < levels; i++ )
    {
      imWidth /= 2;
      imHeight /= 2;
      _imgs[i] = new TImage( imWidth, imHeight, imDim );
    }
}

/*
template <class TImage>
void Pyramid<TImage>::
makePyramid(int levels,int imWidth,int imHeight,int imDim,
	    PyramidType ptype,PFILTER pf) 
{
#ifdef STEERDEBUG
  //debug
  printf("Inside makePyramid for steerable with levels/imWidth...\n");
  printf("Pyramid type %d \n", ptype);
  fflush(stdout);
#endif
  assert(imDim > 0);
  if (ptype==STEERABLE_PYRAMID)
    assert(pf!=NULL);
  _myType = ptype;
  if (_imgs != NULL){
#ifdef STEERDEBUG
    //debug
    printf("Img are not null %d\n", _ownsFine);
#endif
    fflush(stdout);
    if( _ownsFine )
      delete _imgs[0];
    for(int i=1;i<_levels-1;i++)
      delete _imgs[i];
    delete [] _imgs;
  }

  _levels = levels;
  _ownsFine = true;
  _imgs = new TImage*[_levels];    
  _imgs[0] = new TImage(imWidth, imHeight, imDim);
  for( int i = 1; i < _levels; i++ ){
    imWidth /= 2;
    imHeight /= 2;
    _imgs[i] = new TImage( imWidth, imHeight, imDim );
  }
#ifdef STEERDEBUG
  printf("Exiting makePyramid for steerable\n");
#endif
  fflush(stdout);
}
*/


template <class TImage>
void Pyramid<TImage>::clear()
{
  for( int i = 0; i < _levels; i++ )
    _imgs[i]->clear();
}

template <class TImage>
Pyramid<TImage>::~Pyramid()
{
  if (_imgs != NULL)
    {
      if( _ownsFine )
	delete _imgs[0];

      for(int i=1;i<_levels-1;i++)
	delete _imgs[i];
      delete [] _imgs;
    }
}


template <class TImage>
void Pyramid<TImage>::drawColumn(int x,int y,bool rescale) const
{
  int x1=x,y1=y;
  for(int i=_levels-1;i>=0;i--)
    {
      if ((_myType == GAUSSIAN_PYRAMID || i==_levels-1) && !rescale)
	_imgs[i]->draw(x1,y1);
      else
	_imgs[i]->scaleDraw(x1,y1,.5,.5);

      y1 += (_imgs[i]->height() + 5);
    }

}

template <class TImage>
void Pyramid<TImage>::drawSourcesColumn(int x,int y,int rmax,int gmax) const
{
  assert(_myType == SOURCES_PYRAMID);
  int x1=x,y1=y;
  for(int i=_levels-1;i>=0;i--)
    {
      _imgs[i]->sourcesDraw(x1,y1,rmax>>i,gmax>>i);

      y1 += (_imgs[i]->height() + 5);
    }

}

template <class TImage>
bool Pyramid<TImage>::winxyToImagexy(int wx,int wy,int & ix, int &iy,
				     int & level)
{
  for(level=0;level<_levels;level++)
    if (_imgs[level]->winxyToImagexy(wx,wy,ix,iy))
      {
	return true;
      }
  return false;
}

template <class TImage>
int 
Pyramid<TImage>::displayHeight() const
{
  int height = 0;
  for(int level=0;level<_levels;level++)
    height += (_imgs[level]->height() + 5);
  return height;
}
template <class TImage>
COLORSPACE Pyramid<TImage>::Colorspace( void )
{
  if( NLevels() >= 1 )
    return (*this)[0].Colorspace();
  else
    return RGB_SPACE;
}

template <class TImage>
void Pyramid<TImage>::SetColorspace( COLORSPACE c )
{
  for( int index = 0; index < NLevels(); index++ )
    (*this)[index].SetColorspace( c );
}

template <class TImage>
void Pyramid<TImage>::ConvertToColorspace( COLORSPACE c )
{
  for( int index = 0; index < NLevels(); index++ )
    (*this)[index].ConvertToColorspace( c );
}

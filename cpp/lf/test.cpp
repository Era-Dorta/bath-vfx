#ifdef USE_LIBPNG
extern "C" {
#include <png.h>
}
#endif


  something has changed!!!

#include <string.h>
#include <GL/glut.h>
#include <gluigluiglui.h>

#include "compat.h"
#include "Image.h"
#include "FilterLearner.h"

#ifdef THREADS
#include <pthread.h>
#endif

//////////////////////// GLOBAL VARIABLES //////////////////////////////////

int mainWindow;            // GLUT ID of the main window

// current size of the main window
int windowWidth = 500;
int windowHeight = 500;

// User interface controls
GLUI * glui;

#if 0
char * inputSourceExampleName = "InputImages/stockholm-crop-half.png";
char * inputFilteredExampleName = "InputImages/stockholm-crop-half.png";
char * inputSourceName = "InputImages/stockholm-crop2-half.png";
#endif 

#if 0
char * inputSourceExampleName = "InputImages/stockholm-crop-half.png";
char * inputFilteredExampleName = "InputImages/stockholm-crop-paint-half.png";
char * inputSourceName = "InputImages/stockholm-crop2-half.png";
#endif

#if 0
char * inputSourceExampleName = "41.png";
char * inputFilteredExampleName = "41.png";
char * inputSourceName = "41.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/161.png";
char * inputFilteredExampleName = "InputImages/161.png";
char * inputSourceName = "InputImages/AspenTreesCrop1.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/white.png";
char * inputFilteredExampleName = "InputImages/white.png";
char * inputSourceName = "InputImages/white.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/graygrad3.png";
char * inputFilteredExampleName = "InputImages/graygrad3.png";
char * inputSourceName = "InputImages/stockholm-crop2-half.png";
#endif


#if 0
char * inputSourceExampleName = "InputImages/conte1-down.png";
char * inputFilteredExampleName = "InputImages/conte1-down.png";
char * inputSourceName = "InputImages/stockholm-crop.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/conte2-down-src.png";
char * inputFilteredExampleName = "InputImages/conte2-down.png";
char * inputSourceName = "InputImages/stockholm-crop.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/pencil7-src.png";
char * inputFilteredExampleName = "InputImages/pencil7-2.png";
//char * inputSourceName = "InputImages/pencil7-src-rotate.png";
//char * inputSourceName = "Results/summer/linesketch/mi1p1.png";
//char * inputSourceName = "InputImages/fabricTraining.png";
//char * inputSourceName = "InputImages/water1-down-src2.png";
//char * inputSourceName = "InputImages/stockholm-crop.png";
char * inputSourceName = "InputImages/photos/6.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/water1-down-src4.png";
char * inputFilteredExampleName = "InputImages/water1-down.png";
char * inputSourceName = "InputImages/stockholm-crop.png";
#endif

#if 1
char * inputSourceExampleName = "InputImages/lizard1.png";
char * inputFilteredExampleName = "InputImages/lizard1-watercolor.png";
char * inputSourceName = "InputImages/lizard1-rotate.png";
#endif

#if 0
char * inputSourceExampleName = "Results/yiqtest/Y.png";
char * inputFilteredExampleName = "Results/yiqtest/y-ptg.png";
char * inputSourceName = "InputImages/stockholm-crop.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/conte2-down-src.png";//"InputImages/pastel1-down-src.pgm";
char * inputFilteredExampleName = "InputImages/conte2-down-src.png";
char * inputSourceName = "InputImages/cap18.png";
#endif

#if 0
char * inputSourceExampleName = "InputImages/161Downsamp.png";
char * inputFilteredExampleName = "InputImages/161Downsamp.png";
char * inputSourceName = "InputImages/stockholm-crop.png";
#endif


char * outputFilteredName = "output.png";
char * filteredName = "filtered.png";


//  sourceExampleImage : filteredExampleImage  ::  sourceImage : filteredImage
//  noise : filteredExampleImage :: noise : filteredImage

ColorImage * inputSourceExampleImage = NULL;
ColorImage * inputFilteredExampleImage = NULL;
ColorImage * inputSourceImage = NULL;
ColorImage * outputFilteredImage = NULL;

Pyramid<ColorImage> * outputPyramid = NULL;

Image<GLfloat> * sourceExampleImage = NULL;
Image<GLfloat> * filteredExampleImage = NULL;
Image<GLfloat> * sourceImage = NULL;
Image<GLfloat> * filteredImage = NULL;

int sourcePyramidType = GAUSSIAN_PYRAMID;
int filteredPyramidType = GAUSSIAN_PYRAMID;
int filteredFeatureType = RAW_RGB_FEATURE;
int filteredBaseProcedure = FILTER_SYNTH;

int pyramidHeight = 3;
int neighborhoodWidth = 5;
int useSourceImages = false;
int useRandom = true;
int usePoints = false;
int numNeighLevels = 2;
float levelWeighting = 1.0; //9.0/12.0;
float sourceFac = 1;
int useSplineWeights = true;
int savePointsToFile = false;
int maxTSVQdepth = 20;
int heurMaxTSVQdepth = 7;
float maxTSVQerror = 0;
int TSVQbacktracks = 8;
int xstep = 1;
int usePCA = 0;
int convertToYIQ = false;
//int grayscaleMode = 0;
float samplerEpsilon = .1;

int MLPnumHidden = 20;
double MLPdecayWeight = 0;
int MLPsigmoidal = false;

int showSourceExample = true;
int showFilteredExample = true;
int showSource = true;
int showFiltered = true;
int showReconstruction = false;
int showSources = false;

typedef CFilterLearner<GLfloat,GLfloat> TLearner;
TLearner fl;
int searchType = TLearner::HEURISTIC_SEARCH;


GLenum checkForError(char *loc);
void reconstruct(int=0);


/////////////////////// FUNCTION DECLARATIONS ////////////////////////////

void computeOutput(int redisplay=0)
{
  fl.useRandom() = useRandom ? true : false;
  fl.searchType() = (TLearner::SearchType) searchType;
  fl.levelWeighting() = levelWeighting;
  fl.numLevels() = numNeighLevels;
  fl.sourceFac() = sourceFac;
  fl.filteredBaseProcedure() = 
    (filteredBaseProcedure == COPY_SYNTH && 
     filteredFeatureType == DIFFERENCE_FEATURE ? 
     ZERO_SYNTH : (SynthProcedureType)filteredBaseProcedure);

  fl.SetNeighborhood(neighborhoodWidth, (bool)useSplineWeights);
  fl.savePointsToFile() = (bool)savePointsToFile;
  fl.maxTSVQdepth() = (searchType == TLearner::HEURISTIC_SEARCH ? 
		       heurMaxTSVQdepth : maxTSVQdepth);

  fl.maxTSVQerror() = maxTSVQerror;
  fl.TSVQbacktracks() = TSVQbacktracks;  
  fl.xstep() = xstep;
  fl.redisplay() = redisplay;
  fl.usePCA() = (bool) usePCA;
  fl.MLPsigmoidal() = MLPsigmoidal;
  fl.MLPnumHidden() = MLPnumHidden;
  fl.MLPhiddenDecayWeight() = MLPdecayWeight;
  fl.MLPoutputDecayWeight() = MLPdecayWeight;
  fl.samplerEpsilon() = samplerEpsilon;

  //  fl.grayscaleMode() = (bool)grayscaleMode;

  fl.Synthesize();

  reconstruct();
  /*
  for(int x=0;x<filteredImage->width();x++)
    for(int y=0;y<filteredImage->height();y++)
      filteredImage->Pixel(x,y) = sourceImage->Pixel(x,y)/
	double(sourceImage->maxVal());
  //filteredImage->maxVal()*(sin(x/5.0)+1)/2;
  */
}

void idle()
{
  glutSetWindow(mainWindow);
  glutPostRedisplay();
}


int motionWX = -1, motionWY = -1;
int motionSourceWX = -1, motionSourceWY = -1;
Image<float> * motionImage = NULL;

void mousePassiveMotion(int wx,int wy)
{
  // find out if these coordinates correspond to any image
  int ix, iy, level;

  motionWX = wx;
  motionWY = windowHeight - wy;

  bool result = false;

  if (showFiltered)
    fl.filteredPyramid().winxyToImagexy(motionWX,motionWY,ix,iy,level);
  
  if (!result && showSource)
    result = fl.sourcePyramid().winxyToImagexy(motionWX,motionWY,ix,iy,level);

  if (!result && showSources)
    result = fl.sourcesPyramid().winxyToImagexy(motionWX,motionWY,ix,iy,level);

  if (!result)
    {
      motionWX = -1;
    }
  else
    {
      int sourceX = fl.sourcesPyramid()[level].Pixel(ix, iy,0);
      int sourceY = fl.sourcesPyramid()[level].Pixel(ix, iy,1);
      
      if (sourceX >= 0)
	{
	  fl.filteredExamplePyramid()[level].
	    imagexyToWinxy(sourceX,sourceY,motionSourceWX,motionSourceWY);
	}
      else
	{
	  motionSourceWX = -1;
	}
    }

  glutPostRedisplay();
}  

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);

  int x1=0;
  bool rescaleFeature = filteredFeatureType != RAW_RGB_FEATURE;
  if (showSourceExample)
    {
      fl.sourceExamplePyramid().drawColumn(x1,0);
      x1 += (inputSourceExampleImage->width() + 5);
    }

  if (showFilteredExample)
    {
      fl.filteredExamplePyramid().drawColumn(x1,0,rescaleFeature);
      x1 += (inputFilteredExampleImage->width() + 5);
    }

  if (showSource)
    {
      fl.sourcePyramid().drawColumn(x1,0);
      x1 += (inputSourceImage->width() + 5);
    }

  if (showFiltered)
    {
      fl.filteredPyramid().drawColumn(x1,0,rescaleFeature);
      x1 += (fl.filteredPyramid()[0].width() + 5);
    }

  if (showSources)
    {
      fl.sourcesPyramid().drawSourcesColumn(x1,0,
					    inputFilteredExampleImage->width(),
					    inputFilteredExampleImage->height());
      x1 += (fl.filteredPyramid()[0].width() + 5);
    }

  if (outputPyramid != NULL && showReconstruction)
    {
      outputPyramid->drawColumn(x1,0);
      x1 += ((*outputPyramid)[0].width()+5);
    }
  
  if (motionWX >= 0)
    {
      // draw a dot where the mouse is
      glColor3f(1,0,1);
      glPointSize(4);
      glBegin(GL_POINTS);
      glVertex2i(motionWX,motionWY);

      // draw a dot at the source for this pixel
      if (motionSourceWX >= 0)
	{
	  glVertex2i(motionSourceWX, motionSourceWY);
	}

      glEnd();
    }
   
  glutSwapBuffers();
}

void savePNGs(int)
{
  filteredImage->savePNG(filteredName);
  if (outputFilteredImage != NULL)
    outputFilteredImage->savePNG(outputFilteredName);
}

void saveScreenshot(int)
{
  // could make this a lot more efficient with a GLubyte image
  // with grayscale for grayscale images

  ColorImage screen(windowWidth,windowHeight,3);

  screen.read();
  screen.savePNG("screenshot.png");
}

#ifdef THREADS
pthread_t compute_thread;
bool computing = false;

void* compute(void*)
{
  computeOutput(0);
  return NULL;
}

void startComputation(int)
{
	assert(computing == false);

	pthread_attr_t attr;
	size_t size;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 0x2000);
	pthread_attr_getstacksize(&attr, &size);
	
	int result = pthread_create(&compute_thread,&attr, compute, NULL);

	pthread_detach(compute_thread);

	if (result == 0)
		computing = true;
	else
		printf("Unable to start new thread\n");
}
#endif

void reshape(int w,int h)
{
  windowWidth = w;
  windowHeight = h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, (GLfloat) windowWidth, 0.0f, 
          (GLfloat) windowHeight, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, windowWidth, windowHeight);
}

void SetupImages(int resizeWindow)
{
  // ------ create the feature images if necessary -----------

  showSource = useSourceImages;
  showSourceExample = useSourceImages;

  if (resizeWindow)
    glui->sync_live();
  
  // if an image was previously allocated, delete it
  if (filteredExampleImage != NULL &&
      filteredExampleImage != inputFilteredExampleImage)
    delete filteredExampleImage;

  if (convertToYIQ && (inputFilteredExampleImage->dim() != 3 ||
		       inputSourceExampleImage->dim() != 3))
    {
      printf("Can't use YIQ: dimensionality != 3\n");
      
      convertToYIQ = false;
    }

  if (convertToYIQ)
    {
      filteredExampleImage = inputFilteredExampleImage->luminance();
    }
  else
  if (filteredPyramidType != GAUSSIAN_PYRAMID ||
      filteredFeatureType != RAW_RGB_FEATURE)
    // create a copy of the existing image
    {
      filteredExampleImage = new ColorImage(*inputFilteredExampleImage);

      // subtract source image from filteredExample
      if (filteredFeatureType == DIFFERENCE_FEATURE)
	for(int x=0;x<filteredExampleImage->width();x++)
	  for(int y=0;y<filteredExampleImage->height();y++)
	    for(int d=0;d<filteredExampleImage->dim();d++)
	      filteredExampleImage->Pixel(x,y,d) -=
		sourceExampleImage->Pixel(x,y,d);
    }
  else
    // point to the existing image
    filteredExampleImage = inputFilteredExampleImage; 

  // setup source

  // if an image was previously allocated, delete it
  if (sourceExampleImage != NULL && 
      sourceExampleImage != inputSourceExampleImage)
    delete sourceExampleImage;
  
  if (sourceImage != NULL && sourceImage != inputSourceImage)
    delete sourceImage;

  if (convertToYIQ)
  {
    sourceExampleImage = inputSourceExampleImage->luminance();
    sourceImage = inputSourceImage->luminance();
  }
  else
  if (sourcePyramidType != GAUSSIAN_PYRAMID)
    {
      // create a copy of the existing image
      sourceExampleImage = new ColorImage(*inputSourceExampleImage);
      sourceImage = new ColorImage(*inputSourceImage);
    }
  else
    {
      // point to the existing image
      sourceExampleImage = inputSourceExampleImage;
      sourceImage = inputSourceImage;
    }

  // specify pyramid types
  fl.filteredPyramidType() = (PyramidType)filteredPyramidType;
  fl.sourcePyramidType() = (PyramidType)sourcePyramidType;

  fl.filteredBaseProcedure() = 
    (filteredBaseProcedure == COPY_SYNTH && 
     filteredFeatureType == DIFFERENCE_FEATURE ? 
     ZERO_SYNTH : 
     (SynthProcedureType)filteredBaseProcedure);

  // pass images to the filter learner (this also sets it's internal
  //   copy of useSourceImages)
  if (useSourceImages)
    fl.SetImages(sourceExampleImage, filteredExampleImage, sourceImage, 
		 pyramidHeight);
  else
    fl.SetImages(filteredExampleImage, pyramidHeight);

  // adjust the display window size
  int outputWidth;
  if (useSourceImages)
    outputWidth = inputSourceImage->width();
  else
    outputWidth = inputFilteredExampleImage->width();

  windowWidth = -5;

  if (showFilteredExample)
    windowWidth += inputFilteredExampleImage->width() + 5;

  if (showSourceExample)
    windowWidth += inputSourceExampleImage->width() + 5;

  if (showSource)
    windowWidth += inputSourceImage->width() + 5;

  if (showFiltered)
    windowWidth += outputWidth +5;

  if (showReconstruction)
    windowWidth += outputWidth +5;

  if (showSources)
    windowWidth += outputWidth + 5;

  if (windowWidth < 0)
    windowWidth = 5;

  windowHeight = 10;

  if (showFilteredExample || showSourceExample)
    windowHeight += inputSourceExampleImage->height();

  if (showSource || showFiltered || showReconstruction || showSources)
    windowHeight += inputSourceImage->height();
  
  filteredImage = &fl.filteredPyramid()[0];

  if (resizeWindow)
    {
      glutSetWindow(mainWindow);
      glutReshapeWindow(windowWidth, windowHeight);
      glutPostRedisplay();
    }
}

void checkGrayscale()
{
  if (!(inputFilteredExampleImage->dim() > 1 &&
	inputFilteredExampleImage->dim()==inputSourceExampleImage->dim()&&
	inputFilteredExampleImage->dim()==inputSourceImage->dim()))
    return;    

  printf("Checking if the input images are grayscale: ");

  bool grayscaleMode = true;

  int x,y,d;

  for(x=0;x<inputFilteredExampleImage->width();x++)
    for(y=0;y<inputFilteredExampleImage->height();y++)
      for(d=1;d<inputFilteredExampleImage->dim();d++)
	if (inputFilteredExampleImage->Pixel(x,y,0) != 
	    inputFilteredExampleImage->Pixel(x,y,d))
	  {
	    grayscaleMode = false;
	    printf("No.\n");
	    return;
	  }
    

  for(x=0;x<inputSourceExampleImage->width();x++)
    for(y=0;y<inputSourceExampleImage->height();y++)
      for(d=1;d<inputSourceExampleImage->dim();d++)
	if (inputSourceExampleImage->Pixel(x,y,0) != 
	    inputSourceExampleImage->Pixel(x,y,d))
	  {
	    grayscaleMode = false;
	    printf("No.\n");
	    return;
	  }
    

  for(x=0;x<inputSourceImage->width();x++)
    for(y=0;y<inputSourceImage->height();y++)
      for(d=0;d<inputSourceImage->dim();d++)
	if (inputSourceImage->Pixel(x,y,0) != 
	    inputSourceImage->Pixel(x,y,d)) 
	  {
	    grayscaleMode = false;
	    printf("No.\n");
	    return;
	  }

  printf("Yes. Converting.\n");

  ColorImage * filteredEx = new ColorImage(inputFilteredExampleImage->width(),
					   inputFilteredExampleImage->height(),
					   1);
  ColorImage * sourceEx = new ColorImage(inputSourceExampleImage->width(),
					 inputFilteredExampleImage->height(),
					 1);
  ColorImage * source = new ColorImage(inputSourceImage->width(),
				       inputSourceImage->height(),1);

  if (filteredEx == NULL || sourceEx == NULL || source == NULL)
    {
      printf("error: can't allocate grayscale images!!!!\n");
      return;
    }

  for(x=0;x<inputFilteredExampleImage->width();x++)
    for(y=0;y<inputFilteredExampleImage->height();y++)
      filteredEx->Pixel(x,y,0) = inputFilteredExampleImage->Pixel(x,y,0);

  for(x=0;x<inputSourceExampleImage->width();x++)
    for(y=0;y<inputSourceExampleImage->height();y++)
      sourceEx->Pixel(x,y,0) = inputSourceExampleImage->Pixel(x,y,0);

  for(x=0;x<inputSourceImage->width();x++)
    for(y=0;y<inputSourceImage->height();y++)
      source->Pixel(x,y,0) = inputSourceImage->Pixel(x,y,0);

  delete inputFilteredExampleImage, inputSourceExampleImage, inputSourceImage;

  inputFilteredExampleImage = filteredEx;
  inputSourceExampleImage = sourceEx;
  inputSourceImage = source;
}    


void
reconstruct(int)
{
  /*  if (filteredPyramidType != GAUSSIAN_PYRAMID)
    {
      printf("reconstructing source pyramid\n");
      fl.filteredExamplePyramid().reconstruct();
      fl.filteredPyramid().reconstruct();
    }

  if (sourcePyramidType != GAUSSIAN_PYRAMID)
    {
      printf("reconstructing source pyramid\n");
      fl.sourcePyramid().reconstruct();
      fl.sourceExamplePyramid().reconstruct();
    }

  if (filteredFeatureType == DIFFERENCE_FEATURE)
    {
      int x;

      printf("adding differences\n");
      for(x=0;x<filteredExampleImage->width();x++)
	for(int y=0;y<filteredExampleImage->height();y++)
	  for(int d=0;d<filteredExampleImage->dim();d++)
	    filteredExampleImage->Pixel(x,y,d) += 
	      sourceExampleImage->Pixel(x,y,d);
      
      for(x=0;x<filteredImage->width();x++)
	for(int y=0;y<filteredImage->height();y++)
	  for(int d=0;d<filteredImage->dim();d++)
	    filteredImage->Pixel(x,y,d) +=  sourceImage->Pixel(x,y,d);
    }
  */

  // extract color from source image
  if (outputPyramid != NULL)
    delete outputPyramid;
  
  outputPyramid = new Pyramid<ColorImage>(fl.filteredPyramid());
  outputFilteredImage = &outputPyramid->FineImage();

  if (filteredPyramidType != GAUSSIAN_PYRAMID)
    outputPyramid->reconstruct();

  if (filteredFeatureType == DIFFERENCE_FEATURE)
    {
      showReconstruction = true;

      Pyramid<ColorImage> * srcPyr;

      if (sourcePyramidType != GAUSSIAN_PYRAMID)
	{
	  srcPyr = new Pyramid<ColorImage>(fl.sourcePyramid());
	  srcPyr->reconstruct();
	}
      else
	srcPyr = &fl.sourcePyramid();

      for(int x=0;x<outputFilteredImage->width();x++)
	for(int y=0;y<outputFilteredImage->height();y++)
	  for(int d=0;d<outputFilteredImage->dim();d++)
	    outputFilteredImage->Pixel(x,y,d) +=  
	      srcPyr->FineImage().Pixel(x,y,d);
      
      if (sourcePyramidType != GAUSSIAN_PYRAMID)
	delete srcPyr;
    }

  if (convertToYIQ)
    {
      Pyramid<ColorImage> * lumPyr = outputPyramid;
      ColorImage * luminanceImage = outputFilteredImage;

      outputPyramid = 
	new Pyramid<ColorImage>(outputPyramid->NLevels(),
				outputFilteredImage->width(),
				outputFilteredImage->height(),3);
      outputFilteredImage = &outputPyramid->FineImage();
      outputPyramid->clear();

      showReconstruction = true;
      glui->sync_live();
      
      for(int x=0;x<outputFilteredImage->width();x++)
	for(int y=0;y<outputFilteredImage->height();y++)
	  {
	    double r1=inputSourceImage->Pixel(x,y,0)/
	      double(inputSourceImage->maxVal());
	    double g1=inputSourceImage->Pixel(x,y,1)/
	      double(inputSourceImage->maxVal());
	    double b1=inputSourceImage->Pixel(x,y,2)/
	      double(inputSourceImage->maxVal());
	    
	    double dummy,i,q;
	    double r2,g2,b2;
	    double lum = luminanceImage->Pixel(x,y);
	    
	    RGBtoYIQ(r1,g1,b1,dummy,i,q);
	    //	    RGBtoYIQ(r1,g1,b1,dummy,i,q);
	    YIQtoRGB(lum,i,q,r2,g2,b2);
	    
	    outputFilteredImage->Pixel(x,y,0) = 
	      r2*outputFilteredImage->maxVal();
	    outputFilteredImage->Pixel(x,y,1) = 
	      g2*outputFilteredImage->maxVal();
	    outputFilteredImage->Pixel(x,y,2) = 
	      b2*outputFilteredImage->maxVal();
	  }

      delete lumPyr;
      luminanceImage = NULL;
    }
  /*
  sourcePyramidType = GAUSSIAN_PYRAMID;
  filteredPyramidType = GAUSSIAN_PYRAMID;
  filteredFeatureType = RAW_RGB_FEATURE;

  glui->sync_live();  
  */
}

void createUI()
{
	
  // ------------------------ create the UI ----------------------------------
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize( windowWidth, windowHeight );
 
  mainWindow = glutCreateWindow( "Example-Based Image Filtering" );

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );  
  glutPassiveMotionFunc( mousePassiveMotion );

  // --- set up camera projection model and other rendering stuff ----
 
  reshape(windowWidth,windowHeight);

  glClearColor(1,1,1,0);
  //  glClearColor(1.0,.95,.85,0);
  glClearDepth(1.0);

  // --- set up controls ----
    
  glui = GLUI_Master.create_glui( "Controls" );

  reshape(windowWidth,windowHeight);
  
  glui->add_checkbox("Filter",&useSourceImages,1,SetupImages);
  glui->add_spinner("Pyramid height",GLUI_SPINNER_INT,&pyramidHeight,1,SetupImages)->set_int_limits(1,10);
  
  GLUI_Panel * span = glui->add_panel("Search Method");

  glui->add_spinner_to_panel(span,"X Step",GLUI_SPINNER_INT,&xstep)->set_int_limits(1,10);

  GLUI_RadioGroup * searchrg = glui->add_radiogroup_to_panel(span,&searchType);
  glui->add_radiobutton_to_group(searchrg, "Image");
  glui->add_radiobutton_to_group(searchrg, "Vector");
  glui->add_radiobutton_to_group(searchrg, "TSVQ");
  glui->add_radiobutton_to_group(searchrg, "Heuristic");
  glui->add_radiobutton_to_group(searchrg, "MLP");
  glui->add_radiobutton_to_group(searchrg, "TSVQR");

#ifdef LAPACK
  glui->add_checkbox_to_panel(span,"PCA",&usePCA);
#endif

  GLUI_Rollout * tsr = glui->add_rollout_to_panel(span,"TSVQ settings");
  tsr->close();

  glui->add_spinner_to_panel(tsr,"Max TSVQ depth",GLUI_SPINNER_INT,&maxTSVQdepth);
  glui->add_spinner_to_panel(tsr,"Heur Max TSVQ depth",GLUI_SPINNER_INT,&heurMaxTSVQdepth);
  glui->add_spinner_to_panel(tsr,"Max TSVQ error",GLUI_SPINNER_FLOAT,&maxTSVQerror);
  glui->add_spinner_to_panel(tsr,"TSVQ backtrack",GLUI_SPINNER_INT,&TSVQbacktracks);
  glui->add_checkbox_to_panel(span,"Save points files",&savePointsToFile);

  GLUI_Rollout * mr = glui->add_rollout_to_panel(span,"MLP settings");
  mr->close();

  glui->add_spinner_to_panel(mr,"Hidden Neurons",GLUI_SPINNER_INT,
			     &MLPnumHidden);
  glui->add_spinner_to_panel(mr,"Decay Weight",GLUI_SPINNER_FLOAT,
			     &MLPdecayWeight);
  glui->add_checkbox_to_panel(mr,"Sigmoidal",&MLPsigmoidal);
  

  GLUI_Rollout * sampst = glui->add_rollout("Sampling");
  
  glui->add_checkbox_to_panel(sampst,"Random Start",&useRandom);
  glui->add_spinner_to_panel(sampst,"Epsilon",GLUI_SPINNER_FLOAT,&samplerEpsilon)->set_float_limits(0,1e10);

  GLUI_Rollout * wts = glui->add_rollout("Neighborhoods");

  glui->add_spinner_to_panel(wts,"Width",GLUI_SPINNER_INT,
			     &neighborhoodWidth)->set_int_limits(3,15);
  glui->add_spinner_to_panel(wts,"Num levels",GLUI_SPINNER_INT,
			     &numNeighLevels)->set_int_limits(1,5);
  glui->add_checkbox_to_panel(wts,"Spline weights",&useSplineWeights);
  glui->add_spinner_to_panel(wts,"Level weight",GLUI_SPINNER_FLOAT,
			     &levelWeighting)->set_float_limits(0,5);
  glui->add_spinner_to_panel(wts,"Source weight",GLUI_SPINNER_FLOAT,
			     &sourceFac)->set_float_limits(0,1000);

  //  glui->add_checkbox_to_panel(wts,"Grayscale",&grayscaleMode);

  glui->add_column(true);

  GLUI_Rollout * fts = glui->add_rollout("Features");

  GLUI_Panel * spt = glui->add_panel_to_panel(fts,"Source Pyramid Type");
  GLUI_RadioGroup * sptrg = 
    glui->add_radiogroup_to_panel(spt,&sourcePyramidType,1,SetupImages);
  glui->add_radiobutton_to_group(sptrg, "Gaussian");
  glui->add_radiobutton_to_group(sptrg, "Laplacian");

  GLUI_Panel * fpt = glui->add_panel_to_panel(fts,"Filtered Pyramid Type");
  GLUI_RadioGroup * fptrg = 
    glui->add_radiogroup_to_panel(fpt,&filteredPyramidType,1,SetupImages);
  glui->add_radiobutton_to_group(fptrg, "Gaussian");
  glui->add_radiobutton_to_group(fptrg, "Laplacian");

  GLUI_Panel * ff = glui->add_panel_to_panel(fts,"Filtered Features");
  GLUI_Checkbox * yiqcb =
    glui->add_checkbox_to_panel(ff,"YIQ",&convertToYIQ,1,SetupImages); 
  GLUI_RadioGroup * ffrg = 
    glui->add_radiogroup_to_panel(ff,&filteredFeatureType,1,SetupImages);
  glui->add_radiobutton_to_group(ffrg, "Raw");
  glui->add_radiobutton_to_group(ffrg, "Difference Image");

  GLUI_Panel * bspt = glui->add_panel_to_panel(fts,"Filtered Base Procedure");
  GLUI_RadioGroup * bsptrg = 
    glui->add_radiogroup_to_panel(bspt,&filteredBaseProcedure,1,SetupImages);
  glui->add_radiobutton_to_group(bsptrg, "Synthesize");
  glui->add_radiobutton_to_group(bsptrg, "Copy source");
  glui->add_radiobutton_to_group(bsptrg, "Synth, no source")->disable();
  
  GLUI_Rollout * dis = glui->add_rollout("Display");

  glui->add_checkbox_to_panel(dis,"Source example",&showSourceExample);
  glui->add_checkbox_to_panel(dis,"Filtered example",&showFilteredExample);
  glui->add_checkbox_to_panel(dis,"Source",&showSource);
  glui->add_checkbox_to_panel(dis,"Filtered",&showFiltered);
  glui->add_checkbox_to_panel(dis,"Sources",&showSources);
  glui->add_checkbox_to_panel(dis,"Reconstruction\n",&showReconstruction);

  //  glui->add_button("Reconstruct",0,reconstruct);
  

  glui->add_button("Go",1,computeOutput);
#ifdef THREADS
  glui->add_button("Thread Go",0,startComputation);
#endif

  glui->add_separator();

  glui->add_button("Save PNGs",0,savePNGs);
  glui->add_button("Save screenshot",0,saveScreenshot);

  glui->add_separator();

  glui->add_button("Quit",0,exit);
  glui->set_main_gfx_window( mainWindow );

#ifdef THREADS
  GLUI_Master.set_glutIdleFunc( idle );
#else
  GLUI_Master.set_glutIdleFunc( NULL );
#endif

  checkForError("before entering main loop");

  // move the GLUI window out of the way

  glutSetWindow(glui->get_glut_window_id());

  glutPositionWindow(100,100);


  
}

void MLPtest()
{

  FILE *fpx = fopen("train.txt","wt");
  FILE *fpt = fopen("test.txt","wt");

  vector<Neigh*> x;
  vector<CVector<double>*> y;

  float t;
  /*  for(t=0;t<10;t+=.5)
    {
      Neigh * x1 = new Neigh(1,Point2(0,0));
      CVector<double> * y1 = new CVector<double>(1);
      (*x1)[0] = t;
      (*y1)[0] = sin(t);
      x.push_back(x1);
      y.push_back(y1);
      fprintf(fpx,"%f %f\n",(*x1)[0],(*y1)[0]);
    }
  */
  Neigh * x1 = new Neigh(1,Point2(0,0));
  CVector<double> * y1 = new CVector<double>(1);
  (*x1)[0] = 1;
  (*y1)[0] = 2;
  x.push_back(x1);
  y.push_back(y1);

  /*
  x1 = new Neigh(1,Point2(0,0));
  y1 = new CVector<double>(1);
  (*x1)[0] = 1;
  (*y1)[0] = 5;
  x.push_back(x1);
  y.push_back(y1);
  */

  MLP *mlp = new MLP(x,y,20,0,0,false);

  for(t=0;t<10;t+=.01)
    {
      CVector<double> v(1);
      v[0] = t;

      fprintf(fpt,"%f %f\n",t,mlp->apply(v)[0]);
    }
  
  fclose(fpt);
  fclose(fpx);

  exit(1);

}

/**************************************** main() ********************/

void main(int argc, char* argv[])
{
  //  MLPtest();



  //  ------------------  load all the images  -----------------------
  inputFilteredExampleImage = new ColorImage(inputFilteredExampleName);
  inputSourceExampleImage = new ColorImage(inputSourceExampleName);
  inputSourceImage = new ColorImage(inputSourceName);
	
  if (inputSourceExampleImage == NULL || inputFilteredExampleImage == NULL ||
      inputSourceImage == NULL)
    {
      printf("Error: can't allocate images\n");
      exit(1);
    }
	
  checkGrayscale();
  /*
  outputFilteredImage = new ColorImage(inputSourceImage->width(),
				       inputSourceImage->height(),
				       inputSourceImage->dim());

  if (outputFilteredImage == NULL)
    {
      printf("Error: can't allocate filtered image\n");
      exit(1);
    }
  */
  if (inputSourceExampleImage->width() != inputFilteredExampleImage->width() ||
      inputSourceExampleImage->height() != inputFilteredExampleImage->height())
    {
      printf("Example image dimensions don't match\n");
      exit(1);
    }

  fl.filteredPyramidType() = (PyramidType)filteredPyramidType;
  fl.sourcePyramidType() = (PyramidType)sourcePyramidType;
  /*
  windowWidth = inputSourceExampleImage->width() + inputSourceImage->width()+5;
  if (fl.useSourceImages())
    windowWidth += inputFilteredExampleImage->width() + outputFilteredImage->width()+10;
  windowHeight = 2*max(inputSourceExampleImage->height(), inputSourceImage->height())+20;
  */
  SetupImages(0);

  createUI();

	 // --- enter the main loop --- 
  glutMainLoop();
}



GLenum checkForError(char *loc)
{
  GLenum errCode;
  const GLubyte *errString;

  if ((errCode = glGetError()) != GL_NO_ERROR)
    {
      errString = gluErrorString(errCode);
      printf("OpenGL error: %s",errString);

      if (loc != NULL)
        printf("(%s)",loc);

      printf("\n");
    }

  return errCode;
}



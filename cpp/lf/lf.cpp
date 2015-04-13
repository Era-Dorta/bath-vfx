#ifdef USE_LIBPNG
extern "C" {
#include <png.h>
}
#endif

#ifdef _WIN32
#pragma warning( disable : 4786 ) // disable annoying 'identifier greater that 255 characters' warning
#pragma warning( disable : 4800 ) // disable annoying 'assigning int to bool eats cycles' warning
#endif

#include <string.h>
#include <GL/glut.h>
#include <glui.h>

#include "compat.h"
#include "Image.h"
#include "FilterLearner.h"
#include "Histogram.h"
extern "C" {
//#include "spyramid.h"
}
#include "color.h"

#ifdef THREADS
#include <pthread.h>
#endif

#include "ParseArgs.h"


#include <sys/stat.h>

#define MAX_FILENAME_SIZE 1024
//////////////////////// GLOBAL VARIABLES //////////////////////////////////

int mainWindow;            // GLUT ID of the main window

// current size of the main window
int windowWidth = 500;
int windowHeight = 500;

// User interface controls
GLUI * glui;

// command-line parser thing
ParseArgs argParser;

// list of example pair filenames:
vector< string > inputSourceExampleNames;
vector< string > inputFilteredExampleNames;

typedef vector<string> STRVECTOR;
/*
//steerable pyramid filter
char    steerFilterName[80];
PFILTER steerFilter;
int     steerFilterFwConvType=0;
*/
typedef char * MYSTR;
static MYSTR stfConvTypeArr[] ={
    "reflect1",   /* reflect about edge pixels - new name */
    "reflect2",    /* standard reflection */
    "repeat",      /* repeat edge pixel */
    "zero",       /* zero outside of image */
    "extend",        /* extend (reflect & invert) */
    "dont-compute", /* ignore edge (zero output for filter near edge) */
    "predict",      /* predict based on portion covered by filt */
    "ereflect",  	/* orthogonal QMF reflection */
    "treflect" 	/* reflect about edge pixels - old name */
  };

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-mask.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-src.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-testmask.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-mask.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-mask.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/oxbow-mask.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/fieldclouds-mask.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/fieldclouds-src.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/fieldclouds-testmask.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/path4.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/path4.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/path4.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/path2-src-down.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/path2-down.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/newpath2-src.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/path2-src-down.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/path2-down.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/cc-2.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "nudge/waves-src-down-left-nudged.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/waves-down-left.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/mtn-src-down-right.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "nudge/waves-src-down-left-nudged.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/waves-down-left.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/waves-src-down-right.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "nudge/mtn-src-down-left-nudged.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/bigbook/mtn-down-left.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/mtn-src-down-right.png";

#define USE_MODE_MASK    "InputImages/bigbook/mtn-down-left-modemask.png";
#define USE_TARGET_MODE_MASK  "InputImages/bigbook/mtn-down-right-modemask-target.png";
#endif

#if 1
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/ma/14.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/ma/14.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/ma/circle.tar.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/media/water2-down-src1.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/media/water2-down-src1.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/media/water2-down-src2.png";
#endif

#if 0
char inputSourceExampleName[MAX_FILENAME_SIZE] = "InputImages/freud/newreflect2-src-down.png";
char inputFilteredExampleName[MAX_FILENAME_SIZE] = "InputImages/freud/newreflect2-down.png";
char inputSourceName[MAX_FILENAME_SIZE] = "InputImages/bigbook/mtn-src-down-right.png";
#endif

#ifndef USE_MODE_MASK

char filterModeMaskName[MAX_FILENAME_SIZE] = "(none!)";
int useFilterModeMask = false;
#else
char filterModeMaskName[MAX_FILENAME_SIZE] = USE_MODE_MASK;
int useFilterModeMask = true;
#endif


#ifndef USE_TARGET_MODE_MASK

char targetModeMaskName[MAX_FILENAME_SIZE] = "(none!)";
int useTargetModeMask = false;
#else
char targetModeMaskName[MAX_FILENAME_SIZE] = USE_TARGET_MODE_MASK;
int useTargetModeMask = true;
#endif

char steerFilterFileName[MAX_FILENAME_SIZE] = "spfilter.3";

char outputFilteredName[MAX_FILENAME_SIZE] = "output.png";
char filteredName[MAX_FILENAME_SIZE] = "filtered.png";
char outputSummaryPattern[MAX_FILENAME_SIZE] = "summary%d.png";
char outputSummaryName[MAX_FILENAME_SIZE] = "summary.png";
char summaryHTMLPattern[MAX_FILENAME_SIZE] = "summary%d.html";
char summaryHTMLName[MAX_FILENAME_SIZE] = "summary.html";

int summaryIndex = -1;

char outputDirectoryName[MAX_FILENAME_SIZE] = "resultDir";
char outputFilterName[MAX_FILENAME_SIZE] = "filtered";

bool loadSourcesImage = false;
char sourcesImageName[MAX_FILENAME_SIZE] = "sources.data";
Image<GLshort> * oldSourcesImage = NULL;

enum { SUMMARY, TABLE };
int outputStyle = SUMMARY;

int filterColorspace = RGB_SPACE;
int sourceColorspace = RGB_SPACE;

//  sourceExampleImage : filteredExampleImage  ::  sourceImage : filteredImage
//  noise : filteredExampleImage :: noise : filteredImage

vector< ColorImage * > inputSourceExampleImages;
vector< ColorImage * > inputFilteredExampleImages;
//ColorImage * inputSourceExampleImage = NULL;
//ColorImage * inputFilteredExampleImage = NULL;

ColorImage * inputSourceImage = NULL;
ColorImage * outputFilteredImage = NULL;

ColorImage * filterModeMask = NULL;
ColorImage * targetModeMask = NULL;

Pyramid<ColorImage> * outputPyramid = NULL;

vector< Image<GLfloat> * > sourceExampleImages;
vector< Image<GLfloat> * > filteredExampleImages;
//Image<GLfloat> * sourceExampleImage = NULL;
//Image<GLfloat> * filteredExampleImage = NULL;
Image<GLfloat> * sourceImage = NULL;
Image<GLfloat> * filteredImage = NULL;

int sourcePyramidType = GAUSSIAN_PYRAMID;
int filteredPyramidType = GAUSSIAN_PYRAMID;
int filteredFeatureType = RAW_RGB_FEATURE;
int filteredBaseProcedure = FILTER_SYNTH;

int pyramidHeight = 4;
int neighborhoodWidth = 5;
int useSourceImages = true;
int useRandom = true;
int usePoints = false;
int numNeighLevels = 2;
float levelWeighting = 1.0; //9.0/12.0;
float finalSourceFac = -1;
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
bool sourceIsGrayscale = false;
float samplerEpsilon = 0.1;
int numPasses = 1;
int useSourceImagesAfterFirstPass = true;
float modeMaskWeight = 0.01;
int oneway = false;

float coherenceEps = 5;
float coherencePow = 2;

int MLPnumHidden = 20;
double MLPdecayWeight = 0;
int MLPsigmoidal = false;

float annEpsilon = 1;
float heurAnnEpsilon = 1;

int showSourceExample = true;
int showFilteredExample = true;
int showSource = true;
int showFiltered = true;
int showReconstruction = true;
int showSources = false;
int showModeMask = useFilterModeMask || useTargetModeMask;
int showExampleModeMask = useFilterModeMask;

int useBias = false;
int useGain = false;
float biasPenalty = 0;
float gainPenalty = 0;

int matchGrayHistogram = false;
int histogramEq = false;
int matchMeanVariance = false;
int matchBtoA = false;

typedef CFilterLearner<GLfloat,GLfloat> TLearner;
TLearner fl;
int searchType = TLearner::ANN_SEARCH;

bool createSrcLocHisto = false;
bool useInterface = true;
int multithreadedMode = true;

int cheesyBoundaries = true;

int ashikhminLastLevel = false;

int onePixelSource = false;

float timeToProcess = 0;

// function prototypes:
GLenum checkForError(char *loc);
void reconstruct(int=0);
void saveOutput( int=0 );
void saveTable( void );
void printArgs(int); // callback for "Print Args" button
void adjustWindowSize(int=0);
void writeHTMLFile( void );

void SetupArgParser( ParseArgs &parser );
//void ParseArgsSteerFilterCallback(const char *name, char *var,
//				  int maxLen, ParseArgs *parser);


// p for L_p norm; also need to change include/ANN/ANN.h if you change this 
const float ANN_NORM_p = 2;


/////////////////////// FUNCTION DECLARATIONS ////////////////////////////

void computeOutput(int redisplay=0)
{
  fl.useRandom() = useRandom ? true : false;
  fl.searchType() = (TLearner::SearchType) searchType;
  fl.levelWeighting() = levelWeighting;
  fl.finalSourceFac() = finalSourceFac;
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
  fl.numPasses() = numPasses; 
  fl.annEpsilon() = (searchType == TLearner::HEURISTIC_ANN_SEARCH ?
		     heurAnnEpsilon : annEpsilon);
  fl.useSourceImagesAfterFirstPass() = useSourceImagesAfterFirstPass;
  fl.cheesyBoundaries() = cheesyBoundaries;

  fl.useBias() = useBias;
  fl.useGain() = useGain;
  fl.modeMaskWeight() = modeMaskWeight;

  fl.biasPenalty() = biasPenalty;
  fl.gainPenalty() = gainPenalty;

  fl.createSrcLocHisto() = createSrcLocHisto;
  //  fl.grayscaleMode() = (bool)grayscaleMode;

  fl.coherenceEps() = coherenceEps;
  fl.coherencePow() = coherencePow;

  fl.oneway() = oneway;

  fl.ashikhminLastLevel() = ashikhminLastLevel;

  fl.onePixelSource() = onePixelSource;

  //clock_t startTime = clock();
  time_t startTime = time(0);

  fl.Synthesize();

  reconstruct();

  //  clock_t endTime = clock();
  //  timeToProcess = (float)(endTime-startTime) / (float)CLOCKS_PER_SEC;

  time_t endTime = time(0);
  timeToProcess = endTime - startTime;

  printf("Time elapsed: %3.2f s\n", timeToProcess);
  
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

bool winxyToImagexy(int wx,int wy,int &imx,int &imy)
{
  int ix, iy, level;

  bool result = false;

  if (showFiltered)
    result = fl.filteredPyramid().winxyToImagexy(wx,wy,ix,iy,level);
  
  if (!result && showSource)
    result = fl.sourcePyramid().winxyToImagexy(wx,wy,ix,iy,level);

  if (!result && showSources)
    result = fl.sourcesPyramid().winxyToImagexy(wx,wy,ix,iy,level);

  if (result)
    {
      int sourceX = fl.sourcesPyramid()[level].Pixel(ix, iy,0);
      int sourceY = fl.sourcesPyramid()[level].Pixel(ix, iy,1);
      
      if (sourceX >= 0)
      {
	fl.filteredExamplePyramid()[level].
	  imagexyToWinxy(sourceX,sourceY,imx,imy);
      }
      else
      {
	imx = -1;
	imy = -1;
      }
    }
  
  return result;
}


int motionWX = -1, motionWY = -1;
int motionSourceWX = -1, motionSourceWY = -1;
Image<float> * motionImage = NULL;

void mousePassiveMotion(int wx,int wy)
{
  // find out if these coordinates correspond to any image
  motionWX = wx;
  motionWY = windowHeight - wy;

  bool result = winxyToImagexy(motionWX,motionWY,
			       motionSourceWX,motionSourceWY);

  if (!result)
    motionWX = -1;

  glutPostRedisplay();
}  

void mouseClick(int button, int state, int wx,int wy)
{
  if (state == GLUT_UP)
    return;

  motionWX = wx;
  motionWY = windowHeight - wy;

  bool result = winxyToImagexy(motionWX,motionWY,
			       motionSourceWX,motionSourceWY);

  if (!result)
    motionWX = -1;
  else
    printf("example loc = (%d %d)\n",motionSourceWX,motionSourceWY);
  glutPostRedisplay();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);

  int x1=0;
  bool rescaleFeature = filteredFeatureType != RAW_RGB_FEATURE;

  int numExamples = fl.NumExamplePairs();
  for( int index = 0; index < numExamples; index++ )
  {
    if (showSourceExample)
    {
      fl.sourceExamplePyramid(index).drawColumn(x1,0);
      x1 += fl.sourceExamplePyramid(index).displayWidth();
    }
    
    if (showFilteredExample)
    {
      fl.filteredExamplePyramid(index).drawColumn(x1,0,rescaleFeature);
      x1 += fl.filteredExamplePyramid(index).displayWidth();
    }
    
    //    if( showSourceExample && showFilteredExample )
    //      x1 += 5;
  }

  if (showExampleModeMask && fl.exampleModeMaskPyramid().NLevels() > 0)
    {
      fl.exampleModeMaskPyramid().drawColumn(x1,0);
      x1 += fl.exampleModeMaskPyramid().displayWidth();
    }

  if (showSource)
    {
      fl.sourcePyramid().drawColumn(x1,0);
      x1 += fl.sourcePyramid().displayWidth();
    }

  if (showFiltered)
    {
      fl.filteredPyramid().drawColumn(x1,0,rescaleFeature);
      x1 += fl.filteredPyramid().displayWidth();
    }

  if (showModeMask && fl.modeMaskPyramid().NLevels() > 0 )
    {
      fl.modeMaskPyramid().drawColumn(x1,0);
      x1 += fl.modeMaskPyramid().displayWidth();
    }

  if (showSources)
    {
      fl.sourcesPyramid().
	drawSourcesColumn(x1,0,fl.filteredExamplePyramid()[0].width(),
			  fl.filteredExamplePyramid()[0].height());
      x1 += fl.sourcesPyramid().displayWidth();
      //      fl.sourcesPyramid().drawSourcesColumn(x1,0,
      //					    inputFilteredExampleImage->width(),
      //					    inputFilteredExampleImage->height());
      //      x1 += (fl.filteredPyramid()[0].width() + 5);
    }

  if (outputPyramid != NULL && showReconstruction)
    {
      outputPyramid->drawColumn(x1,0);
      x1 += outputPyramid->displayWidth();
    }
  
  if (motionWX >= 0)
    {
      // draw a dot where the mouse is
      glColor3f(1,0,1);
      glPointSize(4);
      glBegin(GL_POINTS);
      glVertex2i(motionWX,motionWY);
      glEnd();

      // draw a dot at the source for this pixel
      if (motionSourceWX >= 0)
	{
	  glColor3f(1,0,1);
	  glPointSize(4);
	  glBegin(GL_POINTS);
	  glVertex2i(motionSourceWX, motionSourceWY);
	  glEnd();
	}

      glEnd();
    }
   
  glutSwapBuffers();
}

void saveSummary(int=0)
{
  writeHTMLFile();

  // ####
  // TODO: fix this to save all of the example pairs, not just the first

  if (useSourceImages)
    {
      // --------- determine the size of the image ------------------
      
      int pairIndex = 0;

      int dim = max(sourceImage->dim(),filteredImage->dim());
      dim = max(dim,inputSourceImage->dim());
      if (outputFilteredImage != NULL)
	dim = max(dim,outputFilteredImage->dim());

      int width;

      int filteredRowWidth = 0;
      /*
      filteredRowWidth = filteredImage->width();
      if (sourceImage != NULL)
	filteredRowWidth += sourceImage->width() + 5;
      if (inputSourceImage != sourceImage && 
	  (!convertToYIQ || !sourceIsGrayscale))
	filteredRowWidth += (inputSourceImage->width() + 5);
      if (filteredImage != outputFilteredImage && 
	  outputFilteredImage != NULL
	  && !(convertToYIQ && sourceIsGrayscale))
	filteredRowWidth += (outputFilteredImage->width() + 5);
      if (showModeMask && fl.modeMaskPyramid().NLevels() > 0)
	filteredRowWidth += (fl.modeMaskPyramid().FineImage().width() + 5);
      */

      if (inputSourceImage != sourceImage && 
	  (!convertToYIQ || !sourceIsGrayscale))
	filteredRowWidth += (inputSourceImage->width() + 5);

      if (sourceImage != NULL)
	filteredRowWidth += (sourceImage->width() + 5);

      filteredRowWidth += (filteredImage->width()+5);

      if (filteredImage != outputFilteredImage && outputFilteredImage != NULL
	  && !(convertToYIQ && sourceIsGrayscale))
	filteredRowWidth += outputFilteredImage->width()+5;

      if (useFilterModeMask || useTargetModeMask)
	filteredRowWidth += fl.modeMaskPyramid().FineImage().width() + 5;


      int exampleRowWidth = filteredExampleImages[pairIndex]->width();
      exampleRowWidth += sourceExampleImages[pairIndex]->width()+5;
      if (sourceExampleImages[pairIndex] != inputSourceExampleImages[pairIndex] && 
	  !convertToYIQ)
	exampleRowWidth += (inputSourceExampleImages[pairIndex]->width() + 5);
      if (filteredExampleImages[pairIndex] != inputFilteredExampleImages[pairIndex] &&
	  !convertToYIQ)
	exampleRowWidth += (inputFilteredExampleImages[pairIndex]->width() + 5);

      if (useFilterModeMask)
	exampleRowWidth += (filterModeMask->width() + 5);

      width = max(filteredRowWidth,exampleRowWidth);

      int height = filteredExampleImages[pairIndex]->height() + filteredImage->height()+5;

      // --------  create the image ----------------------------------
      
      ColorImage summary(width,height,dim);
      summary.set();  // clear it

      // -------- copy the images -----------------------------------


      int x0=0,y0=filteredImage->height()+5;

      // top row

      if (inputSourceExampleImages[pairIndex] != sourceExampleImages[pairIndex] && !convertToYIQ)
	{
	  summary.insert(*inputSourceExampleImages[pairIndex],x0,y0);
	  x0 += (inputSourceExampleImages[pairIndex]->width() + 5);
	}

      summary.insert(*sourceExampleImages[pairIndex],x0,y0);
      x0 += (sourceExampleImages[pairIndex]->width() + 5);

      summary.insert(*filteredExampleImages[pairIndex],x0,y0);
      x0 += (filteredExampleImages[pairIndex]->width() + 5);

      if (inputFilteredExampleImages[pairIndex] != filteredExampleImages[pairIndex] &&
	  !convertToYIQ)
	{
	  summary.insert(*inputFilteredExampleImages[pairIndex],x0,y0);
	  x0 += (inputFilteredExampleImages[pairIndex]->width() + 5);
	}

      if (useFilterModeMask)
	{
	  summary.insert(*filterModeMask,x0,y0);
	  x0 += (filterModeMask->width() + 5);
	}
	  
      // bottom row

      y0 = 0;
      x0 = 0;

      if (inputSourceImage != sourceImage && 
	  (!convertToYIQ || !sourceIsGrayscale))
	{
	  summary.insert(*inputSourceImage,x0,y0);
	  x0 += inputSourceImage->width() + 5;
	}

      if (sourceImage != NULL)
	{
	  summary.insert(*sourceImage,x0,y0,
			 sourcePyramidType != GAUSSIAN_PYRAMID);
	  x0 += sourceImage->width() + 5;
	}

      summary.insert(*filteredImage,x0,y0,
		     filteredFeatureType != RAW_RGB_FEATURE ||
		     filteredPyramidType != GAUSSIAN_PYRAMID);
      x0 += filteredImage->width() + 5;

      if (filteredImage != outputFilteredImage && outputFilteredImage != NULL
	  && !(convertToYIQ && sourceIsGrayscale))
	{
	  summary.insert(*outputFilteredImage,x0,y0);
	  x0 += outputFilteredImage->width()+5;
	}

      if (useFilterModeMask || useTargetModeMask)
	{
	  summary.insert(fl.modeMaskPyramid().FineImage(),x0,y0);
	  x0 += fl.modeMaskPyramid().FineImage().width() + 5;
	}

      summary.savePNG(outputSummaryName);
    }
  else
    {
      int pairIndex = 0;

      int dim = filteredImage->dim();
      int width = filteredImage->width()+filteredExampleImages[pairIndex]->width()+5;
      int height = max(filteredImage->height(),filteredExampleImages[pairIndex]->height());
      
      if (inputFilteredExampleImages[pairIndex] != filteredExampleImages[pairIndex])
	width += (filteredExampleImages[pairIndex]->width()+5);
      
      if (outputFilteredImage != filteredImage && outputFilteredImage !=NULL)
	width += (filteredImage->width()+5);

      dim = max(dim, inputFilteredExampleImages[pairIndex]->dim());
      dim = max(dim, filteredExampleImages[pairIndex]->dim());
		
      ColorImage summary(width,height,dim);

      summary.set();

      int x0=0;

      summary.insert(*inputFilteredExampleImages[pairIndex],x0,0);
      x0 += (inputFilteredExampleImages[pairIndex]->width() + 5);

      if (inputFilteredExampleImages[pairIndex] != filteredExampleImages[pairIndex] &&
	  filteredExampleImages[pairIndex] != NULL)
	{
	  summary.insert(*filteredExampleImages[pairIndex],x0,0,
			 filteredFeatureType != RAW_RGB_FEATURE ||
			 filteredPyramidType != GAUSSIAN_PYRAMID);
	  x0 += (filteredExampleImages[pairIndex]->width() + 5);
	}
	  
      summary.insert(*filteredImage,x0,0,
			 filteredFeatureType != RAW_RGB_FEATURE ||
			 filteredPyramidType != GAUSSIAN_PYRAMID);
      x0 += filteredImage->width()+5;

      if (outputFilteredImage != filteredImage && outputFilteredImage != NULL)
	{
	  summary.insert(*outputFilteredImage,x0,0);
	  x0 += outputFilteredImage->width()+5;
	}

      summary.savePNG(outputSummaryName);
    }
}

void writeHTMLFile( void )
{
  // save param output
  ofstream summaryHTMLFile( summaryHTMLName );
  summaryHTMLFile << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">" << endl;
  summaryHTMLFile << "<HTML>" << endl << "<HEAD>" << endl;
  summaryHTMLFile << "  <TITLE>" << outputSummaryName << "</TITLE>" << endl;
  summaryHTMLFile << "</HEAD>" << endl;
  
  summaryHTMLFile << "<BODY" << endl;
  summaryHTMLFile << "bgcolor=\"#ffffe0\" text=\"#100000\" vlink=\"#101030\" link=\"#002040\" alink=\"#008000\">" << endl;
  
#if 1
  summaryHTMLFile << "<img src=\"" << outputSummaryName << "\">" << endl;
#else
  summaryHTMLFile << "<table border=\"0\" cellpadding=\"10\">" << endl;

  summaryHTMLFile << "<!-- A, f(A), gray(f(A))-->" << endl;
  summaryHTMLFile << "<tr>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << inputSourceExampleName << "\"> </td>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << inputFilteredExampleName << "\"> </td>" << endl;
  summaryHTMLFile << "<!-- TODO: fill this in... -->" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << inputFilteredExampleName << ".gray.png" << "\"> </td>" << endl;
  summaryHTMLFile << "</tr>" << endl;


  summaryHTMLFile << "<!-- B, f(B), gray(f(B))-->" << endl;
  summaryHTMLFile << "<tr>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << inputSourceName << "\"> </td>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << outputFilteredName << "\"> </td>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << filteredName << "\"> </td>" << endl;
  summaryHTMLFile << "</tr>" << endl;

  /*
  summaryHTMLFile << "<!-- f_c(B), f_yiq(B), gray(f_yiq(B))-->" << endl;
  summaryHTMLFile << "<tr>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << ... << "\"> </td>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << ... << "\"> </td>" << endl;
  summaryHTMLFile << "  <td> <img src=\"" << ... << "\"> </td>" << endl;
  summaryHTMLFile << "</tr>" << endl;
  */
  summaryHTMLFile << "</table>" << endl;
#endif

  summaryHTMLFile << "<br>" << endl;
  argParser.DumpValuesHTML( summaryHTMLFile );
  
  summaryHTMLFile << "<br>" << endl;
  summaryHTMLFile << "Elapsed time: " << timeToProcess << "s" << endl;
  
  summaryHTMLFile << "</BODY>" << endl;
  summaryHTMLFile << "</HTML>" << endl;

  printf("Saved %s\n",summaryHTMLName);
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

void startComputation(int=0)
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
#else
void startComputation(int=0)
{
  printf("Multithreaded support not compiled in\n");
}
#endif

void go(int=0)
{
  if (multithreadedMode && useInterface)
    startComputation();
  else
    computeOutput(1);
}      

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
/*
//callback for setting the steering filter
void steerFilterUpdate(int)
{
  //read the steer filter name
  if (steerFilterName==NULL) return;
  steerFilter = LoadPFilter(steerFilterName);
}

void stfChangeParams(int)
{
  if (steerFilter) {
    strcpy(steerFilter->fwd_conv_type, stfConvTypeArr[steerFilterFwConvType]);
  } else 
    printf("Error!: Steering Filter not loaded\n");
}
*/

void matchMV(ColorImage * mvsrc, ColorImage * target, 
	     ColorImage * target2=NULL)
  // Compute a colorspace transform that maps target's colors to mvsrc's 
  // colors, and apply the transform to target.  Also, apply the same 
  // transform to target2 if target2 is non-NULL.
{
  printf("Matching mean and covariance\n");

  int dim = mvsrc->dim();
  assert(dim == target->dim() && (target2 == NULL || dim == target2->dim()));

  if (dim == 1)
    {
      float sourceMean, sourceVariance;
      mvsrc->meanVariance(sourceMean, sourceVariance);
      float targetMean, targetVariance;
      target->meanVariance(targetMean, targetVariance);
      float gain = sqrt(sourceVariance / targetVariance);
      float bias = sourceMean - gain * targetMean;
      
      target->applyGainBias(gain,bias);
      if (target2 != NULL)
	target2->applyGainBias(gain,bias);
    }
  else
    {
#ifndef LAPACK
      printf("can't match colorspace; LAPACK not compiled in\n");
#else
      // warning: misleading variable names ahead -- need to fix this
      CVector<double> sourceExampleMean(dim), sourceMean(dim);
      CMatrix<double> sourceExampleCovar(dim,dim), sourceCovar(dim,dim);
      
      target->meanCovariance(sourceExampleMean, sourceExampleCovar);
      mvsrc->meanCovariance(sourceMean, sourceCovar);
      
      CMatrix<double> eigenvectorsSE(dim,dim);
      CVector<double> eigenvalsSE = 
	sourceExampleCovar.symmetricEigenvectors(eigenvectorsSE,dim,1);
      CMatrix<double> eigenvectorsS(dim,dim);
      CVector<double> eigenvalsS = 
	sourceCovar.symmetricEigenvectors(eigenvectorsS,dim,1);
      
      CMatrix<double> sigmaSE(dim,dim), sigmaS(dim,dim);
      sigmaSE.clear();
      sigmaS.clear();

      for(int d=0;d<dim;d++)
      {
	if (eigenvalsSE[d] > 1e-8)
	  sigmaSE.get(d,d) = 1/sqrt(eigenvalsSE[d]);
	else
	{
	  printf("Note: source covariance is singular; grayscale? (%d)\n",d);
	  sigmaSE.get(d,d) = 1e-8;
	}

	if (eigenvalsS[d] > 0)
	  sigmaS.get(d,d) = sqrt(eigenvalsS[d]);
	else
	  {
	    printf("Note: target covariance is singular; grayscale? (%d)\n",d);
	    sigmaS.get(d,d) = 0;
	  }
      }
      
      CMatrix<double> gain = 
	eigenvectorsS * sigmaS * eigenvectorsS.transpose() *
	eigenvectorsSE * sigmaSE * eigenvectorsSE.transpose();
      CVector<double> bias = sourceMean - gain * sourceExampleMean;

      target->applyGainBias(gain,bias);
      if (target2 != NULL)
	target2->applyGainBias(gain,bias);
            
      /*
      CVector<double> newMean(dim);
      CMatrix<double> newCovar(dim,dim);
      sourceExampleImages[0]->meanCovariance(newMean,newCovar);
      printf("\noldmean = \n");
      sourceExampleMean.printFloat();
      printf("\ntargetmean = \n");
      sourceMean.printFloat();
      printf("\nnewmean = \n");
      newMean.printFloat();
      printf("\noldcovar = \n");
      sourceExampleCovar.printFloat();
      printf("\ntargetcovar = \n");
      sourceCovar.printFloat();
      printf("\nnewcovar = \n");
      newCovar.printFloat();
      printf("\neigenvectorsS = \n");
      eigenvectorsS.printFloat();
      printf("\neigenvalsS = \n");
      eigenvalsS.printFloat();
      printf("\neigenvectorsSE = \n");
      eigenvectorsSE.printFloat();
      printf("\neigenvalsSE = \n");
      eigenvalsSE.printFloat();
      printf("\nsigmaS = \n");
      sigmaS.printFloat();
      printf("\nsigmaSE = \n");
      sigmaSE.printFloat();
      printf("\nevS * sigmaS =\n");
      (eigenvectorsS*sigmaS).printFloat();
      printf("\ngain = \n");
      gain.printFloat();
      printf("\n");
      */
#endif


    }
}

void SetupImages(int)
{
  int index;
  int x, y;

  // convert input images to correct colorspace
  for( index = 0; index < inputSourceExampleImages.size(); index++ )
    inputSourceExampleImages[index]->ConvertToColorspace( (COLORSPACE)sourceColorspace );
  inputSourceImage->ConvertToColorspace( (COLORSPACE)sourceColorspace );

  for( index = 0; index < inputFilteredExampleImages.size(); index++ )
    inputFilteredExampleImages[index]->ConvertToColorspace( (COLORSPACE)filterColorspace );
/*
 // check for steerable filter
  if (sourcePyramidType==STEERABLE_PYRAMID) {
    if (steerFilter==NULL) {
      if (steerFilterName!=NULL) {
	printf("Reading filter from file %s\n",steerFilterName);
	//read the filter from file
	steerFilter = LoadPFilter(steerFilterName);
	//print the filter
	PrintPFilter(steerFilter);
      } else {
	fprintf(stderr, "Error: Steerable pyramid with no filter!!\n");
	exit(1);
      }
    }
  } 
*/
  // ------ create the feature images if necessary -----------

  showSource = useSourceImages;
  showSourceExample = useSourceImages;

  if (glui != NULL)
    glui->sync_live();
  
  // if an image was previously allocated, delete it
  for( index = 0; index < filteredExampleImages.size(); index++ )
  {
    if( filteredExampleImages[index] != NULL )
    {
      if( index >= inputFilteredExampleImages.size() || 
	  filteredExampleImages[index] != inputFilteredExampleImages[index] )
	  delete filteredExampleImages[index];
    }
  }
  
  filteredExampleImages.erase( filteredExampleImages.begin(), 
			       filteredExampleImages.end() );
  
  // if we're supposed to be in YIQ mode, make sure example images have 3 channels
  if( convertToYIQ )
  {
    for( index = 0; index < inputFilteredExampleImages.size(); index++ )
    {
      if ((inputFilteredExampleImages[index]->dim() != 3 ||
	   inputSourceExampleImages[index]->dim() != 3))
      {
	printf("Can't use YIQ: dimensionality != 3\n");
	convertToYIQ = false;
	break;
      }
    }
  }
  
  for( index = 0; index < inputFilteredExampleImages.size(); index++ )
  {
    if (convertToYIQ)
    {
      filteredExampleImages.push_back( inputFilteredExampleImages[index]->luminance() );
    }
    else if (filteredPyramidType != GAUSSIAN_PYRAMID ||
	     filteredFeatureType != RAW_RGB_FEATURE ||
	     matchGrayHistogram || histogramEq || matchMeanVariance)
    {
      // create a copy of the existing image
      filteredExampleImages.push_back( new ColorImage(*inputFilteredExampleImages[index]) );
    }
    else
      // point to the existing image
      filteredExampleImages.push_back( inputFilteredExampleImages[index] ); 
  }
  
  
  // setup source
  
  // if an image was previously allocated, delete it
  for( index = 0; index < sourceExampleImages.size(); index++ )
  {
    if( index >= inputSourceExampleImages.size() ||
	sourceExampleImages[index] != inputSourceExampleImages[index] )
      delete sourceExampleImages[index];
  }
  
  sourceExampleImages.erase( sourceExampleImages.begin(), 
			     sourceExampleImages.end() );
  
  if (sourceImage != NULL && sourceImage != inputSourceImage)
  {
    delete sourceImage;
    sourceImage = NULL;
  }
  
  if (convertToYIQ)
  {
    for( index = 0; index < inputSourceExampleImages.size(); index++ )
      sourceExampleImages.push_back( inputSourceExampleImages[index]->luminance() );
    
    sourceImage = inputSourceImage->luminance();
  }
  else if (sourcePyramidType != GAUSSIAN_PYRAMID  || matchGrayHistogram ||
	   histogramEq || matchMeanVariance || matchBtoA)
  {
    // create a copy of the existing image
    for( index = 0; index < inputSourceExampleImages.size(); index++ )
      sourceExampleImages.push_back( new ColorImage(*inputSourceExampleImages[index]) );
    
    sourceImage = new ColorImage(*inputSourceImage);
  }
  else
  {
    // point to the existing image
    for( index = 0; index < inputSourceExampleImages.size(); index++ )
      sourceExampleImages.push_back( inputSourceExampleImages[index] );
    
    sourceImage = inputSourceImage;
  }
  
  if ((matchGrayHistogram || histogramEq || matchMeanVariance))
  {
    if( sourceExampleImages.size() != 1 )
    {
      printf( "Warning: can't do histogram matching on more than one example pair\n" );
      matchGrayHistogram = false;
      histogramEq = false;
      matchMeanVariance = false;
    }
    
    if ((matchGrayHistogram || histogramEq) && 
	(sourceExampleImages[index]->dim() != 1))
    {
      printf("Can't histogram match for non-gray images\n");
      matchGrayHistogram = false;
      histogramEq = false;
      matchMeanVariance = false;
    }
  }
  
  if (matchGrayHistogram)
  {
    Histogram sHist(*sourceImage,1.0/256);
    Histogram seHist(*sourceExampleImages[0],1.0/256);
    
    for(x=0;x<sourceExampleImages[0]->width();x++)
      for(y=0;y<sourceExampleImages[0]->height();y++)
      {
	sourceExampleImages[0]->Pixel(x,y,0) = 
	  seHist.applyHistMatchFunc(sourceExampleImages[0]->Pixel(x,y,0),
				    sHist);
      }
  }
  
  // this only really works for a single input pair...
  if (histogramEq)
  { 
   if( sourceExampleImages.size() != 1 )
      printf( "Warning: histogram eq only works for a single example pair!\n" );
    
    Histogram seHist(*sourceExampleImages[0],1.0/256);
    
    for(x=0;x<sourceExampleImages[0]->width();x++)
      for(y=0;y<sourceExampleImages[0]->height();y++)
	sourceExampleImages[0]->Pixel(x,y,0) = 
	  seHist.applyHistEqFunc(sourceExampleImages[0]->Pixel(x,y,0));
    
    Histogram sHist(*sourceImage,1.0/256);
    
    for(x=0;x<sourceImage->width();x++)
      for(y=0;y<sourceImage->height();y++)
	sourceImage->Pixel(x,y,0) = 
	  sHist.applyHistEqFunc(sourceImage->Pixel(x,y,0));

    sourceImage->savePNG( "SourceEQ.png");
  }
  
 if (matchBtoA)
    {
      matchMV(sourceExampleImages[0],sourceImage);
    }

  if (matchMeanVariance)
    {
      for(int i=0;i<sourceExampleImages.size();i++)
	matchMV(sourceImage,sourceExampleImages[i],filteredExampleImages[i]);
    }
 
  // subtract source image from filteredExample
  assert( filteredExampleImages.size() == sourceExampleImages.size() );
  for( index = 0; index < filteredExampleImages.size(); index++ )
  {
      if (filteredFeatureType == DIFFERENCE_FEATURE)
	for(x=0;x<filteredExampleImages[index]->width();x++)
	  for(y=0;y<filteredExampleImages[index]->height();y++)
	    for(int d=0;d<filteredExampleImages[index]->dim();d++)
	      filteredExampleImages[index]->Pixel(x,y,d) -=
		sourceExampleImages[index]->Pixel(x,y,d);
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
  {
#ifdef STEERDEBUG
    printf("using source images. Calling SetImages\n");
#endif
/*    if (sourcePyramidType ==STEERABLE_PYRAMID)
    {
      fl.SetImages(sourceExampleImages, filteredExampleImages, sourceImage, 
		   pyramidHeight, 
		   useFilterModeMask ? filterModeMask : NULL,
		   useTargetModeMask ? targetModeMask : NULL,
		   steerFilter);
    }
    else
*/    {
      fl.SetImages(sourceExampleImages, filteredExampleImages, sourceImage, 
		   pyramidHeight, 
		   useFilterModeMask ? filterModeMask : NULL,
		   useTargetModeMask ? targetModeMask : NULL);
    }
  }
  else
    fl.SetImages(filteredExampleImages, pyramidHeight);
  
  if (oldSourcesImage != NULL)
  {
    if (oldSourcesImage->width() != 
	fl.sourcesPyramid().FineImage().width()
	|| oldSourcesImage->height() != 
	fl.sourcesPyramid().FineImage().height())
      printf("old sources image dimension doesn't match filtered\n");
    else
      fl.sourcesPyramid().FineImage().copy(*oldSourcesImage);
  }
  
  filteredImage = &fl.filteredPyramid()[0];
  
  adjustWindowSize();
}

void adjustWindowSize(int)
{
//  int outputWidth;
  windowHeight = 0;
  windowWidth = 0;
  
  for( int index = 0; index < fl.NumExamplePairs(); index++ )
    {
      if (showFilteredExample)
	{
	  windowWidth += fl.filteredExamplePyramid(index).displayWidth();
	  windowHeight = max(windowHeight,
			     fl.filteredExamplePyramid(index).displayHeight());
	}
      
      if (showSourceExample)
	{
	  windowWidth += fl.sourceExamplePyramid(index).displayWidth();
	  windowHeight = max(windowHeight,
			     fl.sourceExamplePyramid(index).displayHeight());
	}
    }
  
  if (showExampleModeMask)
    {
      windowWidth += fl.exampleModeMaskPyramid().displayWidth();
      windowHeight = max(windowHeight,
			 fl.exampleModeMaskPyramid().displayHeight());
    }

  if (showSource)
    {
      windowWidth += fl.sourcePyramid().displayWidth();
      windowHeight = max(windowHeight,
			 fl.sourcePyramid().displayHeight());
    }
    
  if (showFiltered)
    {
      windowWidth += fl.filteredPyramid().displayWidth();
      windowHeight = max(windowHeight,
			 fl.filteredPyramid().displayHeight());
    }
  
  if (showReconstruction && outputPyramid != NULL)
    {
      windowWidth += outputPyramid->displayWidth();
      windowHeight = max(windowHeight,outputPyramid->displayHeight());
    }
    
  if (showSources)
    {
      windowWidth += fl.sourcesPyramid().displayWidth();
      windowHeight = max(windowHeight,
			 fl.sourcesPyramid().displayHeight());
    }

  if (showModeMask)
    {
      windowWidth += fl.modeMaskPyramid().displayWidth();
      windowHeight = max(windowHeight,
			 fl.modeMaskPyramid().displayHeight());
    }
  
  if (glui != NULL)
    {
      glutSetWindow(mainWindow);
      glutReshapeWindow(windowWidth, windowHeight);
      glutPostRedisplay();
    }
}


/*
void adjustWindowSize(int=0)
{
  // adjust the display window size
  int outputWidth;
  if (useSourceImages)
    outputWidth = inputSourceImage->width();
  else
    outputWidth = inputFilteredExampleImages[0]->width(); // #### check this
  
  windowWidth = -5;
  
  for( index = 0; index < fl.NumExamplePairs(); index++ )
    {
      if (showFilteredExample)
	windowWidth += inputFilteredExampleImages[0]->width() + 5; // #### check this
      
      if (showSourceExample)
	windowWidth += inputSourceExampleImages[0]->width() + 5; // #### check this
      
      if( showFilteredExample && showSourceExample )
	windowWidth += 5;
    }
    
    if (showExampleModeMask)
      windowWidth += inputSourceExampleImages[0]->width() + 5; // #### check this

    if (showSource)
      windowWidth += inputSourceImage->width() + 5;
    
    if (showFiltered)
      windowWidth += outputWidth +5;

    if (showReconstruction)
      windowWidth += outputWidth +5;
    
    if (showSources)
      windowWidth += outputWidth + 5;
    
    if (showModeMask)
    windowWidth += outputWidth + 5;
    
    if (windowWidth < 0)
      windowWidth = 5;
    
    windowHeight = 10;

    if (showFilteredExample || showSourceExample)
      windowHeight += inputSourceExampleImages[0]->height(); // #### check this
    
    if (showSource || showFiltered || showReconstruction || showSources)
      windowHeight += inputSourceImage->height();
    
  filteredImage = &fl.filteredPyramid()[0];

  if (glui != NULL)
    {
      glutSetWindow(mainWindow);
      glutReshapeWindow(windowWidth, windowHeight);
      glutPostRedisplay();
    }
}*/ 
     
void checkGrayscale()
{
  int index;
  int x,y,d;

  sourceIsGrayscale = true;
  
  for( index = 0; index < inputFilteredExampleImages.size(); index++ )
  {
    if (!(inputFilteredExampleImages[index]->dim() > 1 &&
	  inputFilteredExampleImages[index]->dim()==inputSourceExampleImages[index]->dim()&&
	  inputFilteredExampleImages[index]->dim()==inputSourceImage->dim()))
      return;    
    
    printf("Checking if the input images are grayscale: ");
    
    bool grayscaleMode = true;
    
    sourceIsGrayscale = true;
    
    for(x=0;x<inputSourceImage->width();x++)
      for(y=0;y<inputSourceImage->height();y++)
	for(d=0;d<inputSourceImage->dim();d++)
	  if (inputSourceImage->Pixel(x,y,0) != 
	      inputSourceImage->Pixel(x,y,d)) 
	  {
	    grayscaleMode = false;
	    sourceIsGrayscale = false;
	    printf("No.\n");
	    return;
	  }
    
    for(x=0;x<inputFilteredExampleImages[index]->width();x++)
      for(y=0;y<inputFilteredExampleImages[index]->height();y++)
	for(d=1;d<inputFilteredExampleImages[index]->dim();d++)
	  if (inputFilteredExampleImages[index]->Pixel(x,y,0) != 
	      inputFilteredExampleImages[index]->Pixel(x,y,d))
	  {
	    grayscaleMode = false;
	    printf("No.\n");
	    return;
	  }
    
    
    for(x=0;x<inputSourceExampleImages[index]->width();x++)
      for(y=0;y<inputSourceExampleImages[index]->height();y++)
	for(d=1;d<inputSourceExampleImages[index]->dim();d++)
	  if (inputSourceExampleImages[index]->Pixel(x,y,0) != 
	      inputSourceExampleImages[index]->Pixel(x,y,d))
	  {
	    grayscaleMode = false;
	    printf("No.\n");
	    return;
	  }
  }

  printf("Yes. Converting.\n");

  for( index = 0; index < inputFilteredExampleImages.size(); index++ )
  {
    ColorImage * filteredEx = new ColorImage(inputFilteredExampleImages[index]->width(),
					     inputFilteredExampleImages[index]->height(),
					     1);
    ColorImage * sourceEx = new ColorImage(inputSourceExampleImages[index]->width(),
					   inputFilteredExampleImages[index]->height(),
					   1);
    if (filteredEx == NULL || sourceEx == NULL )
    {
      printf("error: can't allocate grayscale images!!!!\n");
      return;
    }
    
    for(x=0;x<inputFilteredExampleImages[index]->width();x++)
      for(y=0;y<inputFilteredExampleImages[index]->height();y++)
	filteredEx->Pixel(x,y,0) = inputFilteredExampleImages[index]->Pixel(x,y,0);
    
    for(x=0;x<inputSourceExampleImages[index]->width();x++)
      for(y=0;y<inputSourceExampleImages[index]->height();y++)
	sourceEx->Pixel(x,y,0) = inputSourceExampleImages[index]->Pixel(x,y,0);
    
    delete inputFilteredExampleImages[index];
    delete inputSourceExampleImages[index];
    
    inputFilteredExampleImages[index] = filteredEx;
    inputSourceExampleImages[index] = sourceEx;
  }
  
  ColorImage * source = new ColorImage(inputSourceImage->width(),
				       inputSourceImage->height(),1);
  
  if (source == NULL)
  {
    printf("error: can't allocate grayscale images!!!!\n");
    return;
  }
  
  for(x=0;x<inputSourceImage->width();x++)
    for(y=0;y<inputSourceImage->height();y++)
      source->Pixel(x,y,0) = inputSourceImage->Pixel(x,y,0);
  
  delete inputSourceImage;
  inputSourceImage = source;
}    


void
reconstruct(int)
{
  int index;

  // convert back to RGB
  if( filteredImage )
  {
    filteredImage->SetColorspace( (COLORSPACE)filterColorspace );
    filteredImage->ConvertToColorspace( RGB_SPACE );
  }

  if( outputPyramid )
  {
    outputPyramid->SetColorspace( (COLORSPACE)filterColorspace );
    outputPyramid->ConvertToColorspace( RGB_SPACE );
  }
    
  if( outputFilteredImage )
  {
    outputFilteredImage->SetColorspace( (COLORSPACE)filterColorspace );
    outputFilteredImage->ConvertToColorspace( RGB_SPACE );
  }

  for( index = 0; index < inputSourceExampleImages.size(); index++ )
    inputSourceExampleImages[index]->ConvertToColorspace( RGB_SPACE );
  for( index = 0; index < inputFilteredExampleImages.size(); index++ )
    inputFilteredExampleImages[index]->ConvertToColorspace( RGB_SPACE );
  inputSourceImage->ConvertToColorspace( RGB_SPACE );
  
  // extract color from source image
  if (outputPyramid != NULL && outputPyramid != &fl.filteredPyramid())
    delete outputPyramid;

  if (outputFilteredImage != NULL && outputFilteredImage != filteredImage)
    delete outputFilteredImage;

  if (filteredPyramidType == GAUSSIAN_PYRAMID && !convertToYIQ)
    {
      outputPyramid = NULL;
      outputFilteredImage = NULL;
      //      outputPyramid = fl.filteredPyramid();
      //      outputFilteredImage = &outputPyramid->FineImage();
      return;
    }
  
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

      if( useInterface )
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

void saveSources(int=0)
{
  fl.sourcesPyramid().FineImage().saveData("sources.data");
}

void loadSources(int=0)
{
  oldSourcesImage = new Image<GLshort>(sourcesImageName);
}

void loadSourcesCB(int)
{
  loadSources();
  SetupImages(1);
}

void createUI()
{
  // ------------------------ create the UI ----------------------------------
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );

  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize( windowWidth, windowHeight );
 
  mainWindow = glutCreateWindow( "Example-Based Image Filtering" );

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );  
  glutPassiveMotionFunc( mousePassiveMotion );
  glutMouseFunc (mouseClick);

  // --- set up camera projection model and other rendering stuff ----
 
  reshape(windowWidth,windowHeight);

  glClearColor(1,1,1,0);
  //  glClearColor(1.0,.95,.85,0);
  glClearDepth(1.0);

  // --- set up controls ----
    
  glui = GLUI_Master.create_glui( "Controls" );

  reshape(windowWidth,windowHeight);
  
  glui->add_checkbox("Filter",&useSourceImages,1,SetupImages);

  if (filterModeMask != NULL || targetModeMask != NULL)
    {
      GLUI_Panel * mmp = glui->add_panel("Mode mask");
      
      if (filterModeMask != NULL)
	glui->add_checkbox_to_panel(mmp,"Use Filter MM",&useFilterModeMask,
				    1,SetupImages);

      if (targetModeMask != NULL)
	glui->add_checkbox_to_panel(mmp,"Use Target MM",&useTargetModeMask,1,
				    SetupImages);
      
      if (targetModeMask != NULL || filterModeMask != NULL)
	glui->add_spinner_to_panel(mmp,"Mode mask weight",GLUI_SPINNER_FLOAT,
				   &modeMaskWeight)->set_float_limits(0,1);
    }

  glui->add_spinner("Pyramid height",GLUI_SPINNER_INT,&pyramidHeight,1,SetupImages)->set_int_limits(1,10);
  
  GLUI_Panel * span = glui->add_panel("Search Method");

  //  glui->add_spinner_to_panel(span,"X Step",GLUI_SPINNER_INT,&xstep)->set_int_limits(1,10);

  GLUI_RadioGroup * searchrg = glui->add_radiogroup_to_panel(span,&searchType);
  glui->add_radiobutton_to_group(searchrg, "Image");
  glui->add_radiobutton_to_group(searchrg, "Vector");
  glui->add_radiobutton_to_group(searchrg, "TSVQ");
  glui->add_radiobutton_to_group(searchrg, "Heur+TSVQ");
  glui->add_radiobutton_to_group(searchrg, "MLP");
  glui->add_radiobutton_to_group(searchrg, "TSVQR");
  glui->add_radiobutton_to_group(searchrg, "ANN");
  glui->add_radiobutton_to_group(searchrg, "Heur+ANN");
  glui->add_radiobutton_to_group(searchrg, "Ashikhmin");

  glui->add_spinner_to_panel(span,"Passes",GLUI_SPINNER_INT,&numPasses)->
    set_int_limits(1,3);
  glui->add_checkbox_to_panel(span,"One way",&oneway);
  glui->add_checkbox_to_panel(span,"Use source after #1",
			      &useSourceImagesAfterFirstPass);
  glui->add_checkbox_to_panel(span,"Cheesy boundaries",
			      &cheesyBoundaries);
  glui->add_checkbox_to_panel(span,"Ashikhmin last level", &ashikhminLastLevel);

#ifdef LAPACK
  glui->add_checkbox_to_panel(span,"PCA",&usePCA);
#endif

  glui->add_checkbox_to_panel(span,"Save points files",&savePointsToFile);

  GLUI_Rollout * tsr = glui->add_rollout_to_panel(span,"TSVQ settings");
  tsr->close();

  glui->add_spinner_to_panel(tsr,"Max TSVQ depth",GLUI_SPINNER_INT,&maxTSVQdepth);
  glui->add_spinner_to_panel(tsr,"Heur Max TSVQ depth",GLUI_SPINNER_INT,&heurMaxTSVQdepth);
  glui->add_spinner_to_panel(tsr,"Max TSVQ error",GLUI_SPINNER_FLOAT,&maxTSVQerror);
  glui->add_spinner_to_panel(tsr,"TSVQ backtrack",GLUI_SPINNER_INT,&TSVQbacktracks);

  glui->add_spinner_to_panel(span,"ANN epsilon",GLUI_SPINNER_FLOAT,
			     &annEpsilon);
  glui->add_spinner_to_panel(span,"Heur ANN epsilon",GLUI_SPINNER_FLOAT,
			     &heurAnnEpsilon);

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
			     &neighborhoodWidth)->set_int_limits(3,31);
  glui->add_spinner_to_panel(wts,"Num levels",GLUI_SPINNER_INT,
			     &numNeighLevels)->set_int_limits(1,5);
  glui->add_checkbox_to_panel(wts,"Spline weights",&useSplineWeights);
  glui->add_spinner_to_panel(wts,"Level weight",GLUI_SPINNER_FLOAT,
			     &levelWeighting)->set_float_limits(0,5);
  glui->add_spinner_to_panel(wts,"Final source weight",GLUI_SPINNER_FLOAT,
			     &finalSourceFac)->set_float_limits(-1,5);
  glui->add_spinner_to_panel(wts,"Source weight",GLUI_SPINNER_FLOAT,
			     &sourceFac)->set_float_limits(0,1000);
  glui->add_spinner_to_panel(wts,"Coherence eps",GLUI_SPINNER_FLOAT,
			     &coherenceEps);
  glui->add_spinner_to_panel(wts,"Coherence power",GLUI_SPINNER_FLOAT,
			     &coherencePow);
  /*
  glui->add_checkbox_to_panel(wts,"Bias",&useBias);
  glui->add_checkbox_to_panel(wts,"Gain",&useGain);
  glui->add_spinner_to_panel(wts,"Bias Penalty",GLUI_SPINNER_FLOAT,
			     &biasPenalty);
  glui->add_spinner_to_panel(wts,"Gain Penalty",GLUI_SPINNER_FLOAT,
  &gainPenalty);
  */

  //  glui->add_checkbox_to_panel(wts,"Grayscale",&grayscaleMode);

  glui->add_column(true);

  GLUI_Rollout * fts = glui->add_rollout("Features");

  GLUI_Panel *hpn = glui->add_panel_to_panel(fts,"Histograms");

  //  glui->add_checkbox_to_panel(hpn,"Match histograms",&matchGrayHistogram,1,
  //			      SetupImages);

  //  glui->add_checkbox_to_panel(hpn,"Histogram Eq.",&histogramEq,1,SetupImages);

  glui->add_checkbox_to_panel(hpn,"Match A/A' to B",&matchMeanVariance,1,
			      SetupImages);
  glui->add_checkbox_to_panel(hpn,"Match B to A",&matchBtoA,1,SetupImages);
/*
  //steerable filter parameters
  GLUI_Rollout * stfrll = glui->add_rollout("Steerable Filter");
  GLUI_Panel *stfpn = glui->add_panel_to_panel(stfrll, "Filter Properties");
  GLUI_EditText *stfetext = glui->add_edittext_to_panel(stfpn,
							"Filter file",
							GLUI_EDITTEXT_TEXT,
							(void *)steerFilterName,1,
							steerFilterUpdate);
  GLUI_Rollout *stfwconvrll = glui->add_rollout("Fw Convolution");
  stfwconvrll->close();
  GLUI_Panel *stfwconvpn = glui->add_panel_to_panel(stfwconvrll, 
						    "Fw Convolution");  
  GLUI_RadioGroup * stfrg = glui->add_radiogroup_to_panel(stfwconvpn,
							  &steerFilterFwConvType,1,stfChangeParams);
  
  glui->add_radiobutton_to_group(stfrg,"reflect1");
  glui->add_radiobutton_to_group(stfrg,"reflect2");
  glui->add_radiobutton_to_group(stfrg,"repeat");
  glui->add_radiobutton_to_group(stfrg,"zero");
  glui->add_radiobutton_to_group(stfrg,"extend");
  glui->add_radiobutton_to_group(stfrg,"dont-compute");
  glui->add_radiobutton_to_group(stfrg,"predict");
  glui->add_radiobutton_to_group(stfrg,"ereflect");
  glui->add_radiobutton_to_group(stfrg,"treflect");
*/
  GLUI_Panel * spt = glui->add_panel_to_panel(fts,"Source Pyramid Type");
  GLUI_RadioGroup * sptrg = 
    glui->add_radiogroup_to_panel(spt,&sourcePyramidType,1,SetupImages);
  glui->add_radiobutton_to_group(sptrg, "Gaussian");
  glui->add_radiobutton_to_group(sptrg, "Laplacian");
  glui->add_radiobutton_to_group(sptrg, "Steerable");

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

  glui->add_checkbox_to_panel(dis,"Source example",&showSourceExample,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Filtered example",&showFilteredExample,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Example mode mask",&showExampleModeMask,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Source",&showSource,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Filtered",&showFiltered,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Mode mask",&showModeMask,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Sources",&showSources,0,
			      adjustWindowSize);
  glui->add_checkbox_to_panel(dis,"Reconstruction\n",&showReconstruction,0,
			      adjustWindowSize);

  //  glui->add_button("Reconstruct",0,reconstruct);

  glui->add_button("Go",1,go);
  glui->add_checkbox("Multithread",&multithreadedMode);

  glui->add_separator();

  glui->add_button("Save summary",0,saveSummary);
  glui->add_button("Save PNGs",0,savePNGs);
  glui->add_button("Save screenshot",0,saveScreenshot);
  glui->add_button("Save sources",0,saveSources);
  glui->add_button("Load sources",0,loadSourcesCB);
  glui->add_button("Print args",0,printArgs);

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
  //  Neigh * x1 = new Neigh(1,Point2(0,0));
  Neigh * x1 = new Neigh(1,Point2(0,0,0));
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
/*
void biasTest()
{
  Neigh target(5,Point2(-1,1));
  Neigh source(5,Point2(-1,1));
  Neigh wtarget(5,Point2(-1,1));
  Neigh wsource(5,Point2(-1,1));
  Neigh weights(5,Point2(-1,1));

  int i;
  for(i=0;i<5;i++)
    {
      target[i] = i;
      source[i] = i*i;
      weights[i] = 1/5.0;

      wtarget[i] = target[i]*weights[i];
      wsource[i] = source[i]*weights[i];
    }

  printf("ts = %f, ss = %f, ws = %f\n",target.sum(),source.sum(),weights.sum());

  double dist, bias;

  optimalBias(wtarget,wsource,weights,&dist,&bias);

  printf("target = [");
  target.printFloat();
  printf("];\nsource = [");
  source.printFloat();
  printf("];\nweights = [");
  weights.printFloat();

  printf("];\ndist = %f, bias = %f\n",dist,bias);

}
*/

const int MAX_ARG_FILES = 5;
int argFilesRead = 0;

char argFilename[MAX_FILENAME_SIZE] = "";
//char outputName[MAX_FILENAME_SIZE] = "";

// 
void printArgs( int )
{
  argParser.Dump( stdout, false );
}

void ArgFileCallback( const char *name, char *var, int maxLen, ParseArgs *parser )
{
  if( argFilesRead < MAX_ARG_FILES )
  {
    printf( "now parsing args from file %s\n", argFilename );

    char newArgFilename[MAX_FILENAME_SIZE] = "";
    strcpy( newArgFilename, var );
    argFilesRead++;
    
    ParseArgs newArgParser;
    SetupArgParser( newArgParser );  
    newArgParser.ParseInput( newArgFilename );
  }
}
/*
void ParseArgsSteerFilterCallback(const char *name, char *var,
								  int maxLen, ParseArgs *parser)
{
  if( strcmp( name, "steerFilterName:" ) == 0 )
  {
    printf( "Got steer filter name %s\n", var );
	strcpy(steerFilterName,var);
	printf("SteerFilterName : %s\n", steerFilterName);
  }
}
*/
void ExampleCallback( const char *name, char *var, int maxLen, ParseArgs *parser )
{
  if( strcmp( name, "srcExample:" ) == 0 )
  {
    printf( "Got source example image %s\n", var );
    inputSourceExampleNames.push_back( string( var ) );
  }
  else if( strcmp( name, "filteredExample:" ) == 0 )
  {
    printf( "Got filtered example image %s\n", var );
    inputFilteredExampleNames.push_back( string( var ) );
  }
}

void HelpCallback( const char *name, ParseArgs *parser )
{
  fprintf( stderr, "usage:\n" );
  parser->Dump();
  
#if _WIN32
  getchar();
#endif
  
  exit(1);
}

void ValuesCallback( const char *name, ParseArgs *parser )
{
  parser->Dump( stderr, false );
  
#if _WIN32
  getchar();
#endif
  
  exit(1);
}

void SetupArgParser( ParseArgs &parser )
{
  parser.AddFlagParam( "--help", HelpCallback );
  parser.AddFlagParam( "--values", ValuesCallback );

  // file to read for more args
  parser.AddStringParam( "argFile:", argFilename, MAX_FILENAME_SIZE, ArgFileCallback );
  parser.AddBoolParam( "useInterface:", &useInterface );

  // image filenames
  parser.AddStringParam( "srcExample:", inputSourceExampleName, MAX_FILENAME_SIZE, ExampleCallback );
  parser.AddStringParam( "filteredExample:", inputFilteredExampleName, MAX_FILENAME_SIZE, ExampleCallback );
  parser.AddStringParam( "srcImage:", inputSourceName, MAX_FILENAME_SIZE );
  parser.AddStringParam( "filterModeMask:", filterModeMaskName, MAX_FILENAME_SIZE );
  parser.AddStringParam( "targetModeMask:", targetModeMaskName, MAX_FILENAME_SIZE );

  parser.AddStringParam( "outputPattern:", outputSummaryPattern, MAX_FILENAME_SIZE );
  parser.AddStringParam( "outputDirectory:", outputDirectoryName, MAX_FILENAME_SIZE );
  
  parser.AddBoolParam( "useFilter:", &useSourceImages );
  parser.AddIntParam( "pyramidHeight:", &pyramidHeight );

  EnumValueInfo methodValues[] = { { "Image", 0 }, { "Vector", 1 }, { "TSVQ", 2 },
				   { "HeurTSVQ", 3 }, { "MLP", 4 }, { "TSVQR", 5 },
				   { "ANN", 6 }, { "HeurANN", 7 }, { "Ash", 8 } };
  parser.AddEnumParam( "searchMethod:", methodValues, 9, &searchType );
  
  EnumValueInfo outputStyleValues[] = { { "Summary", SUMMARY }, { "Table", TABLE } };
  parser.AddBoolParam( "cheesyBoundaries:", &cheesyBoundaries );
  parser.AddEnumParam( "outputStyle:", outputStyleValues, 2, &outputStyle );

  parser.AddBoolParam( "ashLastLevel:", &ashikhminLastLevel );
  parser.AddBoolParam( "useBias:", &useBias );
  parser.AddBoolParam( "useGain:", &useGain );
  parser.AddFloatParam( "biasPenalty:", &biasPenalty );
  parser.AddFloatParam( "gainPenalty:", &gainPenalty );
  parser.AddIntParam( "numPasses:", &numPasses );
  parser.AddBoolParam( "savePointsFile:", &savePointsToFile );
  parser.AddIntParam( "maxTSVQDepth:", &maxTSVQdepth );
  parser.AddIntParam( "heurMaxTSVQDepth", &heurMaxTSVQdepth );
  parser.AddFloatParam( "maxTSVQError:", &maxTSVQerror );
  parser.AddIntParam( "numTSVQBacktracks:", &TSVQbacktracks );
  parser.AddFloatParam( "annEpsilon:", &annEpsilon );
  parser.AddFloatParam( "heurAnnEpsilon:", &heurAnnEpsilon );
  parser.AddIntParam( "numHiddenNeurons:", &MLPnumHidden );
  parser.AddDoubleParam( "decayWeight:", &MLPdecayWeight );
  parser.AddBoolParam( "useSigmoidalDecay:", &MLPsigmoidal );
  parser.AddBoolParam( "useRandomStart:", &useRandom );
  parser.AddFloatParam( "samplerEpsilon:", &samplerEpsilon );
  parser.AddIntParam( "neighborhoodWidth:", &neighborhoodWidth );
  parser.AddIntParam( "numLevels:", &numNeighLevels );
  parser.AddBoolParam( "useSplineWeights:", &useSplineWeights );
  parser.AddFloatParam( "levelWeighting:", &levelWeighting );
  parser.AddFloatParam( "finalSourceFac:", &finalSourceFac );
  parser.AddFloatParam( "srcWeight:", &sourceFac );
  parser.AddBoolParam( "matchMeanVariance:", &matchMeanVariance);
  parser.AddBoolParam( "matchBtoA:", &matchBtoA);
  parser.AddBoolParam( "matchGrayHistogram:", &matchGrayHistogram);
  parser.AddBoolParam( "histogramEq:", &histogramEq);
  parser.AddBoolParam( "useFilterModeMask:", &useFilterModeMask );
  parser.AddBoolParam( "useTargetModeMask:", &useTargetModeMask );
  parser.AddStringParam( "filterMM:", filterModeMaskName, MAX_FILENAME_SIZE );
  parser.AddStringParam( "targetMM:", targetModeMaskName, MAX_FILENAME_SIZE );
  parser.AddBoolParam( "createSrcLocHisto:", &createSrcLocHisto );
  parser.AddBoolParam( "loadSourcesImage:", &loadSourcesImage);
  parser.AddFloatParam( "modeMaskWeight:", &modeMaskWeight);
  parser.AddFloatParam( "coherenceEps:", &coherenceEps);
  parser.AddFloatParam( "coherencePow:", &coherencePow);
  parser.AddBoolParam( "oneway:", &oneway);
  parser.AddBoolParam( "onePixelSource:", &onePixelSource);

  EnumValueInfo pyramidTypeValues[] = { { "Gaussian", 0 }, { "Laplacian", 1 }, { "Steerable", 2 } };
  parser.AddEnumParam( "pyramidType:", pyramidTypeValues, 3, &sourcePyramidType );
  parser.AddEnumParam( "filteredPyramidType:", pyramidTypeValues, 2, &filteredPyramidType );

  parser.AddBoolParam( "useYIQ:", &convertToYIQ );

  EnumValueInfo featureTypeValues[] = { { "Raw", 0 }, { "Difference", 1 } };
  parser.AddEnumParam( "filteredFeatureType:", featureTypeValues, 2, &filteredFeatureType );

  // TODO: filtered base procedure enum
  EnumValueInfo filterProcTypeValues[] = { { "Synthesize", 0 }, { "Copy", 1 } };
  parser.AddEnumParam( "filterProcedure:", filterProcTypeValues, 2, &filteredBaseProcedure );

  EnumValueInfo colorspaceTypeValues[] = { { "RGB", RGB_SPACE }, { "XYZ", XYZ_SPACE }, { "Lab", LAB_SPACE }, { "Luv", LUV_SPACE } };
  parser.AddEnumParam( "filterColorspace:", colorspaceTypeValues, 4, &filterColorspace );
  parser.AddEnumParam( "sourceColorspace:", colorspaceTypeValues, 4, &sourceColorspace );

//  parser.AddStringParam( "steerFilterName:", steerFilterFileName, MAX_FILENAME_SIZE, ParseArgsSteerFilterCallback);
}

/**************************************** main() ********************/

void main(int argc, char* argv[])
{
  // this is necessary if we're profiling on windows. 
  // #define PROFILING_WINDOWS
#ifdef PROFILING_WINDOWS
  printf( "Profiling... changing directory...\n" );
  ::SetCurrentDirectory( ".." );
#endif

  inputSourceExampleNames.erase( inputSourceExampleNames.begin(), inputSourceExampleNames.end() );
  inputFilteredExampleNames.erase( inputFilteredExampleNames.begin(), inputFilteredExampleNames.end() );
  
  // read cmd-line args
  argFilesRead = 0;
  SetupArgParser( argParser );
  
  // read params from command line:
  argParser.ParseInput( argc, argv );
  
  // add default pair, if necessary
  if( inputSourceExampleNames.size() == 0 )
    inputSourceExampleNames.push_back( string( inputSourceExampleName ) );
  
  if( inputFilteredExampleNames.size() == 0 )
    inputFilteredExampleNames.push_back( string( inputFilteredExampleName ) );
  
  // print out example pairs:
  if( useSourceImages )
  {
    assert( inputSourceExampleNames.size() == inputFilteredExampleNames.size() );
    
    printf( "example pairs:\n" );
    for( int index = 0; index < inputSourceExampleNames.size(); index++ )
    {
      printf( "src %d: %s\n", index, inputSourceExampleNames[index].c_str() );
      printf( "filt %d: %s\n", index, inputFilteredExampleNames[index].c_str() );
    }
  }
  else
  {
    printf( "example src:\n" );
    for( int index = 0; index < inputSourceExampleNames.size(); index++ )
      printf( "src %d: %s\n", index, inputSourceExampleNames[index].c_str() );
  }
  
  // cheesy thing to set output filenames: (obviously, not done yet)
  /*
    sprintf( outputSummaryName, "%s-summary.png", outputName );
    sprintf( outputFilteredName, "%s-output.png", outputName );
    sprintf( filteredName, "%s-filtered.png", outputName );
    sprintf( summaryHTMLName, "%s.html", outputName );
  */
  //  for(;;) ;
  
  //  MLPtest();
  
  //  biasTest();
  
  //  return;
  
  //  ------------------  load all the images  -----------------------
  inputSourceImage = new ColorImage(inputSourceName);
  if( inputSourceImage == NULL )
  {
    printf( "Error: can't allocate image\n" );
    exit( 1 );
  }

  int index;
  inputFilteredExampleImages.erase( inputFilteredExampleImages.begin(), inputFilteredExampleImages.end() );
  for( index = 0; index < inputFilteredExampleNames.size(); index++ )
  {
    ColorImage *img = new ColorImage( inputFilteredExampleNames[index].c_str() );
    if( img == NULL )
    {
      printf( "Error: can't allocate image\n" );
      exit( 1 );
    }

    inputFilteredExampleImages.push_back( img );
  }
  
  inputSourceExampleImages.erase( inputSourceExampleImages.begin(), inputSourceExampleImages.end() );
  for( index = 0; index < inputSourceExampleNames.size(); index++ )
  {
    ColorImage *img = new ColorImage( inputSourceExampleNames[index].c_str() );
    if( img == NULL )
  {
    printf( "Error: can't allocate image\n" );
    exit( 1 );
  }
  
  inputSourceExampleImages.push_back( img );
  }
  
  //  inputFilteredExampleImage = new ColorImage(inputFilteredExampleName);
  //  inputSourceExampleImage = new ColorImage(inputSourceExampleName);
  for( index = 0; index < inputSourceExampleImages.size(); index++ )
  {
    if (inputSourceExampleImages[index]->width() != inputFilteredExampleImages[index]->width() ||
	inputSourceExampleImages[index]->height() != inputFilteredExampleImages[index]->height())
    {
      printf("Example image dimensions don't match\n");
      exit(1);
    }
  }
  
  if (useFilterModeMask)
    {
      if( inputSourceExampleImages.size() > 1 )
	printf( "Warning: mode mask currently doesn't work with more than one example pair\n" );
      
      filterModeMask = new ColorImage(filterModeMaskName);
      if (filterModeMask->width() != inputSourceExampleImages[0]->width() ||
	  filterModeMask->height() != inputSourceExampleImages[0]->height())
	{
	  printf("Mode mask dimensions don't match\n");
	  exit(1);
	}
    }

  if (useTargetModeMask)
    {
      targetModeMask = new ColorImage(targetModeMaskName);
      if (targetModeMask->width() != inputSourceImage->width() ||
	  targetModeMask->height() != inputSourceImage->height())
	{
	  printf("Target mode mask dimensions don't match\n");
	  exit(1);
	}
    }


  // choose the pyramid height
  if (pyramidHeight <= 0)
    {
      int mindim = min(inputSourceExampleImages[0]->width(),
		       inputSourceExampleImages[0]->height());
      mindim = min(mindim, inputSourceImage->width());
      mindim = min(mindim, inputSourceImage->height());
      
      pyramidHeight = int(floor(log(mindim)/log(2)));
    }

  if (loadSourcesImage)
    {
      loadSources();
    }

  //#ifdef STEERDEBUG
  printf("Using %d pyramid levels\n",pyramidHeight);
  //#endif

  fl.filteredPyramidType() = (PyramidType)filteredPyramidType;
  fl.sourcePyramidType() = (PyramidType)sourcePyramidType;

  checkGrayscale();
  /*
  windowWidth = inputSourceExampleImage->width() + inputSourceImage->width()+5;
  if (fl.useSourceImages())
    windowWidth += inputFilteredExampleImage->width() + outputFilteredImage->width()+10;
  windowHeight = 2*max(inputSourceExampleImage->height(), inputSourceImage->height())+20;
  */

  SetupImages(0);

  if( useInterface )
  {
    createUI();
    
    // --- enter the main loop --- 
    glutMainLoop();
  }
  else
  {
    fprintf( stderr, "running with no GUI\n" );

    // 'Go'
    writeHTMLFile();
    //    saveOutput(); // write the params first, so we can recover
    computeOutput();
    saveOutput();

#ifdef _WIN32
    fprintf( stderr, "\n<end>\n" );
    fflush( stderr );
#ifndef PROFILING_WINDOWS
    getchar();
#endif
#endif
  }
}

void saveOutput( int )
{
  // select the output name
  if (summaryIndex < 0)
    {
      summaryIndex = 0;
      while(1)
	{
	  sprintf(outputSummaryName,outputSummaryPattern,summaryIndex++);
	  FILE * fp = fopen(outputSummaryName,"rb");
	  if (fp == NULL)
	    break;
	  fclose(fp);
	}
      summaryIndex --;
      sprintf(summaryHTMLName, summaryHTMLPattern, summaryIndex);
    }

  if( outputStyle == TABLE )
  {
    saveTable();
  }
  else
  {
    saveSummary();
    saveSources();
    //    writeHTMLFile();
  }
}

void saveTable( void )
{
  char outputSourceExampleName[MAX_FILENAME_SIZE];
  char outputFilteredExampleName[MAX_FILENAME_SIZE];
  char outputSourceName[MAX_FILENAME_SIZE];

  char outputFilteredYIQName[MAX_FILENAME_SIZE];
  char outputFilteredGrayName[MAX_FILENAME_SIZE];
  char outputFilteredColorName[MAX_FILENAME_SIZE];

  const char *filename;
  char buffer[MAX_FILENAME_SIZE];

  // remove extension
  char outputFilteredPrefix[MAX_FILENAME_SIZE];
  strcpy( outputFilteredPrefix, outputFilteredName );
  char *suffix = strstr( outputFilteredPrefix, ".png" );
  if( suffix )
    *suffix = '\0';

  // fill in output filenames
  sprintf( outputFilteredYIQName, "%s-yiq.png", outputFilteredName );
  sprintf( outputFilteredGrayName, "%s-gray.png", outputFilteredName );
  sprintf( outputFilteredColorName, "%s-color.png", outputFilteredName );

  // create output directory
#ifdef _WIN32
  CreateDirectory( outputDirectoryName, NULL );
#else
  mkdir( outputDirectoryName, 0777 );
#endif

  // feh. this is currently hosed for > 1 example pair
  int imageIndex = 0;
  
  // save A

  filename = strrchr( inputSourceExampleNames[imageIndex].c_str(), '/' );
  if( filename == NULL )
    filename = inputSourceExampleNames[imageIndex].c_str();
  else
    filename++; // skip the slash
  
  strcpy( outputSourceExampleName, filename );
  sprintf( buffer, "%s/%s", outputDirectoryName, outputSourceExampleName );
  
  if( inputSourceExampleImages[imageIndex] != NULL )
    inputSourceExampleImages[imageIndex]->savePNG( buffer );
  
  // save f(A)
  filename = strrchr( inputFilteredExampleNames[imageIndex].c_str(), '/' );
  if( filename == NULL )
    filename = inputFilteredExampleNames[imageIndex].c_str();
  else
    filename++; // skip the slash
  
  strcpy( outputFilteredExampleName, filename );
  sprintf( buffer, "%s/%s", outputDirectoryName, outputFilteredExampleName );
  if( inputFilteredExampleImages[imageIndex] != NULL )
    inputFilteredExampleImages[imageIndex]->savePNG( buffer );
      
  // save B
  filename = strrchr( inputSourceName, '/' );
  if( filename == NULL )
    filename = inputSourceName;
  else
    filename++; // skip the slash
  
  strcpy( outputSourceName, filename );
  sprintf( buffer, "%s/%s", outputDirectoryName, outputSourceName );
  if( inputSourceImage != NULL )
    inputSourceImage->savePNG( buffer );
  
  // save f~(B)
  if( convertToYIQ )
  {
    // save f~_yiq(B), f~_yiq(B).y:
    sprintf( buffer, "%s/%s", outputDirectoryName, outputFilteredGrayName );
    filteredImage->savePNG( buffer );

    if( outputFilteredImage != NULL )
    {
      sprintf( buffer, "%s/%s", outputDirectoryName, outputFilteredYIQName );
      outputFilteredImage->savePNG( buffer );
    }    
  }
  else
  {
    // save f~_c(B)
    sprintf( buffer, "%s/%s", outputDirectoryName, outputFilteredColorName );
    filteredImage->savePNG( buffer );
  }
  
  // Table layout:
  //
  //    A      f(A)     f(A).y
  //    B      f(B)     f(B).y
  // f~_c(B)  f~_y(B)  f~_y(B).y

  
  char outputHTMLName[MAX_FILENAME_SIZE];
  sprintf( outputHTMLName, "%s/index.html", outputDirectoryName );
  ofstream outputHTMLFile( outputHTMLName );
  outputHTMLFile << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">" << endl;
  outputHTMLFile << "<HTML>" << endl << "<HEAD>" << endl;
  outputHTMLFile << "  <TITLE>" << outputHTMLName << "</TITLE>" << endl;
  outputHTMLFile << "</HEAD>" << endl;

  outputHTMLFile << "<BODY" << endl;
  outputHTMLFile << "bgcolor=\"#ffffe0\" text=\"#100000\" vlink=\"#101030\" link=\"#002040\" alink=\"#008000\">" << endl;


  // TODO: write table here
  outputHTMLFile << "<TABLE border=\"0\" cellspacing=\"8\">" << endl;
  outputHTMLFile << "  <TR>" << endl;
  outputHTMLFile << "    <TD><IMG src=\"" << outputSourceExampleName << "\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputFilteredExampleName << "\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputFilteredExampleName << "-gray\"> </TD>" << endl; 
  outputHTMLFile << "  </TR>" << endl;

  outputHTMLFile << "  <TR>" << endl;
  outputHTMLFile << "    <TD><IMG src=\"" << outputSourceName << "\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputSourceName << "-filtered.png\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputSourceName << "-filtered-gray.png\"> </TD>" << endl; 

  outputHTMLFile << "  <TR>" << endl;
  outputHTMLFile << "    <TD><IMG src=\"" << outputFilteredColorName << "\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputFilteredGrayName << "\"> </TD>" << endl; 
  outputHTMLFile << "    <TD><IMG src=\"" << outputFilteredYIQName << "\"> </TD>" << endl; 

  outputHTMLFile << "  </TR>" << endl;
  outputHTMLFile << "</TABLE>" << endl;


  outputHTMLFile << "<BR>" << endl;
  outputHTMLFile << "<H2> Parameters:</H2>" << endl;

  argParser.DumpValuesHTML( outputHTMLFile );

  outputHTMLFile << "<br>" << endl;
  outputHTMLFile << "Elapsed time: " << timeToProcess << "s" << endl;

  outputHTMLFile << "</BODY>" << endl;
  outputHTMLFile << "</HTML>" << endl;
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

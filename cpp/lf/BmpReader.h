#ifndef BMP_READER_H

#define BMP_READER_H



#include <stdio.h>

#include "compat.h"

#include <GL/gl.h>



typedef GLushort UNSIGNED16; // a 16-bit value

typedef GLuint UNSIGNED32; // a 32-bit value

typedef GLint SIGNED32; // a 32-bit value



// file format of a bmp file:

// the string 'BM'. for some reason, i find this signature fitting, and hilarious.

// BmpFileHeader

// BmpInfoHeader

// colormap

// pixel data



struct BmpFileHeader

{

  UNSIGNED32  bfSize;       // # of bytes for bitmap file

  UNSIGNED16  bfReserved1;  // should be zero

  UNSIGNED16  bfReserved2;  // should be zero

  UNSIGNED32  bfOffBits;    // offset info the file where the bits are located

};



struct BmpInfoHeader

{

  UNSIGNED32  biSize;           // size of this struct

  SIGNED32    biWidth;          // image width

  SIGNED32    biHeight;         // image height (can be negative -- negative means top-down orientation)

  UNSIGNED16  biPlanes;         // # bitplanes (must be 1)

  UNSIGNED16  biBitCount;       // bits per pixel

  UNSIGNED32  biCompression;    // BI_RGB, BI_RLE8, BI_RLE4, BI_BITFIELDS, BI_JPEG, BI_PNG

  UNSIGNED32  biSizeImage;      // size in bytes -- can be zero for BI_RGB (uncompressed) images

  SIGNED32    biXPelsPerMeter;  // _pels_?!?!? wot the bloody 'ell?!?

  SIGNED32    biYPelsPerMeter; 

  UNSIGNED32  biClrUsed;        // # color indices in color table actually used

  UNSIGNED32  biClrImportant;   // # colors in color table important for displaying image

};





class CBmpReader

{

public:

  CBmpReader( void );



  bool ReadFile( const char *filename );



  unsigned char *PixelData( void ) { return m_pixelData; }

  int Width( void ) { return m_infoHeader.biWidth; }

  int Height( void ) { return m_infoHeader.biHeight; }

  int BytesPerPixel( void ) { return m_infoHeader.biBitCount / 8; }

  

protected:

  bool ReadType( FILE *fp );

  bool ReadFileHeader( FILE *fp );

  bool ReadInfoHeader( FILE *fp );

  bool ReadPixelData( FILE *fp );



  // header info

  BmpFileHeader m_fileHeader;

  BmpInfoHeader m_infoHeader;



  // the pels:

  unsigned char *m_pixelData;

};



#endif // BMP_READER_H




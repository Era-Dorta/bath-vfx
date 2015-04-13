#include "BmpReader.h"

enum
{
  COMP_RGB = 0,
  COMP_RLE8,
  COMP_RLE4,
  COMP_BITFIELDS,
  COMP_JPEG,
  COMP_PNG
};


CBmpReader::CBmpReader( void )
{
  m_pixelData = NULL;
}

bool
CBmpReader::ReadFile( const char *filename )
{
  // a bmp file is laid out on disk like this:
  //   'BM'
  //   BmpFileHeader
  //   BmpInfoHeader
  //   colortable (possibly empty)
  //   pixel data

  FILE *fp = fopen( filename, "rb" );
  if( !fp )
  {
    fprintf( stderr, "Couldn't open image file %s\n", filename );
    return false;
  }

  if( !ReadType( fp ) )
  {
    fprintf( stderr, "Bad type code for image file %s\n", filename );
    fclose( fp );
    return false;
  }

  if( !ReadFileHeader( fp ) )
  {
    fprintf( stderr, "Invalid file header for image file %s\n", filename );
    fclose( fp );
    return false;
  }

  if( !ReadInfoHeader( fp ) )
  {
    fprintf( stderr, "Invalid info header for image file %s\n", filename );
    fclose( fp );
    return false;
  }

  if( !ReadPixelData( fp ) )
  {
    fprintf( stderr, "Couldn't read pixel data for image file %s\n", filename );
    fclose( fp );
    return false;
  }

  fprintf( stderr, "image size: %d x %d\n", m_infoHeader.biWidth, m_infoHeader.biHeight );
  fprintf( stderr, "image data offset: %d\n", m_fileHeader.bfOffBits );

  return true;
}

bool CBmpReader::ReadType( FILE *fp )
{
  char type[2];
  int numRead = fread( type, 1, 2, fp );
  if( numRead != 2 )
    return false;
  
  // The unique header on a .bmp file that identifies it as such is
  // the string "BM". Pathetic, no?
  return type[0] == 'B' && type[1] == 'M';
}

bool CBmpReader::ReadFileHeader( FILE *fp )
{
  int numRead = fread( &m_fileHeader, sizeof( m_fileHeader ), 1, fp );
  if( numRead != 1 )
    return false;

  // TODO: sanity check on contents of file header

  return true;
}

bool CBmpReader::ReadInfoHeader( FILE *fp )
{
  int numRead = fread( &m_infoHeader, sizeof( m_infoHeader ), 1, fp );
  if( numRead != 1 )
    return false;

  // check to see if it's in a format we understand
  if( m_infoHeader.biCompression != COMP_RGB )
    return false;

  if( m_infoHeader.biBitCount != 24 && m_infoHeader.biBitCount != 32 )
    return false;

  return true;
}

bool CBmpReader::ReadPixelData( FILE *fp )
{
  if( m_pixelData )
    delete[] m_pixelData;

  if( fseek( fp, m_fileHeader.bfOffBits, SEEK_SET ) != 0 )
    return false;

  int bytesPerPixel = m_infoHeader.biBitCount / 8;
  int size = m_infoHeader.biWidth * m_infoHeader.biHeight * bytesPerPixel;
  m_pixelData = new unsigned char[ size ];
  int numRead = fread( m_pixelData, 1, size, fp );
  if( numRead != size )
  {
    delete[] m_pixelData;
    m_pixelData = NULL;
    return false;
  }

  return true;
}

#include <stdio.h>
#include "ParseArgs.h"
#include "compat.h"


void ParseArgs::ParseInput( int argc, char *argv[] )
{
  m_inputParams.erase( m_inputParams.begin(), m_inputParams.end() );

  for( int index = 1; index < argc; index++ )
  {
    m_inputParams.push_back( std::string( argv[index] ) );
  }

  ParseInput();
}


void ParseArgs::ParseInput( char *filename )
{
  fprintf( stderr, "Parsing args in file %s\n", filename );

  const int BUFLEN = 2048;
  char buffer[ BUFLEN+1 ];
  std::string name;
  ifstream inFile( filename );

  while( !inFile.eof() )
  {
    // read a line
    inFile.getline( buffer, BUFLEN );

    // if it's a comment, go to next line
    if( buffer[0] == '#' )
      continue;

    istrstream lineStream( buffer );
    while( lineStream >> name )
      m_inputParams.push_back( name );
  }

  ParseInput();
}

void ParseArgs::ParseInput( void )
{
  for( int index = 0; index < m_inputParams.size(); index++ )
  {
    if( m_paramDict.find( m_inputParams[index] ) != m_paramDict.end() )
    {
      ParamInfo info = m_paramDict[ m_inputParams[index] ];

      switch( info.type )
      {
      case FLAG_TYPE:

	if( info.callback )
	{
	  FlagCallbackType *theCallback = (FlagCallbackType *)info.callback;
	  theCallback( info.name.c_str(), this );
	}

	break;
	
      case INT_TYPE:
	index++;
	if( index < m_inputParams.size() )
	  *((int *)(info.var)) = atoi( m_inputParams[index].c_str() );

	if( info.callback )
	{
	  IntCallbackType *theCallback = (IntCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (int *)info.var, this );
	}

	break;
	
      case FLOAT_TYPE:
	index++;
	if( index < m_inputParams.size() )
	{
	  *((float *)(info.var)) = atof( m_inputParams[index].c_str() );
	}

	if( info.callback )
	{
	  FloatCallbackType *theCallback = (FloatCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (float *)info.var, this );
	}

	break;
	
      case DOUBLE_TYPE:
	index++;
	if( index < m_inputParams.size() )
	{
	  *((double *)(info.var)) = atof( m_inputParams[index].c_str() );
	}

	if( info.callback )
	{
	  DoubleCallbackType *theCallback = (DoubleCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (double *)info.var, this );
	}

	break;
	
      case BOOL_TYPE:
	// TODO: try to find next arg, if we don't use it as a value, else assume true
	index++;
	if( index < m_inputParams.size() )
	{
	  if( m_inputParams[index] == "false" || m_inputParams[index] == "FALSE" )
	  {
	    *((bool *)(info.var)) = false;
	  }
	  else
	  {
	    *((bool *)(info.var)) = true;
	  }
	}

	if( info.callback )
	{
	  BoolCallbackType *theCallback = (BoolCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (bool *)info.var, this );
	}

	break;

      case BOOL_INT_TYPE:
	// TODO: try to find next arg, if we don't use it as a value, else assume true
	index++;
	if( index < m_inputParams.size() )
	{
	  if( m_inputParams[index] == "false" || m_inputParams[index] == "FALSE" )
	  {
	    *((int *)(info.var)) = 0;
	  }
	  else
	  {
	    *((int *)(info.var)) = 1;
	  }
	}

	if( info.callback )
	{
	  BoolIntCallbackType *theCallback = (BoolIntCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (int *)info.var, this );
	}

	break;

      case ENUM_TYPE:
	index++;
	if( index < m_inputParams.size() )
	{
	  std::string valName = m_inputParams[index];
	  if( (info.enumMap).find( valName ) != (info.enumMap).end() )
	  {
	    *((int *)(info.var)) = info.enumMap[valName];
	  }
	  else
	  {
	    fprintf( stderr, "didn't find enum value %s\n", valName.c_str() );
	  }
	}

	if( info.callback )
	{
	  EnumCallbackType *theCallback = (EnumCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (int *)info.var, this );
	}
	// ???
	break;

      case STRING_TYPE:
	index++;
	if( index < m_inputParams.size() )
	  strncpy( (char *)(info.var), (const char *)m_inputParams[index].c_str(), info.maxStringLength );

	if( info.callback )
	{
	  StringCallbackType *theCallback = (StringCallbackType *)info.callback;
	  theCallback( info.name.c_str(), (char *)info.var, info.maxStringLength, this );
	}

	break;
      }
    }
    else
      fprintf( stderr, "unrecognized param %s\n", m_inputParams[index].c_str() );
  }
}



void ParseArgs::AddFlagParam( const char *name, FlagCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = FLAG_TYPE;
  info.var = NULL;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::AddIntParam( const char *name, int *var, IntCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = INT_TYPE;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::AddFloatParam( const char *name, float *var, FloatCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = FLOAT_TYPE;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::AddDoubleParam( const char *name, double *var, DoubleCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = DOUBLE_TYPE;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::AddBoolParam( const char *name, bool *var, BoolCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = BOOL_TYPE;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::AddBoolParam( const char *name, int *var, BoolIntCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = BOOL_INT_TYPE;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}


// really an int, with names for values
void ParseArgs::AddEnumParam( const char *name, EnumValueInfo *enumList, int numVals, int *var, EnumCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = ENUM_TYPE;

  // add stuff to lookup map
  for( int index = 0; index < numVals; index++ )
  {
    EnumValueInfo &valInfo = enumList[index];
    info.enumMap[ std::string( valInfo.name ) ] = valInfo.value;
    info.reverseEnumMap[ valInfo.value ] = std::string( valInfo.name );
  }

  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}


void ParseArgs::AddStringParam( const char *name, char *var, int maxLength, StringCallbackType *callback )
{
  ParamInfo info;
  info.name = std::string( name );
  info.type = STRING_TYPE;
  info.maxStringLength = maxLength;
  info.var = var;
  info.callback = (void*)callback;

  m_paramDict[info.name] = info;
}

void ParseArgs::Dump( FILE *outFile, bool dumpType )
{
  std::map< std::string, ParamInfo, std::less<std::string> >::iterator mapIter = m_paramDict.begin();

  while( mapIter != m_paramDict.end() )
  {
    pair<std::string, ParamInfo> item = *mapIter;
    ParamInfo &info = item.second;

    // icky hack...
    if( info.type != FLAG_TYPE )
      fprintf( outFile, "%s ", item.first.c_str() );

    switch( info.type )
    {
    case FLAG_TYPE:
      if( dumpType )
	fprintf( outFile, "%s [flag]\n", item.first.c_str() );
      break;

    case INT_TYPE:
      if( dumpType )
	fprintf( outFile, " [int] =" );
      fprintf( outFile, " %d\n", *((int *)info.var) );
      break;

    case FLOAT_TYPE:
      if( dumpType )
	fprintf( outFile, " [float] =" );
      fprintf( outFile, " %f\n", *((float *)info.var) );
      break;

    case DOUBLE_TYPE:
      if( dumpType )
	fprintf( outFile, " [double] =" );
      fprintf( outFile, " %f\n", *((double *)info.var) );
      break;

    case BOOL_TYPE:
      if( dumpType )
	fprintf( outFile, " [bool] =" );
      fprintf( outFile, " %s\n", *((bool *)info.var) ? "true" : "false" );
      break;

    case BOOL_INT_TYPE:
      if( dumpType )
	fprintf( outFile, " [bool] =" );
      fprintf( outFile, " %s\n", *((int *)info.var) ? "true" : "false" );
      break;

    case ENUM_TYPE:
      if( dumpType )
	fprintf( outFile, " [enum] =" );
      
      if( (info.reverseEnumMap).find( *((int *)info.var) ) != (info.reverseEnumMap).end() )
      {
	fprintf( outFile, " %s", info.reverseEnumMap[ *((int *)info.var) ].c_str() );
      }
      else
	fprintf( outFile, " ???" );

      fprintf( outFile, "\n" );
      break;

    case STRING_TYPE:
      if( dumpType )
	fprintf( outFile, " [string] =" );
      fprintf( outFile, " %s\n", (char *)info.var );
      break;
    }

    if( dumpType && info.type == ENUM_TYPE )
    {
      std::map< std::string, int, std::less<std::string> >::iterator enumIter = info.enumMap.begin();
      while( enumIter != info.enumMap.end() )
      {
	pair<std::string, int> enumItem = *enumIter;
	fprintf( outFile, "\t%s\n", enumItem.first.c_str() );

	enumIter++;
      }
    }

    mapIter++;
  }
}

void ParseArgs::DumpArgumentsHTML( ofstream &out )
{
  std::map< std::string, ParamInfo, std::less<std::string> >::iterator mapIter = m_paramDict.begin();

  out << "<pre>" << endl << endl;

  while( mapIter != m_paramDict.end() )
  {
    pair<std::string, ParamInfo> item = *mapIter;
    ParamInfo &info = item.second;
    out << item.first;

    switch( info.type )
    {
    case INT_TYPE:
      out << " [int]" << endl;
      break;
    case FLOAT_TYPE:
      out << " [float]" << endl;
      break;
    case DOUBLE_TYPE:
      out << " [double]" << endl;
      break;
    case BOOL_TYPE:
      out << " [bool]" << endl;
      break;
    case BOOL_INT_TYPE:
      out << " [bool]" << endl;
      break;
    case ENUM_TYPE:
      out << " [enum]:" << endl;
      break;
    case STRING_TYPE:
      out << " [string]" << endl;
      break;
    }

    if( info.type == ENUM_TYPE )
    {
      std::map< std::string, int, std::less<std::string> >::iterator enumIter = info.enumMap.begin();
      while( enumIter != info.enumMap.end() )
      {
	pair<std::string, int> enumItem = *enumIter;
	out << "\t" << enumItem.first << endl;

	enumIter++;
      }
    }

    mapIter++;
  }

  out << endl << "</pre>" << endl;
}

void ParseArgs::DumpValuesHTML( ofstream &out )
{
  std::map< std::string, ParamInfo, std::less<std::string> >::iterator mapIter = m_paramDict.begin();

  out << "<pre>" << endl;

  while( mapIter != m_paramDict.end() )
  {
    pair<std::string, ParamInfo> item = *mapIter;
    ParamInfo &info = item.second;
    out << item.first << " ";

    switch( info.type )
    {
    case INT_TYPE:
      out << *((int *)info.var) << endl;
      break;
    case FLOAT_TYPE:
      out << *((float *)info.var) << endl;
      break;
    case DOUBLE_TYPE:
      out << *((double *)info.var) << endl;
      break;
    case BOOL_TYPE:
      if( *((bool *)info.var) )
	out << "true" << endl;
      else
	out << "false" << endl;
      break;
    case BOOL_INT_TYPE:
      if( *((int *)info.var) )
	out << "true" << endl;
      else
	out << "false" << endl;
      break;
    case ENUM_TYPE:
      {
	int value = *((int *)info.var);
	if( (info.reverseEnumMap).find( value ) != (info.reverseEnumMap).end() )
	{
	  out << info.reverseEnumMap[ value ] << endl;
	}
	else
	  out << *((int *)info.var) << endl;
      }
      break;
    case STRING_TYPE:
      out << (char *)info.var << endl;
      break;
    }

    /*
    if( info.type == ENUM_TYPE )
    {
      std::map< std::string, int, std::less<std::string> >::iterator enumIter = info.enumMap.begin();
      while( enumIter != info.enumMap.end() )
      {
	pair<std::string, int> enumItem = *enumIter;
	out << "\t" << enumItem.first << endl;

	enumIter++;
      }
    }
    */

    mapIter++;
  }

  out << endl << "</pre>" << endl;
}

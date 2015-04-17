#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "compat.h"
#include <vector>
#include <map>
#include <string>
#include <stdio.h>

class ParseArgs;

typedef void FlagCallbackType( const char *name, ParseArgs *parser );
typedef void IntCallbackType( const char *name, int *var, ParseArgs *parser );
typedef void FloatCallbackType( const char *name, float *var, ParseArgs *parser );
typedef void DoubleCallbackType( const char *name, double *var, ParseArgs *parser );
typedef void BoolCallbackType( const char *name, bool *var, ParseArgs *parser );
typedef void BoolIntCallbackType( const char *name, int *var, ParseArgs *parser );
typedef void EnumCallbackType( const char *name, int *var, ParseArgs *parser );
typedef void StringCallbackType( const char *name, char *var, int maxLen, ParseArgs *parser );


struct EnumValueInfo
{
  char *name;
  int value;
};


class ParseArgs
{
public:
  void ParseInput( int argc, char *argv[] );
  void ParseInput( char *argFilename );

  // add descriptions of params
  void AddFlagParam( const char *name, FlagCallbackType *callback = NULL );
  void AddIntParam( const char *name, int *var, IntCallbackType *callback = NULL );
  void AddFloatParam( const char *name, float *var, FloatCallbackType *callback = NULL );
  void AddDoubleParam( const char *name, double *var, DoubleCallbackType *callback = NULL );
  void AddBoolParam( const char *name, bool *var, BoolCallbackType *callback = NULL );
  void AddBoolParam( const char *name, int *var, BoolIntCallbackType *callback = NULL );
  void AddEnumParam( const char *name, EnumValueInfo *info, int numVals, int *var, EnumCallbackType *callback = NULL ); // really an int, with names for values
  void AddStringParam( const char *name, char *var, int maxLength, StringCallbackType *callback = NULL );
  enum { MAX_NAME_LENGTH = 100 };

  void Dump( FILE *fp = stderr, bool dumpType = true );
  void DumpArgumentsHTML( ofstream &out );
  void DumpValuesHTML( ofstream &out );

protected:

  void ParseInput( void );
  std::vector<std::string> m_inputParams;

  enum ArgType { FLAG_TYPE, INT_TYPE, FLOAT_TYPE, DOUBLE_TYPE, BOOL_TYPE, BOOL_INT_TYPE, ENUM_TYPE, STRING_TYPE };
  struct ParamInfo
  {
    std::string name;
    ArgType type;
    std::map< std::string, int, std::less<std::string> > enumMap;
    std::map< int, std::string, std::less<int> > reverseEnumMap;
    int maxStringLength;
    void *var;
    void *callback;
  };

  std::map< std::string, ParamInfo, std::less<std::string> > m_paramDict;
};

#endif // _PARSE_ARGS_H

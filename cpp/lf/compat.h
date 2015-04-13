#ifndef __COMPAT_H__

#define __COMPAT_H__



#include <assert.h>



// turn out the visual studio warnings (identifier was truncated) 

#ifdef _WIN32

#pragma warning(disable:4786)

#endif



// Windows-specific include for OpenGL and GLUT

#ifdef _WIN32 

#include <windows.h>

#endif



// currently, SGI's STL does not have the new-style iostream 



#if defined(_WIN32) || defined(__GNUG__)
//#if defined(__GNUG__)

#include <iostream>

#include <fstream>

#include <strstream>

#else 

#include <iostream.h>

#include <fstream.h>

#if defined(_WIN32)
#include <strstrea.h>  // windows headers, like so much else, are laughably broken
#else
#include <strstream.h>
#endif

#endif



// 2.8.1 gcc does not support namespaces

// in other cases need this to avoid std:: prefixes

#if ( defined(_WIN32) || defined(__sgi) ) && !defined(__GNUG__)

namespace std {}

using namespace std;

#endif



// standard C++ does not seem to have bidirectional_iterator

// defined; however, both gcc and SGI CC have it

#ifdef _WIN32

template <class T,class dist> 

class bidirectional_iterator: public iterator<bidirectional_iterator_tag,T,dist>

{};

#endif



// includes min/max declaration



#if (defined(__sgi) || defined(__GNU__))

#include <algobase.h>

#endif



// dealing with Microsoft min/max mess: 

//assume that under Windows the code is compiled with NOMINMAX defined

// which disables #define's for min/max.

// however, Microsoft  violates the C++ standard even with 

// NOMINMAX on, and defines templates _cpp_min and _cpp_max 

// instead of templates min/max

// define the correct templates here

/*

#ifdef _WIN32



template<class _Ty> 

inline

        const _Ty& max(const _Ty& _X, const _Ty& _Y)

        {return (_X < _Y ? _Y : _X); }



template<class _Ty> 

inline

        const _Ty& min(const _Ty& _X, const _Ty& _Y)

        {return (_Y < _X ? _Y : _X); }

#endif

*/

// disable VC6.0 warning "identifier was truncated to 255 characters "

// this is routine whe compiling templates

// unfortunately, it does not seem to work in all cases

// and the messages are still visible when compiling STL



#ifdef _WIN32

#pragma warning(disable:4786)

#define M_PI 3.1415926535897932384626

#endif



#ifdef _WIN32

#define isnanf _isnan

#define fcos cos

#define fsin sin

#endif



inline void die() { 

#ifndef NDEBUG

cerr << "unexpected condition, aborting, " << __FILE__  << ":" << __LINE__

     << endl;

#endif

} 



typedef unsigned int uint;



#ifdef WIN32

#define drand48()   (1.0*rand()/RAND_MAX)

#define random   rand

#endif



#include <utility>



#if defined(_WIN32) && defined(_DEBUG) && defined(DEBUGNEW)

#define DNEW DEBUG_NEW

#else

#define DNEW new

#define PUSH_DEBUG_NEW_STATE 

#define POP_DEBUG_NEW_STATE 

#define ENABLE_DEBUG_NEW 

#define DISABLE_DEBUG_NEW 

#endif //DEBUGNEW



#define endlf endl








#ifndef _WIN32
#include <values.h>
#endif

#define MAX_SHORT_VALUE   MAXSHORT







#endif /* __COMPAT_H__ */






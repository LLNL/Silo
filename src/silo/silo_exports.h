#ifndef SILO_EXPORTS_H
#define SILO_EXPORTS_H

#ifndef SILO_API
  #ifdef _WIN32
    #ifdef SILO_STATIC_LIBRARY
      #define SILO_API
    #else
      #ifdef SILO_EXPORTS
        #define SILO_API __declspec(dllexport)
      #else
        #define SILO_API __declspec(dllimport)
      #endif
    #endif
  #else
    #define SILO_API
  #endif
#endif

#endif

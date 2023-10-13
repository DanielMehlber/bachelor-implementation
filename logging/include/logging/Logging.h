#ifndef LOGGING_EXPORT_MACRO_H
#define LOGGING_EXPORT_MACRO_H

#ifdef _WIN32
// For Windows (MSVC compiler)
#ifdef EXPORT_DLL

// When building the library
#define NETWORKING_API __declspec(dllexport)
#else

// When using the library
#define NETWORKING_API __declspec(dllimport)
#endif
#else
// For other platforms (Linux, macOS)
#define NETWORKING_API
#endif

#endif

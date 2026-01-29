// First Commit: 2013-11-05
//
// Authors:
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// Defines the `ISOMODEL_API` macro, which handles the `__declspec(dllexport)`
// and `__declspec(dllimport)` directives for building the code as a shared
// library (DLL) on Windows. It ensures that classes and functions are
// correctly exported and imported.

#ifndef __ISOMODEL_API_HPP__
#define __ISOMODEL_API_HPP__

#if _WIN32 || _MSC_VER

#ifdef openstudio_isomodel_EXPORTS
#define ISOMODEL_API __declspec(dllexport)
#else
// #define ISOMODEL_API __declspec(dllimport)
#define ISOMODEL_API
#endif
#else
#define ISOMODEL_API
#endif
#endif

/// @file ISOModelAPI.hpp
/// @brief DLL export/import macros for the ISOModel shared library.
///
/// Defines the ISOMODEL_API macro for Windows DLL builds. On non-Windows
/// platforms, the macro expands to nothing.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
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

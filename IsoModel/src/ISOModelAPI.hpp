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

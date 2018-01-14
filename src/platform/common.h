#ifndef IMG2SPEC_COMMON_H
#define IMG2SPEC_COMMON_H

#ifndef _stricmp
#define _stricmp(s1, s2) strcasecmp(s1, s2)
#endif

#ifdef _WIN32
#include "win32.h"
#else
#include "nonwin32.h"
#endif

#ifdef __APPLE__
extern "C" {
#endif

int getFileDate(char *aFilename);

const char *openDialog(const char *Title, const char *Filter);

const char *saveDialog(const char *Title, const char *Filter, const char *Ext);

#ifdef __APPLE__
}
#endif

#endif //IMG2SPEC_COMMON_H

#define NOC_FILE_DIALOG_IMPLEMENTATION
#include "noc_file_dialog.h"

#include <sys/types.h>
#include <sys/stat.h>

int getFileDate(char *aFilename) {
    struct stat result;
    if (stat(aFilename, &result) == 0)
#ifdef __APPLE__
        return (int)result.st_mtime;
#else
        return (int)result.st_mtim.tv_sec;
#endif
    else
        return 0;
}

const char *openDialog(const char *Title, const char *Filter) {
    return noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, Filter, NULL, NULL);
}

const char *saveDialog(const char *Title, const char *Filter, const char *Ext) {
    return noc_file_dialog_open(NOC_FILE_DIALOG_SAVE |
                                        NOC_FILE_DIALOG_OVERWRITE_CONFIRMATION,
                                Filter, NULL, NULL);
}

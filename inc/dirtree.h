#ifndef DIRTREE_H
#define DIRTREE_H

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <ftw.h>
#include <vector/vector.h>

struct fileinfo_t
{
    char flags[10];
    uid_t user;
    off_t size;
    time_t last_modification;
    mode_t st_mode;
    char *filename;
};

typedef struct fileinfo_t *fileinfo;

struct dirinfo_t
{
    vector files;
    vector dirs;
    char *dirname;
    struct dirinfo_t *parent;
};
typedef struct dirinfo_t *dirinfo;

fileinfo fileinfo_new(const char *name);

int fileinfo_free(fileinfo self);


dirinfo dirinfo_new(dirinfo parent, const char *name);

int dirinfo_sort(dirinfo self);

int dirinfo_free(dirinfo self);

int dirinfo_rapply(dirinfo self, int (*dirfunc)(dirinfo), int (*filefunc)(fileinfo));

#endif
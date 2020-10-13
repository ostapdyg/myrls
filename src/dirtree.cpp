#include <dirtree.h>

#include<string.h>


fileinfo fileinfo_new(const char *name)
{
    fileinfo self = (fileinfo)malloc(sizeof(fileinfo_t));

    char *local_name = (char *)malloc(sizeof(char) * strlen(name) + 1);
    strcpy(local_name, name);
    self->filename = local_name;
    return self;
}

int fileinfo_free(fileinfo self)
{
    free(self->filename);
    free(self);
    return 0;
}

dirinfo dirinfo_new(dirinfo parent, const char *name)
{
    // printf("    Create dir: %s\n", name);

    dirinfo self = (dirinfo)malloc(sizeof(dirinfo_t));

    self->files = vector_new(0);
    self->dirs = vector_new(0);
    self->parent = parent;

    char *local_name = (char *)malloc(sizeof(char) * strlen(name) + 1);
    strcpy(local_name, name);
    self->dirname = local_name;

    return self;
}


int __str_compare(const char *s1, const char *s2)
{

    while ((*s1 == *s2) && (*s1))
    {
        s1++;
        s2++;
    }
    int c1 = *s1;
    // c1 = c1 ^ ((!!isalpha(c1) & 1) << 5);
    int c2 = *s2;
    // c2 = c2 ^ ((!!isalpha(c2) & 1) << 5);
    // c1 += !!isalpha(c1)*(256

    // int c1 = (*s1);
    // int c2 = (*s2);
    if(isalpha(c1)){
        c1 = 256 + tolower(c1)*2 + !!isupper(c1);// All letters are bigger than special chars, and uppercase letters are bigger than respective lowercase
    }
    // c1 = tolower(c1) + (!!isalpha(c1))*(256 + tolower(c1) + !!isupper(c1))
    if(isalpha(c2)){
        c2 = tolower(c2)*2 + 256 + !!isupper(c2);// All letters are bigger than special chars, and uppercase letters are bigger than respective lowercase
    }

    if (c1 > c2)
    {
        return 1;
    }
    if (c1 < c2)
    {
        return -1;
    }
    return 0;
}

int __fileinfo_compare(const void *a, const void *b)
{
    char *s1 = (*((fileinfo *)a))->filename;
    char *s2 = (*((fileinfo *)b))->filename;
    return __str_compare(s1, s2);
}

int __dirinfo_compare(const void *a, const void *b)
{
    char *s1 = (*((dirinfo *)a))->dirname;
    char *s2 = (*((dirinfo *)b))->dirname;
    return __str_compare(s1, s2);
}

int dirinfo_sort(dirinfo self)
{
    qsort(self->files->data, vector_size(self->files), sizeof(fileinfo), (__fileinfo_compare));
    qsort(self->dirs->data, vector_size(self->dirs), sizeof(dirinfo), (__dirinfo_compare));
    return 0;
}

int dirinfo_rapply(dirinfo self, int (*dirfunc)(dirinfo), int (*filefunc)(fileinfo)){
    (*dirfunc)(self);
    
    fileinfo file;

    for (size_t i = 0; (file = (fileinfo)vector_get(self->files, i)); i++)
    {
        (*filefunc)(file);
    }

    dirinfo next_dir;
    for (size_t i = 0; (next_dir = (dirinfo)vector_get(self->dirs, i)); i++)
    {
        dirinfo_rapply(next_dir, dirfunc, filefunc);
    }

    return 0;
}


int dirinfo_free(dirinfo self)
{
    dirinfo dir;
    for (size_t i = 0; (dir = (dirinfo)vector_get(self->dirs, i)); i++)
    {
        dirinfo_free(dir);
    }
    free(self->dirname);

    fileinfo file;
    for (size_t i = 0; (file = (fileinfo)vector_get(self->files, i)); i++)
    {
        fileinfo_free(file);
    }
    vector_free(self->dirs);
    vector_free(self->files);

    free(self);
}

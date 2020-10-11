#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <ftw.h>

#include <vector/vector.h>


#include <stdio.h>
#include <string.h>

static int ERRNO_buf;

struct fileinfo_t
{
    char flags[10];
    uid_t user;
    off_t size;
    mode_t st_mode;
    char *filename;
};
typedef struct fileinfo_t *fileinfo;

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

struct dirinfo_t
{
    vector files;
    vector dirs;
    char *dirname;
    struct dirinfo_t *parent;
};
typedef struct dirinfo_t *dirinfo;

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


int str_compare(const char *s1, const char *s2)
{
    // for (; (*s1 == *s2) && (*s1); s1++, s2++)
    // {
    // }
    // return strcoll(s1, s2);

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
        c1 = tolower(c1)*2 + 256 + !!isupper(c1);// All letters are bigger than special chars, and uppercase letters are bigger than respective lowercase
    }
    if(isalpha(c2)){
        c2 = tolower(c2)*2 + 256 + !!isupper(c2);// All letters are bigger than special chars, and uppercase letters are bigger than respective lowercase
    }

    //ASCII: ... 90: Z; 91: [; 92: \; 93: ]; 94: ^; 95: _; 96: `; 97: a ...
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

int fileinfo_compare(const void *a, const void *b)
{
    char *s1 = (*((fileinfo *)a))->filename;
    char *s2 = (*((fileinfo *)b))->filename;
    return str_compare(s1, s2);
}

int dirinfo_compare(const void *a, const void *b)
{
    char *s1 = (*((dirinfo *)a))->dirname;
    char *s2 = (*((dirinfo *)b))->dirname;
    return str_compare(s1, s2);
}

int dirinfo_sort(dirinfo self)
{
    qsort(self->files->data, vector_size(self->files), sizeof(fileinfo), (fileinfo_compare));
    qsort(self->dirs->data, vector_size(self->dirs), sizeof(dirinfo), (dirinfo_compare));
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

static dirinfo current_dir;

static int __iterfunc(const char *pathname, const struct stat *statbuf, int flag, FTW *info)
{
    static int depth;

    int cur_depth = info->level;

    // const char* p =pathname + info->base;

    printf("%d: %s\n", cur_depth, pathname);

    while (cur_depth < depth) //Exit dir
    {

        // printf("    Exiting dir: %d\n", depth);
        current_dir = current_dir->parent;
        depth--;
        // if(!current_dir){
        //     return 1;
        // }
    }

    // TODO: Fill f_info
    fileinfo f_info = fileinfo_new(pathname + info->base);

    vector_push(current_dir->files, f_info);

   if (flag == FTW_D) //Enter dir
    {

        dirinfo next_dir = dirinfo_new(current_dir, pathname);

        // printf("    Entering dir:%d\n", depth+1);
        
        vector_push(current_dir->dirs, next_dir);

        current_dir = next_dir;
        depth++;
    }

    return 0;
}

int recursive_print(dirinfo dir)
{
    printf("%s:\n", dir->dirname);

    dirinfo_sort(dir);
    fileinfo file;
    for (size_t i = 0; (file = (fileinfo)vector_get(dir->files, i)); i++)
    {
        printf("    %s\n", file->filename);
    }

    dirinfo next_dir;
    for (size_t i = 0; (next_dir = (dirinfo)vector_get(dir->dirs, i)); i++)
    {
        printf("\n");
        recursive_print(next_dir);
    }

    return 0;
}

// Done
int cleanup(dirinfo root)
{
    dirinfo_free(root);
    return 0;
}

int myrls(char *dirname)
{

    dirinfo root = dirinfo_new(NULL, dirname);
    current_dir = root;

    nftw(dirname, (__iterfunc), 10, 0);
    printf("nftw done\n");

    // for(auto file : root->dirs[0]->dirs[0]->files){
    //     printf("%s\n", file->filename);
    // }

    recursive_print(root);

    cleanup(root);
    return 0;
}

#include <stdio.h>

static const char *help_str =
    "Usage: rls [-h|--help] <path> \n\
Recursively list files.\n\
-h, --help      show this message and exit\n";

int main(int argc, char **argv)
{
    int opt;
    int h_f = 0;

    struct option long_opts[] =
        {{"help", no_argument, 0, 'h'},
         {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, ":hA", long_opts, 0)) != -1)
    {
        switch (opt)
        {
        case 'h':
            h_f = 1;
            break;
        case '?':
        default:
            printf("Unrecognized command line option: %s\n %s", argv[optind - 1], help_str);
            return -1;
        }
    }
    if (h_f)
    {
        printf(help_str);
        return 0;
    }

    // for (; optind < argc; optind++)
    // {

    //     filenames[filenum] = argv[optind];
    //     filenum += 1;
    // }
    // filenames[filenum] = 0;

    int err;
    if ((err = myrls(argv[1])))
    {
        printf("myrls: Non-zero return code: %d\n", err);
        return err;
    }
    // free(filenames);
    return 0;
}

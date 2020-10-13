#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <ftw.h>

#include <pwd.h>
#include <time.h>

#include <vector/vector.h>
#include <dirtree.h>

#include <stdio.h>
#include <string.h>

static int ERRNO_buf;

static dirinfo current_dir;

static size_t username_maxlen;
static size_t size_maxlen;

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
    f_info->last_modification = statbuf->st_mtim.tv_sec;
    f_info->user = statbuf->st_uid;
    f_info->size = statbuf->st_size;
    
    // Update maxlens
    struct passwd *pwd = getpwuid(f_info->user);
    size_t namelen = strlen(pwd->pw_name);
    username_maxlen = username_maxlen*(username_maxlen >= namelen) + namelen*(username_maxlen < namelen);
    char sizebuf[255];
    sprintf(sizebuf, "%zu", f_info->size);
    size_t sizelen = strlen(sizebuf);
    size_maxlen = size_maxlen*(size_maxlen >= sizelen) + sizelen*(size_maxlen < sizelen);
    // size_len = log

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


int printdir(dirinfo dir){
    dirinfo_sort(dir);
    printf("\n%s:\n", dir->dirname);
}

int printfile(fileinfo file)
{
    struct passwd *pwd = getpwuid(file->user);
    // size_t namelen = strlen(pwd->pw_name);
    printf("    %5s %*d ", username_maxlen, pwd->pw_name, size_maxlen, file->size);
    char buf[20];
    strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&(file->last_modification)));
    printf(buf);
    printf("  %s\n", file->filename);
    
}

int recursive_print(dirinfo dir)
{
    dirinfo_rapply(dir, printdir, printfile);
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

#include <sys/stat.h>
#include <sys/types.h>
#include <error.h>

#include <pwd.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include <vector/vector.h>
#include <dirtree.h>

#include <stdio.h>
#include <string.h>

#define TIMESPAMP_SIZE 20

static int ERRNO_buf;

static dirinfo current_dir;

static size_t username_maxlen;
static size_t size_maxlen;

static int __iterfunc(const char *pathname, const struct stat *statbuf, int flag, FTW *info)
{

    static int depth;

    int cur_depth = info->level;

    while (cur_depth < depth) //Exit dir
    {

        current_dir = current_dir->parent;
        depth--;
    }

    if (errno != 0)
    {
        error(0, errno, "Cannot access file%s", pathname);
        ERRNO_buf = 3;
        errno = 0;

        return 0;
    }
    errno = 0;

    fileinfo f_info = fileinfo_new(pathname, info->base);
    f_info->last_modification = statbuf->st_mtim.tv_sec;
    f_info->user = statbuf->st_uid;
    f_info->size = statbuf->st_size;
    f_info->st_mode = statbuf->st_mode;

    static const char *perms[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

    strcpy(f_info->flags, perms[(statbuf->st_mode & S_IRWXU) >> 6]);
    strcpy(f_info->flags + 3, perms[(statbuf->st_mode & S_IRWXG) >> 3]);
    strcpy(f_info->flags + 6, perms[(statbuf->st_mode & S_IRWXO)]);

    f_info->flags[9] = '\0';

    // Update maxlens
    struct passwd *pwd = getpwuid(f_info->user);
    size_t namelen = strlen(pwd->pw_name);
    username_maxlen = username_maxlen * (username_maxlen >= namelen) + namelen * (username_maxlen < namelen);
    //used to determine size
    char sizebuf[20];
    sprintf(sizebuf, "%zu", f_info->size);
    size_t sizelen = strlen(sizebuf);
    //note: this is not the fastest implementation, use for(int i=1; f_info->size /= 10; i++); or (int) log10(num)+1 for better performance

    size_maxlen = size_maxlen * (size_maxlen >= sizelen) + sizelen * (size_maxlen < sizelen);

    //Add file to the files vector 
    vector_push(current_dir->files, f_info);

    if (flag == FTW_D) //Enter dir
    {

        dirinfo next_dir = dirinfo_new(current_dir, pathname);

        vector_push(current_dir->dirs, next_dir);

        current_dir = next_dir;
        depth++;
    }

    return 0;
}

int printdir(dirinfo dir)
{
    dirinfo_sort(dir);
    printf("\n%s:\n", dir->dirname);
    return 0;
}

int printfile(fileinfo file)
{
    struct passwd *pwd = getpwuid(file->user);

    printf("%9s %*s %*zu ", file->flags, (int)username_maxlen, pwd->pw_name, (int)size_maxlen, file->size);
    
    char buf[TIMESPAMP_SIZE];
    strftime(buf, TIMESPAMP_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&(file->last_modification)));
    printf(buf);
    const char *prefix = "";
    const char *suffix = "";
    size_t l;
    char filename_buf[PATH_MAX + 3];

    switch (file->st_mode & S_IFMT)
    {
    case S_IFDIR:
        prefix = "/";
        break;

    case S_IFIFO:
        prefix = "|";
        break;

    case S_IFLNK:
    {
        prefix = "@";

        filename_buf[0] = '-';
        filename_buf[1] = '>';

        l = readlink(file->filepath, filename_buf + 2, PATH_MAX);
        filename_buf[l + 2] = '\0';
        suffix = filename_buf;
        break;
    }
    case S_IFSOCK:
        prefix = "=";
        break;

    case S_IFREG:

        if (file->st_mode & (S_IXGRP | S_IXUSR | S_IXOTH))
        {
            prefix = "*";
        }

        break;

    default:
        prefix = "?";
        break;
    }

    printf("  %s%s%s\n", prefix, file->filename, suffix);
    return 0;
}

int recursive_print(dirinfo dir)
{
    dirinfo_rapply(dir, printdir, printfile);
    return 0;
}

int cleanup(dirinfo root)
{
    dirinfo_free(root);
    return 0;
}

int myrls(const char *dirname)
{
    if (access(dirname, F_OK))
    {
        error(0, errno, "cannot access '%s'", dirname);
        return 1;
    }

    dirinfo root = dirinfo_new(NULL, "");
    current_dir = root;

    int err = nftw(dirname, (__iterfunc), 10, FTW_PHYS);
    if (err)
    {
        error(0, errno, "nftw error");
        return 1;
    }

    if (vector_size(root->dirs))
    {
        recursive_print((dirinfo)vector_get(root->dirs, 0));
    }
    else if (vector_size(root->files))
    {
        printfile((fileinfo)vector_get(root->files, 0));
    }

    cleanup(root);
    return 0;
}

static const char *help_str =
    "Usage: rls [-h|--help] <path> \n\
Recursively list files.\n\
-h, --help      show this message and exit\n";

int main(int argc, char **argv)
{
    int opt;
    int h_f = 0;
    ERRNO_buf = 0;

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
            fprintf(stderr, "Unrecognized command line option: %s\n %s", argv[optind - 1], help_str);
            return 2;
        }
    }
    if (h_f)
    {
        printf(help_str);
        return 0;
    }

    int err;
    switch (argc)
    {
    case 2:
        if ((err = myrls(argv[1])))
        {

            fprintf(stderr, "myrls: Non-zero return code: %d\n", err);
            return err;
        }
        break;
    case 1:
        if ((err = myrls(".")))
        {
            fprintf(stderr, "myrls: Non-zero return code: %d\n", err);
            return err;
        }
        break;
    default:
        fprintf(stderr, "myrls: wrong number of arguments\n");
        printf("%s", help_str);
        return 2;
    }

    return ERRNO_buf;
}

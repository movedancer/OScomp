#define FUSE_USE_VERSION 31

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <fuse.h>

#ifdef HAVE_LIBULOCKMGR
#include <ulockmgr.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include <sys/file.h> /* flock(2) */

static void *xmp_init(struct fuse_conn_info *conn,
                      struct fuse_config *cfg)
{
        (void) conn;
        cfg->use_ino = 1;
        cfg->nullpath_ok = 1;
        cfg->entry_timeout = 0;
        cfg->attr_timeout = 0;
        cfg->negative_timeout = 0;

        return NULL;
}

static int xmp_getattr(const char *path, struct stat *stbuf,
                        struct fuse_file_info *fi)
{
        int res;

        (void) path;

        if(fi)
                res = fstat(fi->fh, stbuf);
        else
                res = lstat(path, stbuf);
        if (res == -1)
                return -errno;

        return 0;
}

struct xmp_dirp {
        DIR *dp;
        struct dirent *entry;
        off_t offset;
};

static int xmp_opendir(const char *path, struct fuse_file_info *fi)
{
        int res;
        struct xmp_dirp *d = malloc(sizeof(struct xmp_dirp));
        if (d == NULL)
                return -ENOMEM;

        d->dp = opendir(path);
        if (d->dp == NULL) {
                res = -errno;
                free(d);
                return res;
        }
        d->offset = 0;
        d->entry = NULL;

        fi->fh = (unsigned long) d;
        return 0;
}

static inline struct xmp_dirp *get_dirp(struct fuse_file_info *fi)
{
        return (struct xmp_dirp *) (uintptr_t) fi->fh;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags)
{
        struct xmp_dirp *d = get_dirp(fi);

        (void) path;
        if (offset != d->offset) {
#ifndef __FreeBSD__
                seekdir(d->dp, offset);
#else
                seekdir(d->dp, offset-1);
#endif
                d->entry = NULL;
                d->offset = offset;
        }
        while (1) {
                struct stat st;
                off_t nextoff;
                enum fuse_fill_dir_flags fill_flags = 0;

                if (!d->entry) {
                        d->entry = readdir(d->dp);
                        if (!d->entry)
                                break;
                }
#ifdef HAVE_FSTATAT
                if (flags & FUSE_READDIR_PLUS) {
                        int res;

                        res = fstatat(dirfd(d->dp), d->entry->d_name, &st,
                                      AT_SYMLINK_NOFOLLOW);
                        if (res != -1)
                                fill_flags |= FUSE_FILL_DIR_PLUS;
                }
#endif
                if (!(fill_flags & FUSE_FILL_DIR_PLUS)) {
                        memset(&st, 0, sizeof(st));
                        st.st_ino = d->entry->d_ino;
                        st.st_mode = d->entry->d_type << 12;
                }
                nextoff = telldir(d->dp);
#ifdef __FreeBSD__
                nextoff++;
#endif
                if (filler(buf, d->entry->d_name, &st, nextoff, fill_flags))
                        break;

                d->entry = NULL;
                d->offset = nextoff;
        }

        return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
        int res;

        res = mkdir(path, mode);
        if (res == -1)
                return -errno;

        return 0;
}


static int xmp_open(const char *path, struct fuse_file_info *fi)
{
        int fd;

        fd = open(path, fi->flags);
        if (fd == -1)
                return -errno;

        fi->fh = fd;
        return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
        int res;

        (void) path;
        res = pread(fi->fh, buf, size, offset);
        if (res == -1)
                res = -errno;

        return res;
}

static int xmp_read_buf(const char *path, struct fuse_bufvec **bufp,
                        size_t size, off_t offset, struct fuse_file_info *fi)
{
        struct fuse_bufvec *src;

        (void) path;

        src = malloc(sizeof(struct fuse_bufvec));
        if (src == NULL)
                return -ENOMEM;

        *src = FUSE_BUFVEC_INIT(size);

        src->buf[0].flags = FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK;
        src->buf[0].fd = fi->fh;
        src->buf[0].pos = offset;

        *bufp = src;

        return 0;
}
static int xmp_rename(const char *from, const char *to, unsigned int flags)
{
        int res;

        // When we have renameat2() in libc, then we can implement flags 
        if (flags)
                return -EINVAL;

        res = rename(from, to);
        if (res == -1)
                return -errno;

        return 0;
}
static const struct fuse_operations xmp_oper = {
        .init           = xmp_init,
        .getattr        = xmp_getattr,
        .opendir        = xmp_opendir,
        .readdir        = xmp_readdir,
        .mkdir          = xmp_mkdir,
        .open           = xmp_open,
        .read           = xmp_read,
//      .rename         = xmp_rename,
};


int main(int argc, char *argv[])
{
        umask(0);
        return fuse_main(argc, argv, &xmp_oper, NULL);
}
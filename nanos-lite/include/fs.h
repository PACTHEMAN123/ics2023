#ifndef __FS_H__
#define __FS_H__

#include <common.h>

size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);

size_t get_ramdisk_size();

int fs_open(const char *pathname, int flags, int mode);

size_t fs_write(int fd, const void *buf, size_t count);

size_t fs_read(int fd, void *buf, size_t len);

size_t fs_lseek(int fd, size_t offset, int whence);

int fs_close(int fd);

size_t serial_write(const void *buf, size_t offset, size_t len);

size_t events_read(void *buf, size_t offset, size_t len); 

size_t dispinfo_read(void *buf, size_t offset, size_t len);

size_t fb_write(const void *buf, size_t offset, size_t len);

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

#endif

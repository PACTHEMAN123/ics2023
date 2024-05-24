#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *, size_t, size_t);
size_t ramdisk_write(const void *, size_t, size_t);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0,  invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode) {
  for(int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    if(strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
Log("open %s", file_table[i].name);
      return i;
    }
  }
  return -1;
}

int fs_close(int fd) {
Log("close %s", file_table[fd].name);

  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
Log("read %s", file_table[fd].name);
  size_t tmp = file_table[fd].open_offset;
  file_table[fd].open_offset += len;
  //assert(file_table[fd].open_offset <= file_table[fd].size);
  return ramdisk_read(buf, file_table[fd].disk_offset + tmp, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
 Log("write %s", file_table[fd].name);
 size_t tmp = file_table[fd].open_offset;
  file_table[fd].open_offset += len;
  //assert(file_table[fd].open_offset <= file_table[fd].size);
  return ramdisk_write(buf, file_table[fd].disk_offset + tmp, len);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
 // Log("%d", fd);
  size_t ret = 0;
  switch(whence) {
    case SEEK_SET: ret = offset; break;
    case SEEK_CUR: ret = file_table[fd].open_offset + offset; break;
    case SEEK_END: ret = file_table[fd].size + offset; break;
    default: assert(0);
  }
  file_table[fd].open_offset = ret;
  assert(ret <= file_table[fd].size);
  Log("%s offset: %d", file_table[fd].name, file_table[fd].open_offset);
  return ret; 
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
}

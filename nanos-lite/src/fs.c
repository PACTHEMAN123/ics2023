#include <fs.h>

#define min(x,y) ((x < y) ? x : y)
#define NR_FILE (sizeof(file_table) / sizeof(file_table[0]))

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

size_t file_read(void *buf, size_t offset, size_t len) {
  return ramdisk_read(buf, offset, len);
}

size_t file_write(const void *buf, size_t offset, size_t len) {
  return ramdisk_write(buf, offset, len);
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0,  invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  {"/dev/fb", 0, 0, 0, NULL, NULL},
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
//Log("close %s", file_table[fd].name);
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len) {
//Log("read %s from %d to %d", file_table[fd].name, file_table[fd].open_offset, file_table[fd].open_offset + len);
  if(fd < 5) return file_table[fd].read(buf, 0, len);
  size_t tmp = file_table[fd].open_offset;
  size_t count = min(len, (file_table[fd].size - file_table[fd].open_offset));
  file_table[fd].open_offset += count;
  //assert(file_table[fd].open_offset <= file_table[fd].size);
  return file_table[fd].read(buf, file_table[fd].disk_offset + tmp, count);
}

size_t fs_write(int fd, const void *buf, size_t len) {
// Log("write %s", file_table[fd].name);
  if(fd < 5) return file_table[fd].write(buf, 0, len);
  size_t tmp = file_table[fd].open_offset; 
  size_t count = min(len, (file_table[fd].size - file_table[fd].open_offset));
  file_table[fd].open_offset += count;
  //assert(file_table[fd].open_offset <= file_table[fd].size);
  return file_table[fd].write(buf, file_table[fd].disk_offset + tmp, count);
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
  if(fd < 5) ret = 0;
  assert(ret <= file_table[fd].size);
  //Log("%s offset: %d", file_table[fd].name, file_table[fd].open_offset);
  return ret; 
}


void init_fs() {
  
  for(int i = 5; i < NR_FILE; i++) {
    if(file_table[i].write == NULL) file_table[i].write = file_write;
    if(file_table[i].read == NULL) file_table[i].read = file_read;
  } 
  // TODO: initialize the size of /dev/fb
  int fd = fs_open("/dev/fb", 0, 0);
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  file_table[fd].size = w * h;

 
}

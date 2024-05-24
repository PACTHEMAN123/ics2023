#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {

  int fd = fs_open(filename, 0, 0);
printf("%d", fd);
  Elf_Ehdr ehdr;
  fs_read(fd, (void *)(&ehdr), sizeof(ehdr));

  char magic[16] = { 0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0,0,0,0,0,0,0,0,0};
  assert(strcmp((char *)ehdr.e_ident, magic) == 0);
  Elf_Phdr phdr;

  
  for(int i = 0; i < ehdr.e_phnum; i++) {
    fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
    fs_read(fd, (void *)(&phdr), sizeof(phdr));
    if(phdr.p_type != PT_LOAD)continue;
    //printf("filesz:%d memsz:%d vaddr:%p offset:%d", phdr.p_filesz, phdr.p_memsz, phdr.p_vaddr, phdr.p_offset);
    fs_lseek(fd, phdr.p_offset, SEEK_SET);
    fs_read(fd, (void *)(phdr.p_vaddr), phdr.p_filesz);
    memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
  } 

  fs_close(fd);

  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


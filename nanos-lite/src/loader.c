#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *, size_t, size_t);

static uintptr_t loader(PCB *pcb, const char *filename) {

  Elf_Ehdr ehdr;
  ramdisk_read((void *)(&ehdr), 0, sizeof(ehdr));

  Elf_Phdr phdr;

  for(int i = 0; i < ehdr.e_phnum; i++) {
    ramdisk_read((void *)(&phdr), ehdr.e_phoff + i * ehdr.e_phentsize, sizeof(phdr));
    if(phdr.p_type != PT_LOAD)continue;
    printf("filesz:%d memsz:%d vaddr:%p offset:%d", phdr.p_filesz, phdr.p_memsz, phdr.p_vaddr, phdr.p_offset);
    ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_filesz); 
    memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
  } 
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


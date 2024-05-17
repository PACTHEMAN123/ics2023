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
  ramdisk_read((void *)(&phdr), ehdr.e_phoff, sizeof(phdr));

  ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_filesz);

  memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz); 
   
  return phdr.p_vaddr;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


static int load_icode(int fd, int argc, char **kargv) { // load_icode from disk fd, For LAB4
    #ifdef LAB4_EX2
    if(current->mm != NULL)//检查进程的内存管理器是否清空
        panic("load_icode: current->mm must be empty.\n");
    int ret = -E_NO_MEM;
    struct mm_struct *mm;
    if((mm = mm_create()) == NULL)//建立内存管理器
        goto bad_mm;
    if(setup_pgdir(mm) != 0)//设置内存管理器的页目录表
     goto bad_pgdir_cleanup_mm;
    struct __elfhdr ___elfhdr__;
    struct elfhdr32 __elf, *elf = &__elf;
    if((ret = load_icode_read(fd, &___elfhdr__, sizeof(struct __elfhdr), 0)) != 0)
        goto bad_elf_cleanup_pgdir;
  
    _load_elfhdr((unsigned char*)&___elfhdr__, &__elf);
    if(elf->e_magic != ELF_MAGIC) {
        ret = -E_INVAL_ELF;
        goto bad_elf_cleanup_pgdir;
    }
    struct proghdr _ph, *ph = &_ph;
    uint32_t vm_flags, phnum;
    uint32_t perm = 0;
    struct Page *page;
    for (phnum = 0; phnum < elf->e_phnum; phnum ++) {
        off_t phoff = elf->e_phoff + sizeof(struct proghdr) * phnum;
        if((ret = load_icode_read(fd, ph, sizeof(struct proghdr), phoff)) != 0)
            goto bad_cleanup_mmap;
        if(ph->p_type != ELF_PT_LOAD)
            continue ;
        if(ph->p_filesz > ph->p_memsz){
            ret = -E_INVAL_ELF;
            goto bad_cleanup_mmap;
        }
      
        vm_flags = 0;
        perm |= PTE_U;
        if(ph->p_flags & ELF_PF_X)
            vm_flags |= VM_EXEC;
        if(ph->p_flags & ELF_PF_W)
            vm_flags |= VM_WRITE;
        if(ph->p_flags & ELF_PF_R)
            vm_flags |= VM_READ;
        if(vm_flags & VM_WRITE)
            perm |= PTE_W; 
        if((ret = mm_map(mm, ph->p_va, ph->p_memsz, vm_flags, NULL)) != 0)
            goto bad_cleanup_mmap;
        off_t offset = ph->p_offset;
        size_t off, size;
        uintptr_t start = ph->p_va, end, la = ROUNDDOWN_2N(start, PGSHIFT);
        end = ph->p_va + ph->p_filesz;
        while (start < end) {
            if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if(end < la)
                size -= la - end;
            if((ret = load_icode_read(fd, page2kva(page) + off, size, offset)) != 0)
                goto bad_cleanup_mmap;
            fence_i(page2kva(page)+off, size);
            start += size, offset += size;
        }
        end = ph->p_va + ph->p_memsz;
        if(start < la) {
            if(start >= end)
                continue ;
            off = start + PGSIZE - la, size = PGSIZE - off;
            if(end < la)
                size -= la - end;
            memset(page2kva(page) + off, 0, size);
            fence_i(page2kva(page) + off, size);
            start += size;
            assert((end < la && start == end) || (end >= la && start == la));
        }
        while (start < end) {
            if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL){
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if(end < la)
                size -= la - end;
            memset(page2kva(page) + off, 0, size);
            fence_i(page2kva(page) + off, size);
            start += size;
        }
    }
    sysfile_close(fd);
    vm_flags = VM_READ | VM_WRITE | VM_STACK;//设置用户栈的权限,将用户栈所在的虚拟内存区域设置为合法
    if((ret = mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags, NULL)) != 0)
        goto bad_cleanup_mmap;
    mm_count_inc(mm);//切换到用户进程空间
    current->mm = mm;
    current->cr3 = PADDR(mm->pgdir);
    lcr3(PADDR(mm->pgdir));
    uintptr_t stacktop = USTACKTOP - argc * PGSIZE;
    char **uargv = (char **)(stacktop - argc * sizeof(char *));
    for (int i = 0; i < argc; i ++)
        uargv[i] = strcpy((char *)(stacktop + i * PGSIZE), kargv[i]);
    struct trapframe *tf = current->tf;
    memset(tf, 0, sizeof(struct trapframe));
    tf->tf_era = elf->e_entry;
    tf->tf_regs.reg_r[LOONGARCH_REG_SP] = USTACKTOP;
    uint32_t status = 0;
    status |= PLV_USER;
    status |= CSR_CRMD_IE;
    tf->tf_prmd = status;
    tf->tf_regs.reg_r[LOONGARCH_REG_A0] = argc;
    tf->tf_regs.reg_r[LOONGARCH_REG_A1] = (uint32_t)uargv;
    ret = 0;//函数正常退出
    out:
        return ret;
    bad_cleanup_mmap:
        panic("bad_cleanup_mmap");
        exit_mmap(mm);
    bad_elf_cleanup_pgdir:
        panic("bad_elf_cleanup_pgdir");
        put_pgdir(mm);
    bad_pgdir_cleanup_mm:
        panic("bad_pgdir_cleanup_mm");
        mm_destroy(mm);
    bad_mm:
        panic("bad_mm");
        goto out;
#endif
}
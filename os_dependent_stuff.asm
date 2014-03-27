; syscalls
%ifidn __OUTPUT_FORMAT__,elf64
; http://lxr.linux.no/linux+v3.13.5/arch/x86/syscalls/syscall_64.tbl
  %define SYSCALL_OPEN    2
  %define SYSCALL_WRITE   1
  %define SYSCALL_MMAP    9
  %define SYSCALL_MUNMAP  11
  %define SYSCALL_PWRITE  18
  %define SYSCALL_FORK    57
  %define SYSCALL_WAITID  247
  %define SYSCALL_EXIT    60
%elifidn __OUTPUT_FORMAT__,macho64
; http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/kern/syscalls.master
  %define SYSCALL_OPEN    0x2000005
  %define SYSCALL_WRITE   0x2000004
  %define SYSCALL_MMAP    0x20000C5
  %define SYSCALL_MUNMAP  0x2000049
  %define SYSCALL_PWRITE  0x200009A
  %define SYSCALL_FORK    0x2000002
  %define SYSCALL_WAITID  0x20000AD
  %define SYSCALL_EXIT    0x2000001
%endif

; fcntls
%ifidn __OUTPUT_FORMAT__,elf64
; http://lxr.linux.no/linux+v3.13.5/include/uapi/asm-generic/fcntl.h
  %define O_RDONLY      0o0000000
  %define O_WRONLY      0o0000001
  %define O_RDWR        0o0000002
  %define O_CREAT       0o0000100
  %define O_EXCL        0o0000200
  %define O_NOCTTY      0o0000400
  %define O_TRUNC       0o0001000
  %define O_APPEND      0o0002000
  %define O_NONBLOCK    0o0004000
  %define O_DSYNC       0o0010000
  %define FASYNC        0o0020000
  %define O_DIRECT      0o0040000
  %define O_LARGEFILE   0o0100000
  %define O_DIRECTORY   0o0200000
  %define O_NOFOLLOW    0o0400000
  %define O_NOATIME     0o1000000
  %define O_CLOEXEC     0o2000000
  %define O_SYNC        0o4000000
  %define O_PATH        0o10000000
%elifidn __OUTPUT_FORMAT__,macho64
; http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/sys/fcntl.h
  %define O_RDONLY      0x0000
  %define O_WRONLY      0x0001
  %define O_RDWR        0x0002
  %define O_NONBLOCK    0x0004
  %define O_APPEND      0x0008
  %define O_SHLOCK      0x0010
  %define O_EXLOCK      0x0020
  %define O_ASYNC       0x0040
  %define O_SYNC        0x0080
  %define O_NOFOLOW     0x0100
  %define O_CREAT       0x0200
  %define O_TRUNC       0x0400
  %define O_EXCL        0x0800
  %define O_NOCTTY      0x20000
  %define O_DIRECTORY   0x100000
  %define O_SYMLINK     0x200000
  %define O_DSYNC       0x400000
%endif

; mmap() and mprotect() flags
%ifidn __OUTPUT_FORMAT__,elf64
; http://lxr.linux.no/linux+v3.13.5/include/uapi/asm-generic/mman-common.h
  %define MAP_SHARED   0x01
  %define MAP_PRIVATE  0x02
  %define MAP_FIXED    0x10
  %define MAP_ANON     0x20
  %define PROT_NONE     0b0000
  %define PROT_READ     0b0001
  %define PROT_WRITE    0b0010
  %define PROT_EXEC     0b0100
  %define PROT_SEM      0b1000
  %define PROT_GROWSDOWN 0x01000000
  %define PROT_GROWSUP   0x02000000
%elifidn __OUTPUT_FORMAT__,macho64
; http://www.opensource.apple.com/source/xnu/xnu-2050.18.24/bsd/sys/mman.h
  %define MAP_SHARED    0b000000000001
  %define MAP_PRIVATE   0b000000000010
  %define MAP_FIXED     0b000000010000
  %define MAP_RENAME    0b000000100000
  %define MAP_NORESERVE 0b000001000000
  %define MAP_INHERIT   0b000010000000
  %define MAP_NOEXTEND  0b000100000000
  %define MAP_SEMAPHORE 0b001000000000
  %define MAP_NOCACHE   0b010000000000
  %define MAP_JIT       0b100000000000
  %define MAP_FILE      0x0000
  %define MAP_ANON      0x1000
  %define PROT_NONE     0b000
  %define PROT_READ     0b001
  %define PROT_WRITE    0b010
  %define PROT_EXEC     0b100
%endif

; wait() flags are shared
; http://lxr.linux.no/linux+v3.13.5/include/uapi/linux/wait.h
%define WEXITED      0x04
%define P_ALL        0
%define P_PID        1
%define P_PGID       2
%define ECHILD       10

default rel
section .text

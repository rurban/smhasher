%include "os_dependent_stuff.asm"
bits 64

%macro hash_function 5
; clobbers rcx and all arguments
; %1 = key pointer
; %2 = key length
; %3 = desired return register
; %4 & 5 are scratch

  xor %3, %3

%%begin:
  cmp %2, 8
  jl %%last_bits
  crc32 %3, qword[%1]
  add %1, 8
  sub %2, 8
  jnz %%begin

  jmp %%ret

%%last_bits:
  ; zero out the higher bits of the last partial byte
  ; r11 holds the last byte
  mov %4, [%1]                  

  ; shift operator can only use cl, so put the number of bits remaining into rcx
  mov rcx, %2                    
  shl rcx, 3                      ; multiply by 8 to get bits

  ; rdx will hold the desired mask
  mov %5, 1
  shl %5, cl
  dec %5
  and %4, %5
  crc32 %3, %4
  
%%ret:

%endmacro


global fhtw_new
fhtw_new:
  push rdi ; store requested size for later

; rdi is the size of the the table
; we need 3 pieces of metadata - occupancy, capacity, size of info word in bits
; and 3 arrays - keys, values, and hop info words
; info word size does not change the amount of memory allocated; just says how many bits we use 
  inc rdi
  mov rax, 24
  mul rdi

; load arguments to mmap
  mov rsi, rax                      ; size 
  mov r10, MAP_SHARED | MAP_ANON    ; not backed by a file
  mov r8, -1                        ; file descripter is -1
  mov r9, 0                         ; no offset
  mov rax, SYSCALL_MMAP
  mov rdx, PROT_READ | PROT_WRITE   ; read/write access
  mov rdi, 0                        ; we don't care about the particular address
  
  syscall
  test rax, rax
  ;js .error ; local error label
  
; initialize metadata
  ; occupancy at [rax] has already been set to 0 by mmap
  pop r11
  mov [rax + 8], r11                ; store capacity
  dec r11
  bsr r11, r11                           
  mov [rax + 16], r11               ; store size of info word - logarithmic in capacity
  
  ret
  

global fhtw_free
fhtw_free:
  ; put size of memory to be freed in rsi
  mov r11, [rdi + 8]
  inc r11
  mov rax, 24
  mul r11

  ; arguments to munmap
  ; rdi already set to address
  mov rsi, rax
  mov rax, SYSCALL_MUNMAP

  syscall
  ret


global fhtw_set
fhtw_set:
  ; rdi = table pointer
  ; rsi = key pointer
  ; rdx = key length
  ; rcx = value
  ; r8 = value length? -- used in function!

  ; make sure there is room in the table
  mov r11, [rdi]
  cmp r11, qword[rdi + 8]
  je .table_full

  ; calculate hash
  mov r8, rsi                              ; save key pointer in r8
  mov r9, rcx                              ; save value in r9
  hash_function rsi, rdx, rax, r11, r10

  ; linear probe for empty space
  div qword[rdi + 8]                             ; hash value is in rax, we divide by table size.  

  shl rdx, 3                                ; get index in bits
  add rdx, rdi      
  add rdx, 24                               ; add rdi + 24 to get start of key array

  .begin:
    cmp qword[rdx], 0
    je .end
    add rdx, 8
    jmp .begin
  .end:
     
  ; empty space is in rdx
  ; if empty space is within length of bitmap, insert
  
  ; STUB just insert into empty space - linear probe table
  mov [rdx], r8                             ; insert key
  mov rax, [rdi + 8]                        ; next 3 lines calculate value position
  shl rax, 3
  add rax, rdx
  mov [rax], r9                             ; insert value
  

  ; if space is too far away, hop the space back until it is close enough

  ; when it's close enough, jump to insert

  ; increase occupancy
  
  ret


.table_full:
  ; return error code
  mov rax, -1
  ret


global fhtw_get
fhtw_get:
  ; table in rdi
  ; key in rsi
  ; keylen in rdx

; STUB linear probing

  mov r8, rsi
  mov r9, rdx
  hash_function rsi, rdx, rax, r10, r11
  mov r10, rdi
  
  div qword[r10 + 8]                             ; hash value is in rax, we divide by table size.  

  ; get pointer in the key array into rdx
  shl rdx, 3                                ; get index in bits
  add rdx, r10      
  add rdx, 24                               ; add rdi + 24 to get start of key array

  .begin:
    ; if we've hit an empty space the key is not valid
    cmp qword[rdx], 0
    jz .fail

    ; TODO loop back if we're at the end of the table
    ; TODO return fail if we've reached our original position

    ; repe cmps compares strings one byte at a time; it expects
    ; rcx == strlen, rdi == str1, rsi == str2
    ; clobbers all registers, so we have to reset them each time
    mov rcx, r9                             
    mov rdi, r8
    mov rsi, [rdx]
    repe cmpsb
    
    ; zero flag will be set if the two strings are equal

    jz .success
    add rdx, 8
    jmp .begin
  
  .success:

  .fail:
  

global fhtw_hash
fhtw_hash:
  hash_function rdi, rsi, rax, rdx, r11
  ret


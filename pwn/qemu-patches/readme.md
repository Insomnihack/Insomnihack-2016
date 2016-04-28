## Infos

When doing the teaser, the toasted challenge got pwned by multiple teams with a "simple" read/jmp shellcode on the stack, because qemu does not enforce NX in userland emulation. 
This motivated me to write some security enhancements to qemu-user, which started on the PIE patch made by gynophage [here](https://github.com/legitbs/finals-2014/blob/master/eliza/qemu/qemu-2.1.0-rc1.patch) for the defcon 2014 finals.

The PIE implementation was ported to the stack and basically all the mmap'ing done by qemu.

NX was simply implemented by checking at _each_ instructions where we're at IE if $pc is in a X segment.

W^X was a bit more difficult, as a segment mapped W cannot in the future be mapped R then RX. Two new bits where setup to mark if the page Was ever marked write or executable

The mmap option only shows the addresses at which the segment is mapped, with it's permissions.



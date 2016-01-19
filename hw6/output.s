.data
_g_pi: .float 3.141600
.text
.text
_start_sqr:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_sqr
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x8, [sp, #48]
str x14, [sp, #56]
str x15, [sp, #64]
str s16, [sp, #72]
str s17, [sp, #76]
str s18, [sp, #80]
str s19, [sp, #84]
str s20, [sp, #88]
str s21, [sp, #92]
str s22, [sp, #96]
str s23, [sp, #100]
ldr s16, [x29, #16]
ldr s17, [x29, #16]
fmul s16, s16, s17
fmov s0, s16
bl _end_sqr
_end_sqr:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x8, [sp, #48]
ldr x14, [sp, #56]
ldr x15, [sp, #64]
ldr s16, [sp, #72]
ldr s17, [sp, #76]
ldr s18, [sp, #80]
ldr s19, [sp, #84]
ldr s20, [sp, #88]
ldr s21, [sp, #92]
ldr s22, [sp, #96]
ldr s23, [sp, #100]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_sqr: .word 100
.text
_start_calarea:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_calarea
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x8, [sp, #48]
str x14, [sp, #56]
str x15, [sp, #64]
str s16, [sp, #72]
str s17, [sp, #76]
str s18, [sp, #80]
str s19, [sp, #84]
str s20, [sp, #88]
str s21, [sp, #92]
str s22, [sp, #96]
str s23, [sp, #100]
ldr x14, =_g_pi
ldr s16, [x14, #0]
ldr w9, [x29, #16]
scvtf s17, w9
str s17, [sp, #0]
add sp, sp, -8
bl _start_sqr
add sp, sp, 8
fmov s17, s0
fmul s16, s16, s17
str s16, [x29, #-4]
ldr s16, [x29, #-4]
fmov s0, s16
bl _end_calarea
_end_calarea:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x8, [sp, #48]
ldr x14, [sp, #56]
ldr x15, [sp, #64]
ldr s16, [sp, #72]
ldr s17, [sp, #76]
ldr s18, [sp, #80]
ldr s19, [sp, #84]
ldr s20, [sp, #88]
ldr s21, [sp, #92]
ldr s22, [sp, #96]
ldr s23, [sp, #100]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_calarea: .word 104
.text
_start_floor:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_floor
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x8, [sp, #48]
str x14, [sp, #56]
str x15, [sp, #64]
str s16, [sp, #72]
str s17, [sp, #76]
str s18, [sp, #80]
str s19, [sp, #84]
str s20, [sp, #88]
str s21, [sp, #92]
str s22, [sp, #96]
str s23, [sp, #100]
ldr s16, [x29, #16]
fcvtzs w9, s16
str w9, [x29, #-4]
ldr w9, [x29, #-4]
mov w0, w9
bl _end_floor
_end_floor:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x8, [sp, #48]
ldr x14, [sp, #56]
ldr x15, [sp, #64]
ldr s16, [sp, #72]
ldr s17, [sp, #76]
ldr s18, [sp, #80]
ldr s19, [sp, #84]
ldr s20, [sp, #88]
ldr s21, [sp, #92]
ldr s22, [sp, #96]
ldr s23, [sp, #100]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_floor: .word 104
.text
_start_MAIN:
str x30, [sp, #0]
str x29, [sp, #-8]
add x29, sp, #-8
add sp, sp, #-16
ldr x30, =_frameSize_MAIN
ldr x30, [x30, #0]
sub sp, sp, w30
str x9, [sp, #8]
str x10, [sp, #16]
str x11, [sp, #24]
str x12, [sp, #32]
str x13, [sp, #40]
str x8, [sp, #48]
str x14, [sp, #56]
str x15, [sp, #64]
str s16, [sp, #72]
str s17, [sp, #76]
str s18, [sp, #80]
str s19, [sp, #84]
str s20, [sp, #88]
str s21, [sp, #92]
str s22, [sp, #96]
str s23, [sp, #100]
.data
_CONSTANT_0: .ascii "Enter an Integer :\000"
.align 3
.text
ldr x9, =_CONSTANT_0
mov x0, x9
bl _write_str
bl _read_int
mov w9, w0
str w9, [x29, #-4]
ldr w9, [x29, #-4]
str w9, [sp, #0]
add sp, sp, -8
bl _start_calarea
add sp, sp, 8
fmov s16, s0
str s16, [x29, #-8]
ldr s16, [x29, #-8]
ldr s17, [x29, #-8]
str s17, [sp, #0]
add sp, sp, -8
bl _start_floor
add sp, sp, 8
mov w9, w0
scvtf s17, w9
fsub s16, s16, s17
str s16, [x29, #-12]
ldr s16, [x29, #-8]
fmov s0, s16
bl _write_float
.data
_CONSTANT_1: .ascii " \000"
.align 3
.text
ldr x9, =_CONSTANT_1
mov x0, x9
bl _write_str
ldr s16, [x29, #-12]
fmov s0, s16
bl _write_float
.data
_CONSTANT_2: .ascii "\n\000"
.align 3
.text
ldr x9, =_CONSTANT_2
mov x0, x9
bl _write_str
.data
_CONSTANT_3: .word 0
.align 3
.text
ldr w9, _CONSTANT_3
mov w0, w9
bl _end_MAIN
_end_MAIN:
ldr x9, [sp, #8]
ldr x10, [sp, #16]
ldr x11, [sp, #24]
ldr x12, [sp, #32]
ldr x13, [sp, #40]
ldr x8, [sp, #48]
ldr x14, [sp, #56]
ldr x15, [sp, #64]
ldr s16, [sp, #72]
ldr s17, [sp, #76]
ldr s18, [sp, #80]
ldr s19, [sp, #84]
ldr s20, [sp, #88]
ldr s21, [sp, #92]
ldr s22, [sp, #96]
ldr s23, [sp, #100]
ldr x30, [x29, #8]
mov sp, x29
add sp, sp, #8
ldr x29, [x29,#0]
RET x30
.data
_frameSize_MAIN: .word 112

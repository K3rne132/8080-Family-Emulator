;Intel 8085 hidden instruction set tester
org 0x100
jmp start

chc_msg: db "check finished",0x0A,0x0D,'$'
err_msg: db "check failed",0x0A,0x0D,'$'

start:
lxi h, 0x1234
db 0x10 ; ARHL
jc err
mov a, h
cpi 0x09
jnz err
mov a, l
cpi 0x1a
jnz err

lxi h, 0x89ab
db 0x10 ; ARHL
jnc err
mov a, h
cpi 0x44
jnz err
mov a, l
cpi 0xd5
jnz err
call fin

lxi h, 0x4321
lxi b, 0x1234
db 0x08 ; DSUB
jz err
jpo err
jp err
mov a, h
cpi 0x30
mov a, l
cpi 0xed
jnz err
mov a, b
cpi 0x12
mov a, c
cpi 0x34
jnz err
call fin

mvi a, 0x50
mvi e, 0xf0
sub e
jm err
db 0xFD
dw err ; JK err
cpi 0x60
jnz err

mvi a, 0x50
mvi e, 0xb0
sub e
jp err
db 0xFD
dw err ; JK err
cpi 0xa0
jnz err

mvi a, 0x50
mvi e, 0x30
sub e
jm err
db 0xFD
dw err ; JK err
cpi 0x20
jnz err

mvi a, 0xd0
mvi e, 0xb0
sub e
jm err
db 0xFD
dw err ; JK err
cpi 0x20
jnz err

lxi h, 0x1234
lxi d, 0x9876
db 0x28,0xda ; LDHI 0xda
mov a, d
cpi 0x13
jnz err
mov a, e
cpi 0x0e
jnz err
call fin

lxi sp, 0x1234
lxi d, 0x9876
db 0x38,0xda ; LDSI 0xda
mov a, d
cpi 0x13
jnz err
mov a, e
cpi 0x0e
jnz err
call fin

lxi h, 0x0000
lxi d, 0x1000
mvi a, 0x12
stax d
inx d
mvi a, 0x34
stax d
dcx d
db 0xed ; LHLX
mov a, l
cpi 0x12
jnz err
mov a, h
cpi 0x34
jnz err
call fin

lxi h, 0x4321
lxi d, 0x2000
db 0xd9 ; SHLX
ldax d
cpi 0x21
jnz err
inx d
ldax d
cpi 0x43
jnz err
call fin

lxi d, 0x1234
db 0x18 ; RDEL
jc err
mov a, d
cpi 0x24
jnz err
mov a, e
cpi 0x68
jnz err

lxi d, 0x6789
db 0x18 ; RDEL
jc err
mov a, d
cpi 0xcf
jnz err
mov a, e
cpi 0x12
jnz err
call fin

hlt

fin:
	lxi d, chc_msg
	call print
	ret

err:
	lxi d, err_msg
	call print
	hlt
	
print:
	mvi c, 9
	call 0x0005
	ret

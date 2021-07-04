nop

mov x 1
sys 0xB

;print working strings for nop, sys, mov
mov x 0x6e  sys 6 mov x 0x6f  sys 6 
mov x 0x70  sys 6 mov x 0x20  sys 6 
mov x 0x77  sys 6 mov x 0x6f  sys 6 
mov x 0x72  sys 6 mov x 0x6b  sys 6 
mov x 0x69  sys 6 mov x 0x6e  sys 6 
mov x 0x67  sys 6 mov x 0x21  sys 6 
mov x 0x0a  sys 6 mov x 0x73  sys 6 
mov x 0x79  sys 6 mov x 0x73  sys 6 
mov x 0x20  sys 6 mov x 0x77  sys 6 
mov x 0x6f  sys 6 mov x 0x72  sys 6 
mov x 0x6b  sys 6 mov x 0x69  sys 6 
mov x 0x6e  sys 6 mov x 0x67  sys 6 
mov x 0x21  sys 6 mov x 0x0a  sys 6 
mov x 0x6d  sys 6 mov x 0x6f  sys 6 
mov x 0x76  sys 6 mov x 0x20  sys 6 
mov x 0x77  sys 6 mov x 0x6f  sys 6 
mov x 0x72  sys 6 mov x 0x6b  sys 6 
mov x 0x69  sys 6 mov x 0x6e  sys 6 
mov x 0x67  sys 6 mov x 0x21  sys 6
mov x 0x0a  sys 6

;test jmp
jmp jmp_works

mov x 2
sys 0xB

;print jmp not working
mov x 0x6a  sys 6
mov x 0x6d  sys 6
mov x 0x70  sys 6
mov x 0x20  sys 6
mov x 0x6e  sys 6
mov x 0x6f  sys 6
mov x 0x74  sys 6
mov x 0x20  sys 6
mov x 0x77  sys 6
mov x 0x6f  sys 6
mov x 0x72  sys 6
mov x 0x6b  sys 6
mov x 0x69  sys 6
mov x 0x6e  sys 6
mov x 0x67  sys 6
mov x 0x0a  sys 6
ext 1

;print ' not working'
not_working:
  mov x 0x20  sys 6  mov x 0x6e  sys 6 
  mov x 0x6f  sys 6  mov x 0x74  sys 6 
  mov x 0x20  sys 6  mov x 0x77  sys 6 
  mov x 0x6f  sys 6  mov x 0x72  sys 6 
  mov x 0x6b  sys 6  mov x 0x69  sys 6 
  mov x 0x6e  sys 6  mov x 0x67  sys 6 
  mov x 0x0a  sys 6  ext 1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_string:
  ;psh y ;uncomment this if you want to preserve y register
  mov y x
  -:
    jeq + [y] 0
    mov x [y]
    sys 6
    add y 1
    jmp -
  +: ; pop y ;uncomment this if you want to preserve y register
  ret

;doubles number
test_jsr:
  mov a x
  mov x 1
  sys 0xB
  mov x 0x6a  sys 6 mov x 0x73  sys 6 
  mov x 0x72  sys 6 mov x 0x20  sys 6 
  mov x 0x77  sys 6 mov x 0x6f  sys 6 
  mov x 0x72  sys 6 mov x 0x6b  sys 6 
  mov x 0x69  sys 6 mov x 0x6e  sys 6 
  mov x 0x67  sys 6 mov x 0x21  sys 6 
  mov x 0x0a  sys 6 
  mov x a
  add x x
  ret

  mov x 2
  sys 0xB
  mov x 0x72  sys 6 mov x 0x65  sys 6  mov x 0x74  sys 6 
  jmp not_working


assert: ;a = address of op text, x = value, y = expected;
  jeq assert_return x y
  mov x 2
  sys 0xB
  mov x a
  jsr print_string
  mov x not_working_t
  jsr print_string
  ext 1
  assert_return: ret

print_working: ;a = address of op text
  mov x a
  jsr print_string
  mov x working_t
  jsr print_string
  ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;print jmp working
jmp_works:
  mov x 0x6a  sys 6 mov x 0x6d  sys 6 
  mov x 0x70  sys 6 mov x 0x20  sys 6 
  mov x 0x77  sys 6 mov x 0x6f  sys 6 
  mov x 0x72  sys 6 mov x 0x6b  sys 6 
  mov x 0x69  sys 6 mov x 0x6e  sys 6 
  mov x 0x67  sys 6 mov x 0x21  sys 6 
  mov x 0x0a  sys 6

;test jeq
jeq + 5 5
mov x 2 ; color red
sys 0xB
mov x 0x6a  sys 6 mov x 0x65  sys 6  mov x 0x71  sys 6 ;'jeq'
jmp not_working

;print jeq working
+: mov x 0x6a  sys 6 mov x 0x65  sys 6
mov x 0x71  sys 6 mov x 0x20  sys 6 
mov x 0x77  sys 6 mov x 0x6f  sys 6 
mov x 0x72  sys 6 mov x 0x6b  sys 6 
mov x 0x69  sys 6 mov x 0x6e  sys 6 
mov x 0x67  sys 6 mov x 0x21  sys 6 
mov x 0x0a  sys 6 

;test add
mov x 5
add x 7
jeq + x 12
mov x 2 ; color red
sys 0xB
mov x 0x61  sys 6 mov x 0x64  sys 6  mov x 0x64  sys 6 ;'add'
jmp not_working

; add works
+: mov x 0x61  sys 6 mov x 0x64  sys 6 
mov x 0x64  sys 6 mov x 0x20  sys 6 
mov x 0x77  sys 6 mov x 0x6f  sys 6 
mov x 0x72  sys 6 mov x 0x6b  sys 6 
mov x 0x69  sys 6 mov x 0x6e  sys 6 
mov x 0x67  sys 6 mov x 0x21  sys 6 
mov x 0x0a  sys 6 

mov x 5
jsr test_jsr
jeq + x 10
mov x 2 ; color red
sys 0xB
mov x 0x6a  sys 6 mov x 0x73  sys 6  mov x 0x72  sys 6 ;'jsr'
jmp not_working

+: mov x ret_t
  jsr print_string
  mov x working_t
  jsr print_string


mov a sub_t
mov x 5
sub x 3
mov y 2
jsr assert ; 5 - 3 = 2
mov x 5
sub x 0
mov y 5
jsr assert ; 5 - 0 = 5
mov x 0
sub x 1
mov y 0xffff ; 0 - 1 = 0xffff
jsr assert
jsr print_working


mov a mul_t
mov x 5
mul x 3
mov y 15
jsr assert ; 5 * 3 = 5
mov x 5
mul x 0
mov y 0
jsr assert ; 5 - 0 = 0
mov x 0x1234
mul x 0x1234
mov y 0x5a90 ; 0x1234 ^ 2 = 0x5a90 (wrapping)
jsr assert
jsr print_working


mov a mod_t
mov x 5
mod x 3
mov y 2
jsr assert ; 5 % 3 = 2
mov x 3
mod x 5
mov y 3
jsr assert ; 3 % 5 = 3
mov x 0x1234
mod x 2
mov y 0 ; 0x1234 % 2 = 0
jsr assert
jsr print_working


;testing jne
jne jne_fail 2 2
jne + 2 3
jmp jne_fail
jne + 2 1
jne_fail:
  mov x 2
  sys 0xB
  mov x jne_n
  jsr print_string
  ext 1
+:
  mov x jne_w
  jsr print_string

;testing jgt
jgt jgt_fail 2 2
jgt + 3 2
jmp jgt_fail
+: jgt jgt_fail 2 3
jmp +
jgt_fail:
  mov x 2
  sys 0xB
  mov x jgt_n
  jsr print_string
  ext 1
+:
  mov x jgt_w
  jsr print_string

;testing jge
jge + 2 2
jmp jge_fail
+: jge + 3 2
jmp jge_fail
+: jge jge_fail 2 3
jmp +
jge_fail:
  mov x 2
  sys 0xB
  mov x jge_n
  jsr print_string
  ext 1
+:
  mov x jge_w
  jsr print_string

;testing jlt
jlt jlt_fail 2 2
jlt jlt_fail 3 2
jlt + 2 3
jlt_fail:
  mov x 2
  sys 0xB
  mov x jlt_n
  jsr print_string
  ext 1
+:
  mov x jlt_w
  jsr print_string

;testing jle
jle + 2 2
jmp jle_fail
+: jle jle_fail 3 2
jle + 2 3
jle_fail:
  mov x 2
  sys 0xB
  mov x jle_n
  jsr print_string
  ext 1
+:
  mov x jle_w
  jsr print_string
  


mov a and_t
mov x 5
and x 3
mov y 1
jsr assert ; 5 & 3 = 1
mov x 47
and x 29
mov y 13
jsr assert ; 47 & 29 = 13
mov x 0x1234
and x 0xf0f0
mov y 0x1030 ; 0x1234 & 0xf0f0 = 0x1030
jsr assert
jsr print_working

mov a orr_t
mov x 5
orr x 3
mov y 7
jsr assert ; 5 | 3 = 7
mov x 47
orr x 29
mov y 63
jsr assert ; 47 | 29 = 63
mov x 0x8080
orr x 0x0301
mov y 0x8381 ; 0x8080 | 0x0301 = 0x8381
jsr assert
jsr print_working

mov a not_t
mov x 5
not x
mov y 0xfffa
jsr assert ; 5 | 3 = 7
mov x 0x8000
not x
mov y 0x7fff
jsr assert ; ~0x8000 = 0x7fff
mov x 0xf0f0
not x
mov y 0x0f0f ; ~0xf0f0 = 0x0f0f0
jsr assert
jsr print_working

mov a xor_t
mov x 5
xor x 3
mov y 6
jsr assert ; 5 ^ 3 = 6
mov x 0x5057
xor x 0x1234
mov y 0x4263
jsr assert ; 0x5057 ^ 0x1234 = 0x4263
mov x 0x8080
xor x 0x3301
mov y 0xb381 ; 0x8080 ^ 0x0301 = 0x8381
jsr assert
jsr print_working

mov a lsl_t
mov x 5
lsl x 11
mov y 0x2800
jsr assert ; 5 << 11 == 0x2800
mov x 0x5057
lsl x 0x1234
mov y 0x0000
jsr assert ; 0x5057 << 0x1234 = 0x0000
mov x 0x3457
lsl x 3
mov y 0xa2b8 ; 0x3457 << 3 = 0xa2b8
jsr assert
jsr print_working

mov a lsr_t
mov x 0xc000
lsr x 11
mov y 0x0018
jsr assert ; 0xc000 >> 11 == 0x0018
mov x 0x5057
lsr x 0x1234
mov y 0x0000
jsr assert ; 0x5057 >> 0x1234 = 0x0000
mov x 0x3457
lsr x 3
mov y 0x068a ; 0x3457 >> 3 = 0x068a
jsr assert
jsr print_working

;test push and pop
mov x 0
psh 5
pop x
jne psh_fail x 5
psh 8
psh 2
pop x
pop x
jne psh_fail x 8
psh 1
psh 2
psh 3
psh 0x8080
psh 0x1234
psh 0xffff
pop x
pop x
pop x
jne psh_fail x 0x8080
pop x
pop x
jne psh_fail x 2
jmp psh_working
psh_fail:
  mov x 2
  sys 0xB
  mov x psh_t
  jsr print_string
  mov x not_working_t
  jsr print_string
  mov x pop_t
  jsr print_string
  mov x not_working_t
  jsr print_string
  ext 1
psh_working:
  mov x psh_t
  jsr print_string
  mov x working_t
  jsr print_string
  mov x pop_t
  jsr print_string
  mov x working_t
  jsr print_string

mov x ext_t
jsr print_string
mov x working_t
jsr print_string
ext 0



jne_n: .text('jne not working') 0xa 0x0
jne_w: .text('jne working!') 0xa 0x0

jgt_n: .text('jgt not working') 0xa 0x0
jgt_w: .text('jgt working!') 0xa 0x0

jge_n: .text('jge not working') 0xa 0x0
jge_w: .text('jge working!') 0xa 0x0

jlt_n: .text('jlt not working') 0xa 0x0
jlt_w: .text('jlt working!') 0xa 0x0

jle_n: .text('jle not working') 0xa 0x0
jle_w: .text('jle working!') 0xa 0x0

working_t: .text(' working!') 0xa 0x0
not_working_t: .text(' not working') 0xa 0x0

ret_t: .text('ret') 0x0
sub_t: .text('sub') 0x0
mul_t: .text('mul') 0x0
mod_t: .text('mod') 0x0
and_t: .text('and') 0x0
orr_t: .text('orr') 0x0
not_t: .text('not') 0x0
xor_t: .text('xor') 0x0
lsl_t: .text('lsl') 0x0
lsr_t: .text('lsr') 0x0
psh_t: .text('psh') 0x0
pop_t: .text('pop') 0x0
ext_t: .text('ext') 0x0
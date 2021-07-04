jmp start
number: 1337


print_string:
  mov y x
  -:
    jeq + [y] 0
    mov x [y]
    sys 6
    add y 1
    jmp -
  +:
  ret

decimal_t: .text('    decimal: ') 0x0
hex_t:     .text('hexadecimal: ') 0x0
binary_t:  .text('     binary: ') 0x0
numbits_s: .text('  bit count: ') 0x0

start:
  mov x decimal_t
  jsr print_string
  mov x $number
  jsr print_dec
  mov x 0xA
  sys 6

  mov x hex_t
  jsr print_string
  mov x $number
  jsr print_hex
  mov x 0xA
  sys 6

  mov x binary_t
  jsr print_string
  mov x .text('0')
  sys 6
  mov x .text('b')
  sys 6
  mov a 16
  mov d 0
  loop:
    sub a 1
    mov b $number
    mov c 1
    lsl c a
    and b c
    mov x 48
    jeq + b 0
      add x 1
      add d 1
    +: sys 6
    jne loop a 0
  mov x 0xA
  sys 6
  mov x numbits_s
  jsr print_string
  mov x d
  jsr print_dec
  ext 0










########################################################
#                    printnum.asm                      #
########################################################

exp: ;calculate x ^ y
  psh a
  psh b
  psh c
  psh d
  mov a 1
  -:
    jeq + y 0
    mul a x
    sub y 1
    jmp -
  +: mov x a
    pop d
    pop c
    pop b
    pop a
  ret

fast_exp_10: ;calculate 10 ^ x
  jle + x 4
  mov x 2
  sys 0xB
  mov x fast_exp_10_error_t
  jsr print_string
  ext 1
  +: mov y fast_exp_10_lookup
    add y x
    mov x [y]
  ret

  fast_exp_10_lookup: 0x1 0xa 0x64 0x3e8 0x2710
  fast_exp_10_error_t: .text('fast_exp_10 only works for exponents <= 4 because of 16 bit limit') 0x0



fast_exp_10_unsafe: ;calculate 10 ^ x
  mov y fast_exp_10_lookup
  add y x
  mov x [y]
  ret



print_dec: ;print a value (x) in decimal format
  psh a
  psh b
  psh c
  psh d
  mov $print_dec_z_flag 0
  mov a 0
  mov d x
  mov b 4
  -:
    ;mov x 10              ;normal exp
    ;mov y b
    ;jsr exp

    mov x b              ;fast exp   
    jsr fast_exp_10     

    mov c x
    jlt else d c
      sub d c
      add a 1
      jmp +
    else:
      mov x a
      mov a 0
      sub b 1
      jeq print_dec_skip $print_dec_z_flag 1
      jeq + x 0
      print_dec_skip: mov $print_dec_z_flag 1
      add x 48
      sys 6
    +: jne - b 0xffff
  pop d
  pop c
  pop b
  pop a
  ret
print_dec_z_flag: 0x0

print_hex: ;print a value (x) in hex format, preceded by 0x
  psh a
  psh b
  psh c
  psh d
  mov y x
  mov c 12
  mov x 48
  sys 6
  mov x 120
  sys 6
-:
  mov a y
  mov b 0b1111
  lsl b c
  and a b
  lsr a c
  jgt abcdef a 9
  add a 48
  jmp repeat
  abcdef:
    sub a 10
    add a 65
  repeat: mov x a
    sys 6
    jeq + c 0
    sub c 4
    jmp -
  +:
    pop d
    pop c
    pop b
    pop a
  ret  
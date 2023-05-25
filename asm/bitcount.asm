jmp start_file

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

print_file:
  sys 7
  -:
    jeq + x 0
    sys 6
    sys 7
    jmp -
  +:
  ret

print_header:
  mov x aoc_day
  jsr print_file
  mov x 0xA
  sys 6
  ext 0

start_file:
  jsr print_file
  mov x 0xA
  sys 6
  ext 0
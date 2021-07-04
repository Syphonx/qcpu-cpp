jmp read_program_in

bit_width: 16

read_program_in:
  mov a program
  mov c 0
  -: #read program in
    sys 7
    jeq read_program_end x 0
    jeq read_program_end x .text('*')
    jeq + x .text('<')
    jeq + x .text('>')
    jeq + x .text('+')
    jeq + x .text('-')
    jeq + x .text('[')
    jeq + x .text(']')
    jeq + x .text(',')
    jeq + x .text('.')
      jmp -
    +:
    mov [a] x
    jne + x 91 
      psh a
    +: jne + x 93
      pop d
      # a is current pc
      # d is matching previous bracket
      # c is index (program - a)
      mov y a
      sub y d
      mov b c
      sub b y
      # b is index of previous bracket
      mov x brackets
      add x b
      mov [x] a
      mov x brackets
      add x c
      mov [x] d
    +: 
      add c 1
      add a 1
      jmp -
  read_program_end:
  
  mov $index tape
  mov $pc program
  exec: mov x $pc
    mov x [x]
    jeq done x 0
    jne + x .text('<')
      sub $index 1
      jmp continue
    +: jne + x .text('>')
      add $index 1
      jmp continue
    +: jne + x .text('+')
      mov x $index
      add [x] 1
      jmp continue
    +: jne + x .text('-')
      mov x $index
      sub [x] 1
      jmp continue
    +: jne + x .text('[')
      mov x $index
      jne + [x] 0
        mov x $pc
        sub x program
        mov a brackets
        add a x
        mov $pc [a]
      jmp continue
    +: jne + x .text(']')
      mov x $index
      jeq + [x] 0
        mov x $pc
        sub x program
        mov a brackets
        add a x
        mov $pc [a]
      jmp continue
    +: jne + x .text(',')
      sys 7
      mov y $index
      mov [y] x
      jmp continue
    +: jne + x .text('.')
      mov y $index
      mov x [y]
      sys 6
    +: continue:
      add $pc 1
      jmp exec
    done:
      ext 0





brackets_length: 0
brackets: .ds(0x1000)

pc: 0
program: .ds(0x1000)

index: 0
tape: .ds(0x1000)
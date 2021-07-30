jmp init

alphabet:
a_a: 3        1 7 4 1  4 1 7 7  2 5 6 5
a_b: 8        1 1 1 7  1 1 4 1  4 1 5 3  5 3 4 4  1 4 5 4  5 4 7 6  7 6 6 7  1 7 6 7
a_c: 7        7 2 5 1  5 1 3 1  3 1 1 3  1 3 1 5  1 5 3 7  3 7 5 7  5 7 7 6  
a_d: 6        1 1 1 7  1 1 5 1  5 1 7 3  7 3 7 5  7 5 5 7  1 7 5 7  
a_e: 4        1 1 1 7  1 1 7 1  1 4 6 4  1 7 7 7  
a_f: 3        1 1 1 7  1 1 7 1  1 4 6 4  
a_g: 9        7 2 5 1  5 1 3 1  3 1 1 3  1 3 1 5  1 5 3 7  3 7 5 7  5 7 7 5  5 5 7 5  7 5 7 7  
a_h: 3        1 1 1 7  1 4 7 4  7 1 7 7 
a_i: 3        2 1 6 1  4 1 4 7  1 7 7 7  
a_j: 6        2 1 7 1  5 1 7 3  7 3 7 5  7 5 5 7  5 7 3 7  3 7 1 6   
a_k: 4        1 1 1 7  1 4 3 3  3 3 4 1  3 3 7 7  
a_l: 2        1 1 1 7  1 7 7 7  
a_m: 4        1 7 1 1  1 1 4 4  4 4 7 1  7 1 7 7  
a_n: 3        1 7 1 1  1 1 7 7  7 7 7 1  
a_o: 8        7 3 5 1  5 1 3 1  3 1 1 3  1 3 1 5  1 5 3 7  3 7 5 7  5 7 7 5  7 3 7 5  
a_p: 5        1 1 1 7  1 1 6 1  6 1 7 3  7 3 6 5  1 5 6 5  
a_q: 9        7 3 5 1  5 1 3 1  3 1 1 3  1 3 1 5  1 5 3 7  3 7 5 7  5 7 7 5  7 3 7 5  4 4 8 8  
a_r: 6        1 1 1 7  1 1 4 1  4 1 5 3  5 3 4 4  1 4 4 4  4 4 7 7  
a_s: 7        7 3 6 1  6 1 2 1  2 1 1 3  1 3 7 5  7 5 6 7  2 7 6 7  1 5 2 7  
a_t: 2        1 1 7 1  4 1 4 7  
a_u: 5        1 1 1 6  1 6 3 7  3 7 5 7  5 7 7 6  7 6 7 1  
a_v: 2        1 1 4 7  4 7 7 1 
a_w: 4        1 1 2 7  2 7 4 3  4 3 6 7  6 7 7 1   
a_x: 2        1 1 7 7  1 7 7 1  
a_y: 2        1 1 4 4  7 1 1 7  
a_z: 3        1 1 7 1  7 1 1 7  1 7 7 7  
end: 0

alphabet_map: .ds(30)

text: 
    .text('hello ') 2 .text('jak') 1 10
    .text('here is my text rendering') 10
    .text('code using a font i made') 10
    10
    .text('here is all the characters') 10
    .text('  abcdefg') 10
    .text('  hijklmn') 10
    .text('  opqrstu') 10
    .text('  vwxyz') 10
    10
    .text('i also wrote a simple') 10
    2 .text('random number generator') 1 10
    .text('which i am using to') 10
    .text('give the letters a bit of') 10
    2 .text('jitteriness') 1 10
    0

orig_pos_x: 4096
orig_pos_y: 4096
pos_x: 4096
pos_y: 4096

grid_size: 0x100
grid_count: 8
char_size: 0 ; grid size * grid count

init:
    ; set char size
    mov $char_size $grid_size
    mul $char_size $grid_count 

    ; build alphabet map, which is index of memory addresses for each chara
    ; so alphabet_map[n] will give you address of nth letter
    mov a alphabet
    mov x alphabet_map
-:  mov [x] a
    mov b [a]
    mul b 4
    add a b
    add a 1
    add x 1
    jne - [a] 0



loop:
    mov $pos_x $orig_pos_x
    mov $pos_y $orig_pos_y

    mov b text
-:  mov c [b]
    jeq space c 32
    jeq newline c 10
    jeq color_white c 1
    jeq color_red c 2
character:
    sub c .text('a')
    add c alphabet_map
    mov a [c]

    mov x $pos_x
    mov y $pos_y
    jsr print_letter  
    jmp next

newline:
    mov $pos_x $orig_pos_x
    add $pos_y $char_size
    jmp next_no_advance

space:
    jmp next

color_white:
    mov x 0
    sys 25
    jmp next_no_advance
color_red:
    mov x 1
    sys 25
    jmp next_no_advance

next:
    add $pos_x $char_size
next_no_advance:
    add b 1
    
    jne - [b] 0

    sys 32
    jsr rand
    jmp loop








letter_pos_x: 0
letter_pos_y: 0
letter_address: 0
print_letter:
    psh b psh c psh d ; save arguments to local variable storage
    mov $letter_pos_x x
    mov $letter_pos_y y
    mov $letter_address a
    add $letter_address 1               ; add 1 because we skip first value
    mov d [a]                           ; load number of lines into d
    sub d 1
-:                                      ; foreach line of character
    mov c d                             ; draw from x1,y1 to x2,y2
    mul c 4                             ; multiply character size by $grid_size
    add c $letter_address               ; and offset drawing position by argument x, y

    mov x [c]
    mul x $grid_size
    add x $letter_pos_x
    add x $rand_num_small

    add c 1
    mov y [c]
    mul y $grid_size
    add y $letter_pos_y
    add y $rand_num_small

    add c 1
    mov a [c]
    mul a $grid_size
    add a $letter_pos_x
    add a $rand_num_small

    add c 1
    mov b [c]
    mul b $grid_size
    add b $letter_pos_y
    add b $rand_num_small

    sys 21
    jsr rand

    jeq + d 0
    sub d 1
    jmp -
+:  pop d pop c pop b
    ret






rand:
    psh y
	mov y $rand_num
	lsr y 1
	mul $rand_num 3
	xor $rand_num y
    mov $rand_num_small $rand_num
    mod $rand_num_small 0x100
    pop y
	ret


rand_num_small: 0
rand_num: 1
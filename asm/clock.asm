jmp init

digits: 
    d_0: 9        7 3 6 1  6 1 2 1  2 1 1 3  1 3 1 5  1 5 2 7  2 7 6 7  6 7 7 5  7 3 7 5  7 1 1 7  
    d_1: 3        2 2 4 1  4 1 4 7  1 7 7 7  
    d_2: 5        1 3 3 1  3 1 5 1  5 1 7 3  7 3 1 7  1 7 7 7  
    d_3: 7        2 1 7 1  7 1 4 3  4 3 7 4  7 4 7 6  7 6 5 7  5 7 3 7  3 7 1 6  
    d_4: 3        5 8 5 1  5 1 1 6  1 6 7 6  
    d_5: 7        6 1 1 1  1 1 1 3  1 3 5 3  5 3 7 4  7 4 7 6  7 6 5 7  5 7 1 7  
    d_6: 8        3 1 1 3  1 3 1 6  1 6 2 7  2 7 6 7  6 7 7 6  7 6 7 5  7 5 6 4  6 4 1 4  
    d_7: 3        2 1 7 1  7 1 1 7  2 4 6 4  
    ;d_8: 15       2 1 1 2  1 2 1 3  1 3 2 4  2 4 1 5  1 5 1 6  1 6 2 7  2 7 6 7  6 7 7 6  7 6 7 5  7 5 6 4  6 4 7 3  7 3 7 2  7 2 6 1  6 1 2 1  2 4 6 4  
    d_8: 8        2 1 1 3  1 3 7 5  7 5 6 7  6 7 2 7  2 7 1 5  1 5 7 3  7 3 6 1  6 1 2 1  
    d_9: 5        7 4 2 4  2 4 1 2  1 2 2 1  2 1 7 1  7 1 7 7  
end: 0
colon: 4        3 1 5 3  5 1 3 3  3 5 5 7  3 7 5 5  

digit_map: .ds(100)

pos_x: 0x8000
pos_y: 0x8000

grid_size: 0x400
grid_count: 8
char_size: 0 ; grid size * grid count

init:
    ; set char size
    mov $char_size $grid_size
    mul $char_size $grid_count 

    mov x $char_size
    lsr x 1
    sub $pos_y x
    mul x 5
    sub $pos_x x

    ; build alphabet map, which is index of memory addresses for each chara
    ; so digit_map[n] will give you address of nth number character
    mov a digits
    mov x digit_map
-:  mov [x] a
    mov b [a]
    mul b 4
    add a b
    add a 1
    add x 1
    jne - [a] 0



loop:
    add $timer_ticks 1
    jlt + $timer_ticks 60
    add $timer_sd 1
    mov $timer_ticks 0
+:  
    jlt + $timer_sd 10
    add $timer_st 1
    mov $timer_sd 0
+: 
    jlt + $timer_st 6
    add $timer_md 1
    mov $timer_st 0
+: 
    jlt + $timer_md 10
    add $timer_mt 1
    mov $timer_md 0
+:

; minute tens
    mov d $pos_x
    mov x d
    mov y $pos_y

    mov a $timer_mt
    add a digit_map
    mov a [a]
    jsr print_letter

; minute digits
    add d $char_size
    mov x d
    mov y $pos_y

    mov a $timer_md
    add a digit_map
    mov a [a]
    jsr print_letter

; colon
    mov x 1
    sys 25
    add d $char_size
    mov x d
    mov y $pos_y

    mov a colon
    jsr print_letter
    mov x 0
    sys 25

; second tens
    add d $char_size
    mov x d
    mov y $pos_y

    mov a $timer_st
    add a digit_map
    mov a [a]
    jsr print_letter

; second digits
    add d $char_size
    mov x d
    mov y $pos_y

    mov a $timer_sd
    add a digit_map
    mov a [a]
    jsr print_letter



    sys 32
    jsr rand
    jmp loop



timer:
timer_mt: 0
timer_md: 0
timer_st: 0
timer_sd: 0
timer_ticks: 0


.org(0x1000)

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

    jsr rand

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
    mod $rand_num_small 0x200
    pop y
	ret


rand_num_small: 0
rand_num: 1
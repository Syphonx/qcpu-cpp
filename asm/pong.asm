jmp start

ball_position: 
ball_position_x: 0x8000
ball_position_y: 0x8000

ball_velocity:
ball_velocity_x: 0x80FF ; first bit = 0 ? left : right
ball_velocity_y: 0x8080 ; first bit = 0 ? up : down

ball_size: 0xFF








start:
    jsr move_ball
    jsr draw_ball
	sys 0x20
    jmp start







draw_ball_inner_brightness: 0
draw_ball_top: 0
draw_ball_bottom: 0
draw_ball_left: 0
draw_ball_right: 0
draw_ball:
    mov $draw_ball_top $ball_position_y
    mov $draw_ball_bottom $ball_position_y
    mov $draw_ball_left $ball_position_x
    mov $draw_ball_right $ball_position_x
    sub $draw_ball_top $ball_size
    add $draw_ball_bottom $ball_size
    sub $draw_ball_left $ball_size
    add $draw_ball_right $ball_size


    mov x 0
    sys 25
    mov x $draw_ball_left
    mov y $draw_ball_top
    sys 22
    mov x $draw_ball_right
    sys 23
    mov y $draw_ball_bottom
    sys 23
    mov x $draw_ball_left
    sys 23
    mov y $draw_ball_top
    sys 23

    mov x 1
    sys 25

    mov x $draw_ball_inner_brightness
    sys 24
    add $draw_ball_inner_brightness 0xFF

    mov x $draw_ball_left
    mov a $draw_ball_right
    mov b $draw_ball_bottom
    sys 21

    mov y $draw_ball_bottom
    mov b $draw_ball_top
    sys 21

    ret










move_ball:
    jsr move_ball_x
    jsr move_ball_y
    ret

move_ball_x:
    mov x $ball_velocity_x
    and x 0b1000000000000000
    jgt move_ball_right x 0
move_ball_left:
    mov x $ball_velocity_x
    and x 0b0111111111111111                                # x = abs(velocity.x)
    mov y $ball_position_x                                  # y = position.x
    sub y x                                                 # y = position.x - velocity.x
    jge + y $ball_position_x                                # if (y > 0) { // bc numbers wrap, `n - 1  > n` if it overflows
    mov $ball_position_x y                                  #   position.x = y
    ret                                                     # }
+:                                                          # else {
    mov y $ball_position_x 
    sub x y
    mov $ball_position_x x                                  #       position.x = abs(velocity.x) - position.x
    xor $ball_velocity_x 0b100000000000000000               #       velocity.x *= -1
    ret                                                     # }
                                                            
move_ball_right:
    mov x $ball_velocity_x
    and x 0b0111111111111111                                # x = abs(velocity.x)
    mov y $ball_position_x                                  # y = position.x
    add y x                                                 # y = position.x - velocity.x
    jle + y $ball_position_x                                # if (y < width) { // bc numbers wrap, `n + 1 < n` if it overflows
    mov $ball_position_x y                                  #   position.x = y
    ret                                                     # }
+:                                                          # else {
    mov y $ball_position_x 
    sub x y
    mov $ball_position_x x                                  #       position.x = abs(velocity.x) - position.x
    xor $ball_velocity_x 0b100000000000000000               #       velocity.x *= -1
    ret                                                     # }



move_ball_y:
    mov x $ball_velocity_y
    and x 0b1000000000000000
    jgt move_ball_down x 0
move_ball_up:
    mov x $ball_velocity_y
    and x 0b0111111111111111                                # x = abs(velocity.y)
    mov y $ball_position_y                                  # y = position.y
    sub y x                                                 # y = position.y - velocity.y
    jge + y $ball_position_y                                # if (y > 0) { // bc numbers wrap, `n - 1  > n` if it overflows
    mov $ball_position_y y                                  #   position.y = y
    ret                                                     # }
+:                                                          # else {
    mov y $ball_position_y 
    sub x y
    mov $ball_position_y x                                  #       position.y = abs(velocity.y) - position.y
    xor $ball_velocity_y 0b100000000000000000               #       velocity.y *= -1
    ret                                                     # }
                                                            
move_ball_down:
    mov x $ball_velocity_y
    and x 0b0111111111111111                                # x = abs(velocity.y)
    mov y $ball_position_y                                  # y = position.y
    add y x                                                 # y = position.y - velocity.x
    jle + y $ball_position_y                                # if (y < width) // bc numbers wrap, `n + 1 < n` if it overflows
    mov $ball_position_y y                                  #   position.y = y
    ret                                                     # }
+:                                                          # else {
    mov y $ball_position_y 
    sub x y
    mov $ball_position_y x                                  #       position.y = abs(velocity.y) - position.y
    xor $ball_velocity_y 0b100000000000000000               #       velocity.y *= -1
    ret                                                     # }
             

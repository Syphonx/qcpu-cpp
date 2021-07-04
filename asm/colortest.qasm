mov a 0

-: 
  mov x a
  sys 0xB
  mov x 99 sys 6 
  mov x 111 sys 6 
  mov x 108 sys 6 
  mov x 111 sys 6 
  mov x 117 sys 6 
  mov x 114 sys 6 
  mov x 32 sys 6 
  mov x 116 sys 6 
  mov x 101 sys 6 
  mov x 115 sys 6 
  mov x 116 sys 6 
  mov x 33 sys 6 
  mov x 32 sys 6 
  mov x 58 sys 6 
  mov x 41 sys 6 
  mov x 10 sys 6 
  add a 1
  jle - a 8

mov x 10 sys 6 
mov x 10 sys 6 
mov x 10 sys 6 

mov a 0
-: 
  mov x a
  sys 0xB
  mov x 0 sys 0xC mov x 97 sys 6 
  mov x 1 sys 0xC mov x 98 sys 6 
  mov x 2 sys 0xC mov x 99 sys 6 
  mov x 3 sys 0xC mov x 100 sys 6 
  mov x 4 sys 0xC mov x 101 sys 6 
  mov x 5 sys 0xC mov x 102 sys 6 
  mov x 6 sys 0xC mov x 103 sys 6 
  mov x 7 sys 0xC mov x 104 sys 6 
  mov x 8 sys 0xC mov x 105 sys 6 
  mov x 10 sys 6 
  add a 1
  jle - a 8
ext 0
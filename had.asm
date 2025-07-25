regwrite reg_a start
jmp reg_a

delay:
regwrite reg_a dcont
regwrite reg_c 0x08 0x00
regwrite reg_b 0x00 0x00
dcont:
inc reg_b
jpine reg_a reg_c reg_b
ret

movram:
regwrite reg_g 0x00 0x01
ramr reg_g reg_h
regwrite reg_g 0x00 0x03
ramw reg_g reg_h
regwrite reg_g 0x00 0x02
ramr reg_g reg_h
regwrite reg_g 0x00 0x04
ramw reg_g reg_h
ret

black:
regwrite reg_g 0x00 0x03
regwrite reg_h 0x00 0x04
regwrite reg_i 0x00 0x40
regwrite reg_j 0x00 0x00
ramr reg_g reg_g
ramr reg_h reg_h
mul reg_i reg_h
add ar reg_g
add ar ramsize
ramw ar reg_j
ret

incregh:
inc reg_h
ret

decregh:
dec reg_h
ret

right:
regwrite reg_b movram
call reg_b
regwrite reg_g 0x00 0x01
regwrite reg_c 0x00 0x3f
ramr reg_g reg_h
regwrite reg_b incregh
regwrite call rcont
jpis reg_b reg_h reg_c

rcont:
ramw reg_g reg_h
regwrite reg_b delay
call reg_b
regwrite reg_b black
call reg_b
regwrite reg_a cont
jmp reg_a

down:
regwrite reg_b movram
call reg_b
regwrite reg_g 0x00 0x02
regwrite reg_c 0x00 0x3f
ramr reg_g reg_h
regwrite reg_b incregh
regwrite call rcont
jpis reg_b reg_h reg_c

up:
regwrite reg_b movram
call reg_b
regwrite reg_g 0x00 0x02
regwrite reg_c 0x00 0x00
ramr reg_g reg_h
regwrite reg_b decregh
regwrite call rcont
jpig reg_b reg_h reg_c

left:
regwrite reg_b movram
call reg_b
regwrite reg_g 0x00 0x01
regwrite reg_c 0x00 0x00
ramr reg_g reg_h
regwrite reg_b decregh
regwrite call rcont
jpig reg_b reg_h reg_c

changedir:
regmov iport reg_k
ret

start:
regwrite reg_b 0x00 0x00
regwrite reg_c changedir
regwrite call start1
jpine reg_c reg_b iport

start1:
regwrite reg_f 0x11 0x11
regwrite reg_e up
jpie reg_e reg_f reg_k

regwrite reg_f 0x22 0x22
regwrite reg_e left
jpie reg_e reg_f reg_k

regwrite reg_f 0x33 0x33
regwrite reg_e down
jpie reg_e reg_f reg_k

regwrite reg_f 0x44 0x44
regwrite reg_e right
jpie reg_e reg_f reg_k

cont:
regwrite reg_g 0x00 0x01
regwrite reg_h 0x00 0x02
regwrite reg_i 0x00 0x40
ramr reg_g reg_g
ramr reg_h reg_h
mul reg_i reg_h
add ar reg_g
add ar ramsize
regwrite reg_i 0x00 0xff
ramw ar reg_i
regmov ar reg_d
regwrite reg_a cont
jmp reg_a
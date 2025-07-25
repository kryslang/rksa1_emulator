regwrite reg_a 0x00 0x00
regwrite reg_b 0x00 0x00
loopa:
inc reg_a
inc reg_b
regwrite reg_c regarst
regwrite call reta
regwrite reg_d 0x00 0x06
jpig reg_c reg_a reg_d
reta:
regwrite reg_c regbrst
regwrite call reta
regwrite reg_d 0x10 0x00
jpig reg_c reg_b reg_d
retb:
add reg_b ramsize
ramw ar reg_a
regwrite reg_c loopa
jmp reg_c

regbrst:
regwrite reg_b 0x00 0x00
ret

regarst:
regwrite reg_a 0x00 0x00
ret
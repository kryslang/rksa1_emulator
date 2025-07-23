REGWRITE reg_a 0x00 0x00
REGWRITE reg_f 0x00 0x07
REGWRITE reg_b 0x00 0x20
REGWRITE reg_c 0x00 0x40
REGWRITE reg_d 0x00 0x1F
REGWRITE reg_f 0x00 0x40
REGWRITE reg_e 0x00 0x14
MUL reg_b reg_c
ADD AR ramsize
ADD AR reg_a
RAMW AR reg_d
DEC reg_a
MUL reg_b reg_c
ADD AR ramsize
ADD AR reg_a
RAMW AR reg_f
INC reg_a
INC reg_a
JPIS reg_e reg_a reg_f
HALT


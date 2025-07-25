regwrite reg_a 0x00 0x02
regwrite reg_b 0x00 0x10
regwrite reg_c 0x00 0x40
regwrite reg_d 0x00 0x3c
regwrite reg_j 0x00 0x18
regwrite reg_l 0x00 0x03
regwrite reg_e ahoj
regwrite reg_h ahoj2
regwrite reg_i ahoj3
regwrite reg_j chxd
regwrite reg_k chyd
regwrite reg_f incsa
regwrite reg_g incsb
jmp reg_e

chxd: ;změní registry tak aby se x zmenšovalo a aby se podruhé spustil chxi
regwrite reg_f decsa
regwrite reg_j chxi
jmp reg_e

chyd: ;změní registry tak aby se y zmenšovalo a aby se podruhé spustil chyi
regwrite reg_g decsb
regwrite reg_k chyi
jmp reg_e

chxi: ;změní registry tak aby se x zvětšovalo a aby se podruhé spustil chxd
regwrite reg_f incsa
regwrite reg_j chxd
jmp reg_e

chyi: ;změní registry tak aby se y zvětšovalo a aby se podruhé spustil chyd
regwrite reg_g incsb
regwrite reg_k chyd
jmp reg_e

incsa: ;zvětší x, nakreslí a zase zvětší x
inc reg_a
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
inc reg_a
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
inc reg_a
jmp reg_h

incsb: ;nakreslí a zvětší y
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
inc reg_b
jmp reg_i

decsa: ;zmenčí x, nakreslí a zase zmenší x
dec reg_a
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
dec reg_a
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
dec reg_a
jmp reg_h

decsb: ;nakreslí a zmenší y
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j
dec reg_b
jmp reg_i

ahoj: ;nějaké proskakování sem tam
jmp reg_f

ahoj2: ;nějaké proskakování sem tam
jmp reg_g

ahoj3: ;hlavní cyklus
mul reg_b reg_c
add ar ramsize
add ar reg_a
ramw ar reg_j ;tohle byly příkazy na vykreslení pixelu na x, y
jpig reg_j reg_a reg_d
jpig reg_k reg_b reg_d
jpis reg_j reg_a reg_l
jpis reg_k reg_b reg_l ;různé kontroly kdy měnit směr odrážejícího se pixelu
jmp reg_e

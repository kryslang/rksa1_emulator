f = open("main.asm", "r")
f2 = open("flash.bin", "wb")
ins = [
    "add",
    "sub",
    "regwrite",
    "regmov",
    "ramw",
    "ramr",
    "jmp",
    "jpie",
    "jpig",
    "jpis",
    "inc",
    "dec",
    "nop",
    "jpine",
    "bitsleft",
    "halt",
    "bitsright",
    "call",
    "ret",
    "regswap",
    "mul",
    "div",
]
buff = ""
buffc1 = ""
buffc2 = ""
buffc3 = ""
f2.write((12).to_bytes(1, "big"))
n = 0
s = 0


def writ(buff):
    s = 0

    print("writ")
    for i in range(0, len(ins)):
        if ins[i] == buff:
            s = i
        # print(s)
    f2.write(s.to_bytes(1, "big"))
    print(s)


nextli = False
inse = 0
lablist = [
    "reg_k",
    "reg_l",
    "reg_j",
    "reg_i",
    "reg_a",
    "reg_b",
    "reg_c",
    "reg_d",
    "reg_e",
    "reg_f",
    "reg_g",
    "reg_h",
    "stdout",
    "pc",
    "ar",
    "iport",
    "oport",
    "flag",
    "call",
	"ramsize",
]
cordlist = [7, 8, 3, 2, 10, 11, 12, 4, 5, 6, 17, 18, 9, 0, 1, 13, 14, 15, 16, 19]
orn = 0
for i in f.readlines():
    n = 0
    if n >= len(i) - 1:
        nextli = True
        break
    while not (i[n] == " " or i[n] == "\n"):
        buff += i[n]
        n += 1
        # print(n,"f", inse)
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    # print(buff,"hello")
    # print(buff[len(buff)-2],"maamamaa,mamamammammamm")
    if buff[len(buff) - 1] == ":":
        lablist.append(buff.replace(":", ""))
        cordlist.append(orn)
        #f2.write((12).to_bytes(1, "big"))
        print((12))
        print(lablist, cordlist, buff)
    buff=""
f.close()
f = open("main.asm", "r")
for i in f.readlines():
    n = 0
    if n >= len(i) - 1:
        nextli = True
        break
    buff = ""
    buffc1 = ""
    buffc2 = ""
    buffc3 = ""
    buffc4 = ""
    n = 0
    nextli = False

    while not (i[n] == " " or i[n] == "\n"):
        buff += i[n]
        n += 1
        # print(n,"f", inse)
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    # print(buff,"hello")
    # print(buff[len(buff)-2],"maamamaa,mamamammammamm")
    if buff[len(buff) - 1] == ":":
        #lablist.append(buff.replace(":", ""))
        #cordlist.append(orn + 1)
        f2.write((12).to_bytes(1, "big"))
        print((12))
        print(lablist, cordlist)
    else:
        writ(buff)
        print(buff, "debug")
    n += 1
    if nextli:
        continue

    while not (i[n] == " " or i[n] == "\n"):
        buffc1 += i[n]
        n += 1
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    n += 1
    # print(buffc1, inse)
    if buffc1[0] == "0":
        f2.write(int(buffc1, 16).to_bytes(1, "big"))
    elif buffc1[0] == "'":
        f2.write(bytes(buffc1[1], "utf-8"))
    else:
        for h in range(0, len(lablist)):
            if buffc1 == lablist[h]:
                if h > 19:
                    f2.write((cordlist[h] - 1).to_bytes(2, "big"))
                else:
                    f2.write((cordlist[h]).to_bytes(1, "big"))
                print(buffc1, cordlist[h])

    if nextli:
        continue

    while not (i[n] == " " or i[n] == "\n"):
        buffc2 += i[n]
        n += 1
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    n += 1
    if buffc2[0] == "0":
        f2.write(int(buffc2, 16).to_bytes(1, "big"))
    elif buffc2[0] == "'":
        f2.write(bytes(buffc2[1], "utf-8"))
    else:
        for h in range(0, len(lablist)):
            if buffc2 == lablist[h]:
                if h > 19:
                    f2.write((cordlist[h] - 1).to_bytes(2, "big"))
                else:
                    f2.write((cordlist[h]).to_bytes(1, "big"))

                print(cordlist[h])
    print(buffc2)
    if nextli:
        continue
    while not (i[n] == " " or i[n] == "\n"):
        buffc3 += i[n]
        n += 1
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    n += 1
    # print(buffc3, inse)
    if buffc3[0] == "0":
        f2.write(int(buffc3, 16).to_bytes(1, "big"))
    elif buffc3[0] == "'":
        f2.write(bytes(buffc3[1], "utf-8"))
    else:
        for h in range(0, len(lablist)):
            if buffc3 == lablist[h]:
                if h > 19:
                    f2.write((cordlist[h] - 1).to_bytes(2, "big"))
                else:
                    f2.write((cordlist[h]).to_bytes(1, "big"))
                print(cordlist[h])
    print(buffc3)
    if nextli:
        continue

    while not (i[n] == " " or i[n] == "\n"):
        buffc4 += i[n]
        n += 1
        if n >= len(i) - 1:
            nextli = True
            break
    orn += 1
    n += 1
    # print(buffc4, inse)
    if buffc4[0] == "0":
        f2.write(int(buffc4, 16).to_bytes(1, "big"))
    elif buffc4[0] == "'":
        f2.write(bytes(buffc4[1], "utf-8"))
    else:
        for h in range(0, len(lablist)):
            if buffc4 == lablist[h]:
                if h > 19:
                    f2.write((cordlist[h] - 1).to_bytes(2, "big"))
                else:
                    f2.write((cordlist[h]).to_bytes(1, "big"))
                print(cordlist[h])
    print(buffc4)
    if nextli:
        continue
    inse += 1
    if nextli:
        continue
print((5).to_bytes(2, "big"))
f2.write((15).to_bytes(1, "big"))
f.close()
f2.close()

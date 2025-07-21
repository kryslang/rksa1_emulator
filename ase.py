f=open("main.asm","r")
f2=open("flash.bin","wb")
ins=["ADD","SUB","REGWRITE","REGMOV","RAMW","RAMR","JMP","JPIE","JPIG","JPIS","INC","DEC","NOP\n","JPINE","BITSLEFT","HALT\n","BITSRIGHT","CALL","RET","REGSWAP"]
buff=""
buffc1=""
buffc2=""
buffc3=""
f2.write((12).to_bytes(1,"big"))
n=0
s=0
def writ(buff):
	s=0
	n=0
	
	print("writ")
	for i in range(0,len(ins)):
		if(ins[i]==buff):
			s=i
		#print(s)
	f2.write(s.to_bytes(1,"big"))
		
nextli=False
inse=0
lablist=[]
cordlist=[]
orn=0
for i in f.readlines():
	n=0
	if(n>=len(i)):
		nextli=True
		break
	buff=""
	buffc1=""
	buffc2=""
	buffc3=""
	buffc4=""
	n=0
	nextli=False
	
	
	while i[n]!=" ":
		buff+=i[n]
		n+=1
		print(n,"f", inse)
		if(n>=len(i)):
			nextli=True
			break
	orn+=1
	print(buff,"hello")	
	#print(buff[len(buff)-2],"maamamaa,mamamammammamm")
	if(buff[len(buff)-2]==':'):
		lablist.append(buff.replace(":",""))
		cordlist.append(orn)
		f2.write((12).to_bytes(1,"big"))
		print(lablist,cordlist)
	else:
		writ(buff)
	n+=1
	if(nextli):
		continue
	
	while i[n]!=" ":
		buffc1+=i[n]
		n+=1
		if(n>=len(i)):
			nextli=True
			break
	orn+=1
	n+=1
	print(buffc1, inse)
	if(buffc1[0]=='0'):
		f2.write(int(buffc1,16).to_bytes(1, "big"))
	elif(buffc1[0]=="'"):
		f2.write(bytes(buffc2[1], "utf-8"))
	else:
		for h in range(0,len(lablist)):
			if(buffc1==lablist[h]):
				f2.write((cordlist[h]).to_bytes(1, "big"))
	if(nextli):
		continue
	
	while i[n]!=" ":
		buffc2+=i[n]
		n+=1
		if(n==len(i)):
			nextli=True
			break
	orn+=1
	n+=1
	if(buffc1[0]=='0'):
		f2.write(int(buffc1,16).to_bytes(1, "big"))
	elif(buffc1[0]=="'"):
		f2.write(bytes(buffc2[1], "utf-8"))
	else:
		for h in range(0,len(lablist)):
			if(buffc1==lablist[h]):
				f2.write((cordlist[h]).to_bytes(1, "big"))
	print(buff,"ahoj",buffc1, buffc2)
	if(nextli):
		continue
	while i[n]!=" ":
		buffc3+=i[n]
		n+=1
		if(n>=len(i)):
			nextli=True
			break
	orn+=1
	n+=1
	print(buffc3, inse)
	if(buffc1[0]=='0'):
		f2.write(int(buffc1,16).to_bytes(1, "big"))
	elif(buffc1[0]=="'"):
		f2.write(bytes(buffc2[1], "utf-8"))
	else:
		for h in range(0,len(lablist)):
			if(buffc1==lablist[h]):
				f2.write((cordlist[h]).to_bytes(1, "big"))
	if(nextli):
		continue
		
	while i[n]!=" ":
		buffc4+=i[n]
		n+=1
		if(n>=len(i)):
			nextli=True
			break
	orn+=1
	n+=1
	print(buffc4, inse)
	if(buffc1[0]=='0'):
		f2.write(int(buffc1,16).to_bytes(1, "big"))
	elif(buffc1[0]=="'"):
		f2.write(bytes(buffc2[1], "utf-8"))
	else:
		for h in range(0,len(lablist)):
			if(buffc1==lablist[h]):
				f2.write((cordlist[h]).to_bytes(1, "big"))
	if(nextli):
		continue
	inse+=1
	if(nextli):
		continue
	
f2.write((15).to_bytes(1,"big"))
f.close()
f2.close()

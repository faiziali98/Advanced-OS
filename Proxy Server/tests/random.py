# f = open('workfile', 'wb')
# f.write('0123456789abcdef')
# f.close()
# f = open('workfile', 'ab')
# f.seek(15)
# f.write('0123456789abcdef')
import os


sizes = {10000000,20000000,30000000,40000000,50000000,60000000,70000000}

for s in sizes:
	name = "./Test_files/"+str(s)+".txt"
	print name

	with open(name,'wb') as f:
		f.write('test')

	# strn = open(,'r').read()
	# for x in range(2,11):
	# 	with open('test'+str(x)+'.txt','wb') as f:
	# 		f.write(strn)

	while ((os.stat(name).st_size)<s):
		strg = open(name,'r').read()
		with open(name,'ab') as f:
			f.write(strg)
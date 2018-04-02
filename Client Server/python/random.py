# f = open('workfile', 'wb')
# f.write('0123456789abcdef')
# f.close()
# f = open('workfile', 'ab')
# f.seek(15)
# f.write('0123456789abcdef')
import os

# with open('test1.txt','wb') as f:
# 	f.write('test')

strn = open('test1.txt','r').read()
for x in range(2,11):
	with open('test'+str(x)+'.txt','wb') as f:
		f.write(strn)

# while ((os.stat('test1.txt').st_size)<10000000):
# 	str = open('test1.txt','r').read()
# 	with open('test1.txt','ab') as f:
# 		f.write(str)
import requests
import urllib2
import Queue
import os
import sys
from threading import Thread


def filedown(url, id):
	file_name = url.split('/')[-1]
	u = urllib2.urlopen(url)
	meta = u.info()
	rec = 0;

	file_size = int(meta.getheaders("Content-Length")[0])
	f = open('Files/'+str(id)+file_name.split('.')[0]+'.'+file_name.split('.')[1], 'wb')
	print "Downloading: %s Bytes: %s" % (file_name, file_size)

	try:
		while True:
		    buffer = u.read(8192)
		    if not buffer or buffer=="Error":
				print len(buffer)
		    	break
		    rec += len(buffer)
		    f.write(buffer)
	except:
		pass

	f.close()
	return rec

def doWork(url,id,num):
	total = 0;
	for n in range(1,num+1):
		total += filedown(url+str(n)+'.txt',id);
		print total

url = 'http://localhost:8080/Test_files/tes'
threads = []

for i in range(1, int(sys.argv[1])+1):
	t = Thread(target=doWork, args = (url,i,int(sys.argv[2])))
	threads.append(t);
	t.daemon = True
	t.start()

for t in threads:
	t.join()

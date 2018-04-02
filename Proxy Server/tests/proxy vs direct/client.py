import requests
import urllib2
import Queue
import os
import time
import subprocess
import sys
from threading import Thread


def filedown(url):
	file_name = url.split('/')[-1]

	proxy_support = urllib2.ProxyHandler({"http":"http://localhost:8081"})
	opener = urllib2.build_opener(proxy_support)
	urllib2.install_opener(opener)

	try:
		u = urllib2.urlopen(url)
		meta = u.info()
		rec = 0;

		file_size = int(meta.getheaders("Content-Length")[0])
		f = open('Files/'+file_name, 'wb')
		try:
			while True:
			    buffer = u.read(8192)
			    if not buffer:
					break
			    rec += len(buffer)
			    f.write(buffer)
		except:
			pass

		f.close()
		return rec
	except:
		return -1

url = 'http://localhost:8080/Test_files/'
subprocess.call('rm -rf Files', shell=True)
subprocess.call('mkdir Files', shell=True)
sizes = {10000000,20000000,30000000,40000000,50000000,60000000,70000000}
f = open('timeProxyLocal.txt','ab')

# for i in range(1, int(sys.argv[1])+1):
# 	t = Thread(target=doWork, args = (url,i,int(sys.argv[2])))
# 	threads.append(t);
# 	t.daemon = True
# 	t.start()

# for t in threads:
# 	t.join()

for s in sizes:
	print "downloading "+str(s)
	t0 = time.time()
	filedown(url+str(s)+".txt")
	t = time.time()-t0
	f.write(str(s)+' '+str(t)+'\n')
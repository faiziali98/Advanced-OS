import requests
import urllib2
import Queue
import time
import os
import subprocess
import sys
from threading import Thread


def filedown(url):
	file_name = url.split('/')[-1]

	proxy_support = urllib2.ProxyHandler({"http":"http://localhost:8081"})
	opener = urllib2.build_opener(proxy_support)
	urllib2.install_opener(opener)

	try:
		print "Getting File"
		u = urllib2.urlopen(url)
		meta = u.info()
		rec = 0;

		file_size = float(meta.getheaders("Content-Length")[0])
		splits = file_name.split('.')

		f = open('Files/'+splits[0]+'.'+splits[1], 'wb')
		try:
			while True:
			    buffer = u.read(8192)
			    if not buffer:
					break
			    rec += len(buffer)
			    f.write(buffer)
			    sys.stdout.write("Downloaded %d of %d bytes (%0.2f%%)\r" % 
       								(rec, file_size, (rec/file_size)*100))
		except:
			pass

		f.close()
		sys.stdout.write('\n')
		print "File received"
	except:
		return -1

url = 'http://localhost:8080/Test_files/test'
subprocess.call('rm -rf Files', shell=True)
subprocess.call('mkdir Files', shell=True)
filedown(url+'.txt')

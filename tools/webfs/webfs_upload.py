#!/usr/bin/env python3
# -*- coding: cp1251 -*-
'''
test webfs_upload.py
Created on 27/09/2015.

@author: pvvx
'''
#
# Use module 'requests' - http://docs.python-requests.org/
#
 
import sys
import requests 
from requests.auth import HTTPBasicAuth
 
 
if __name__ == '__main__':
	if len(sys.argv) == 2:
		if sys.argv[1] == '-h':
			print 'Usage: filename rtlurl username password'
			sys.exit(0)
	filename = './build/bin/WEBFiles.bin'
	rtlurl = 'http://rtl871x1/fsupload'
	username = 'RTL871X'
	password = '0123456789'

	if len(sys.argv) > 1: 
		if sys.argv[1]:
			filename = sys.argv[1]
	if len(sys.argv) > 2: 
		if sys.argv[2]:
			rtlurl = sys.argv[2]
	if len(sys.argv) > 3:
		if sys.argv[3]:
			username = sys.argv[3]
	if len(sys.argv) > 4:
		if sys.argv[4]:
			password = sys.argv[4]

	files = {'file': (filename, open(filename, 'rb'), 'application/octet-stream')}
#    postdata = {'':''}
#    headers = {'Connection' : 'keep-alive'}
    
#    q = s.post(theurl, auth=HTTPBasicAuth(username, password), headers=headers, data=postdata, files=files)
	s = requests.Session()
	q = s.post(rtlurl, auth=HTTPBasicAuth(username, password), files=files)
	if q.status_code <> 200:
		print 'Error Connection, HTTP code: %d' %q.status_code
		sys.exit(-1);
	else:
		print 'Webfs upload Ok, HTTP code: %d' %q.status_code
	sys.exit(0);

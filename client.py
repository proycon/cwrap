#!/usr/bin/env python

import urllib
import urllib2
import sys

URL = "http://localhost:8888"

def client(tweettext):
    #data = urllib.urlencode({'input': tweettext})
    headers = {"Content-type": "application/json", "Accept": "application/json"}
    request = urllib2.Request(URL, tweettext)
    response = urllib2.urlopen(request)
    return response.read()


for i, line in enumerate(iter(sys.stdin.readline, '')):
    line = line.strip()
    print "Sent     #" +str(i) + ": \"" + line + "\""
    print "Received #" + str(i) + ": \"" + client(line) + "\""



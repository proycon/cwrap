#!/usr/bin/env python

import urllib
import urllib2

URL = "http://localhost:8888"

def client(tweettext):
    #data = urllib.urlencode({'input': tweettext})
    headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
    request = urllib2.Request(URL, "Dit is een test")
    response = urllib2.urlopen(request)
    return response.read()

for i in range(0,1000000):
    print i, client("Dit is een test")



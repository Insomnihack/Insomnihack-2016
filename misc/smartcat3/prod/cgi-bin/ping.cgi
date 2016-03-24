#!/usr/bin/env python3
# encoding: utf-8

import cgi
import subprocess
import string
import os

# FWIW: The same vulnerability was found in the real world ;)
#       Devs have crazy ideas sometimes...

# The firewall only allows the strict minimum required for this chall, aka
# OUTPUT on udp/53 and icmp. INPUT is port 80 only.

def sanity_check(txt):
    charset = list(string.ascii_letters + string.digits + string.punctuation)

    for _ in r"$&\;`|*":
        charset.remove(_)

    return any(c not in charset for c in txt)

print("Content-type: text/html")
print
print("""
<html>
<head>
      <title>Can I haz Smart Cat ???</title>
      <link rel="stylesheet" href="//cdnjs.cloudflare.com/ajax/libs/highlight.js/9.1.0/styles/github.min.css">
      <script src="//cdnjs.cloudflare.com/ajax/libs/highlight.js/9.1.0/highlight.min.js"></script>
      <style>code { border: 1px solid black; padding: 10px; }</style>
</head>
<body>
  <h3> Smart Cat interface [<a href="ping.cgi?src=1">source</a>]</h3>
""")

form = cgi.FieldStorage()
dest = form.getvalue("dest", "127.0.0.1")
src = form.getvalue("src")


if sanity_check(dest):
    out = "Invalid characters!"
    color = "red"
else:
    # No env tricks this time, find something that is not CGI-specific :)

    os.environ.clear()
    try:
        status = subprocess.call("ping -c1 " + dest,
                                 timeout=2,
                                 shell=True,
                                 executable="/bin/bash",
                                 stdin=None,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
    except subprocess.TimeoutExpired:
        status = 1

    out = "Host is " + ("down" if status else "up")
    color = "red" if status else "green"

print("""
  <form method="post" action="ping.cgi">
    <p>Host to check: <input type="text" name="dest" placeholder="127.0.0.1" /></p>
  </form>

  <p>Status: <tt style="color: %s;">%s</tt></p>
  <img src="../img/cat.jpg"/><!-- grimmlin loves cats -->
""" % (color, cgi.escape(out)))

if src:
    with open(__file__, "rb") as f:
        print('<script>hljs.initHighlightingOnLoad();</script>')
        print('''<pre><code class="python hljs">%s</code></pre>''' % cgi.escape(f.read().decode()))

print("</body></html>")

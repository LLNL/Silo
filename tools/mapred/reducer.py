#!/usr/bin/env python
 
from operator import itemgetter
import sys
 
current_word = None
current_z = None
current_rgb = None
word = None
 
# input comes from STDIN
for line in sys.stdin:
    # remove leading and trailing whitespace
    line = line.strip()
 
    # parse the input we got from mapper.py
    word, pixval = line.split('\t', 1)
 
    rgbz = pixval.split('|')
    rgb = rgbz[0]
    z = rgbz[1]

    # convert count (currently a string) to int
    try:
        z = float(z)
    except ValueError:
        # count was not a number, so silently
        # ignore/discard this line
        continue
 
    # this IF-switch only works because Hadoop sorts map output
    # by key (here: word) before it is passed to the reducer
    if current_word == word:
        if z < current_z:
            current_z = z
            curent_rgb = rgb
    else:
        if current_word:
            # write result to STDOUT
            print '%s\t%s' % (current_word, current_rgb)
        current_word = word
        current_rgb = rgb 
        current_z = z 
 
# do not forget to output the last word if needed!
if current_word == word:
    print '%s\t%s' % (current_word, current_rgb)

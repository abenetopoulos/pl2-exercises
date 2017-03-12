#!/usr/bin/env python

#TODO: fix off-by-one error

import sys
import urllib
import httplib

def powers_of_two(x):
    powers = []
    i = 1
    p = 0
    while i <= x:
        if i & x:
            powers.append(p)
        i <<= 1
        p += 1
    return powers

if (len(sys.argv) < 2) :
    print "No webpage provided. Exiting..."
    exit(1)

webpageURL = sys.argv[1]
print webpageURL
openRes = urllib.urlopen(webpageURL)
questionString = "<span class=\"question\">"
for l in openRes.readlines():
    if questionString in l:
        targetString = l.split(">")
        targetNum = int(targetString[-3].split("<")[0])

powers = powers_of_two(targetNum)
width = 80
height = 20
befungeProg = [[' ' for x in range(width)] for y in range(height)]
currentRow = 0
currentColumn = 0

befungeProg[0][0] = "$"
befungeProg[0][1] = "!"
currentColumn = 2

prepend = False
prev = 0

for power in powers:
    temp = power
    if power > 0 :
        power -= prev
        befungeProg[currentRow][currentColumn] = ":"
        currentColumn += 1

        while power > 0 :
            if (not prepend and currentColumn == width - 1) or (prepend and currentColumn == 0):
                if prepend:
                    firstChar = ">"
                else:
                    firstChar = "<"

                befungeProg[currentRow][currentColumn] = "v"
                befungeProg[currentRow + 1][currentColumn] = firstChar
                currentRow += 1

                prepend = not prepend

            if prepend:
                befungeProg[currentRow][currentColumn] = "+"
                befungeProg[currentRow][currentColumn - 1] = ":"
                currentColumn -= 2
            else:
                befungeProg[currentRow][currentColumn] = ":"
                befungeProg[currentRow][currentColumn + 1] = "+"
                currentColumn += 2

            power -= 1

    prev = temp

while True :
    if prepend:
        if currentColumn - 4 >= 0 :
            befungeProg[currentRow][currentColumn] = "v"
            befungeProg[currentRow][currentColumn - 3] = "\\"
            befungeProg[currentRow][currentColumn - 4] = ">"
            befungeProg[currentRow + 1][currentColumn] = "<"
            befungeProg[currentRow + 1][currentColumn - 1] = "+"
            befungeProg[currentRow + 1][currentColumn - 2] = "\\"
            befungeProg[currentRow + 1][currentColumn - 3] = ":"
            befungeProg[currentRow + 1][currentColumn - 4] = "|"
            befungeProg[currentRow + 2][currentColumn - 4] = ">"
            befungeProg[currentRow + 2][currentColumn - 3] = "$"
            currentRow += 2
            currentColumn -= 2
            prepend = False
            break
        else :
            prepend = False
    else:
        if currentColumn + 4 < width :
            befungeProg[currentRow][currentColumn] = "v"
            befungeProg[currentRow][currentColumn + 3] = "\\"
            befungeProg[currentRow][currentColumn + 4] = "<"
            befungeProg[currentRow + 1][currentColumn] = ">"
            befungeProg[currentRow + 1][currentColumn + 1] = "+"
            befungeProg[currentRow + 1][currentColumn + 2] = "\\"
            befungeProg[currentRow + 1][currentColumn + 3] = ":"
            befungeProg[currentRow + 1][currentColumn + 4] = "|"
            befungeProg[currentRow + 2][currentColumn + 4] = "<"
            befungeProg[currentRow + 2][currentColumn + 3] = "$"
            currentRow += 2
            currentColumn += 2
            prepend = True
            break
        else :
            prepend = True

if prepend:
    befungeProg[currentRow][currentColumn] = '.'
    befungeProg[currentRow][currentColumn - 1] = '@'
else:
    befungeProg[currentRow][currentColumn] = '.'
    befungeProg[currentRow][currentColumn + 1] = '@'

for line in befungeProg:
    print ''.join(line)

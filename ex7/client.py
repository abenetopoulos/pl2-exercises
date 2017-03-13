#!/usr/bin/env python

#TODO:
# - try to compress generation of powers of two

import sys
import requests

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

ses = requests.Session()

questionString = "<span class=\"question\">"
correctString = "<p class=\"right\">"
wrongString = "<p class=\"wrong\">"
width = 85
height = 20

for i in xrange(11):
    r = ses.get(webpageURL)
    cookies = ses.cookies
    openRes = r.content

    for l in openRes.split('\n'):
        if questionString in l:
            targetString = l.split(">")
            targetNum = int(targetString[-3].split("<")[0])
            break

    print 'Round ' + str(i) + ', number ' + str(targetNum)
    powers = powers_of_two(targetNum)
    befungeProg = [[' ' for x in range(width)] for y in range(height)]
    currentRow = 0
    currentColumn = 0

    befungeProg[0][0] = "$"
    befungeProg[0][1] = "!"
    currentColumn = 2

    prepend = False
    prev = 0

    haveRemovedOne = False

    for power in powers:
        temp = power
        if power > 0 :
            power -= prev
            befungeProg[currentRow][currentColumn] = ":"
            currentColumn += 1

            if (powers[0] != 0 and not haveRemovedOne):
                befungeProg[currentRow][currentColumn] = "\\"
                befungeProg[currentRow][currentColumn + 1] = "$"
                currentColumn += 2
                haveRemovedOne = True

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

    programString = ""

    for line in befungeProg:
        if len(''.join(line).rstrip()) > 0 :
            programString = programString + ''.join(line).rstrip() + "\r\n"

    print 'Submitted solution:'
    print programString

    payload = {'submit' : "Submit", 'program' : programString[:-2]}
    resp = ses.post(webpageURL, data = payload)

    for l in resp.content.split('\n'):
        if correctString in l or wrongString in l:
            print l.split('>')[1].split('<')[0]
            break

    payload = {'again' : "again"}
    resp = ses.post(webpageURL, data = payload)

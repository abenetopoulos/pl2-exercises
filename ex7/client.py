#!/usr/bin/env python

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

def get_next_position(befungeProg, currentRow, currentColumn, prepend) :
    if (not prepend and currentColumn == width - 2) or (prepend and currentColumn == 1):
        if prepend:
            befungeProg[currentRow][currentColumn - 1] = "v"
            befungeProg[currentRow + 1][currentColumn - 1] = ">"
            currentColumn -= 1
        else:
            befungeProg[currentRow][currentColumn + 1] = "v"
            befungeProg[currentRow + 1][currentColumn + 1] = "<"
            currentColumn += 1

        currentRow += 1

        prepend = not prepend

    if prepend :
        currentColumn -= 1
    else:
        currentColumn += 1

    return prepend, currentRow, currentColumn

for i in xrange(10):
    r = ses.get(webpageURL)
    ##cookies = ses.cookies
    openRes = r.content

    for l in openRes.split('\n'):
        if questionString in l:
            targetString = l.split(">")
            targetNum = int(targetString[-3].split("<")[0])
            break

    print 'Round ' + str(i + 1) + ', number ' + str(targetNum)
    powers = powers_of_two(targetNum)
    befungeProg = [[' ' for x in range(width)] for y in range(height)]
    currentRow = 0
    currentColumn = 0

    befungeProg[0][0] = "$"
    befungeProg[0][1] = "!"
    currentColumn = 1
    prepend = False

    prev = 0

    haveRemovedOne = False

    for power in powers:
        temp = power
        if power > 0 :
            power -= prev
            prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
            befungeProg[currentRow][currentColumn] = ":"

            if (powers[0] != 0 and not haveRemovedOne):
                prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
                befungeProg[currentRow][currentColumn] = "\\"
                prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
                befungeProg[currentRow][currentColumn] = "$"
                haveRemovedOne = True

            while power > 0 :
                prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
                befungeProg[currentRow][currentColumn] = ":"
                prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
                befungeProg[currentRow][currentColumn] = "+"

                power -= 1

        prev = temp

    for i in xrange(len(powers)):
        prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
        befungeProg[currentRow][currentColumn] = "+"

    prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
    befungeProg[currentRow][currentColumn] = '.'
    prepend, currentRow, currentColumn = get_next_position(befungeProg, currentRow, currentColumn, prepend)
    befungeProg[currentRow][currentColumn] = '@'

    programString = ""

    for line in befungeProg:
        if len(''.join(line).rstrip()) > 0 :
            programString = programString + ''.join(line).rstrip() + "\r\n"

    print 'Submitted solution:'
    print programString[:-2]

    payload = {'submit' : "Submit", 'program' : programString[:-2]}
    resp = ses.post(webpageURL, data = payload)

    for l in resp.content.split('\n'):
        if correctString in l or wrongString in l:
            print l.split('>')[1].split('<')[0]
            break

    payload = {'again' : "again"}
    resp = ses.post(webpageURL, data = payload)

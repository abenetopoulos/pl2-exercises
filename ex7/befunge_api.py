#!/usr/bin/env python

from flask import Flask, request, Response
import subprocess
from time import sleep
import string
import os

app = Flask(__name__)

def execute_program(program):
    filename = ".temp"
    tmp = open(filename, "w")
    tmp.write(program)
    tmp.close()

    proc = subprocess.Popen(["./vm", ".temp"], stdout=subprocess.PIPE)

    for i in xrange(10):
        sleep(0.1) #NOTE: ugh...

        if (proc.poll() != None):
            res, err = proc.communicate()
            os.remove(filename)
            return res
    
    proc.kill()
    os.remove(filename)
    return "TIMEOUT"

def check_program(program):
    bannedChars = set(string.digits + '"' + 'p' + 'g' + '&' + '~')

    for c in program:
        if c in bannedChars:
            return False

    return True

@app.route("/befunge_api/", methods = ['POST'])
def run_befunge_prog():
    print request.method
    program = request.data
    url = request.full_path

    programValid = True
    if (url.split('?')[1] == "restrict"):
        programValid = check_program(program)

    if (programValid):
        res = execute_program(program)
        if res == "TIMEOUT":
            return Response(response = res, status = '400')
        else:
            return Response(response = str(res), status = '200')
    else :
        return Response(response = '-1', status = '400')

if __name__ == "__main__":
    app.run()

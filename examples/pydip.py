#!/usr/bin/env python3
# Interactive PyDIP session. Pre-imports PyDIP as dip.
# Allows interactive plotting using Show()
import threading, code, time
import PyDIP as dip
import PyDIP.viewer as dipview

done = False

def replThread():
    global done
    code.interact(local={'dip':dip,'Show':dipview.Show})
    done = True
    
thread = threading.Thread(target=replThread)
thread.start()

while not done:
    dipview.Draw()
    time.sleep(0.001)

thread.join()

dipview.Spin()

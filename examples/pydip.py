#!/usr/bin/env python3
# Interactive PyDIP session. Pre-imports PyDIP as dip.
# Allows interactive plotting using Show()
import threading, code, time
import PyDIP as dip
import PyDIP.viewer as dipview

def replThread():
    code.interact(local={'dip':dip,'Show':dipview.Show})
    
thread = threading.Thread(target=replThread)
thread.start()

while thread.is_alive():
    dipview.Draw()
    time.sleep(0.01)

thread.join()

dipview.Spin()

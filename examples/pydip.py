#!/usr/bin/env python3

# Interactive PyDIP session. Pre-imports PyDIP as dip.
# Allows interactive plotting using Show()
# You will need to close the windows when exiting the interactive
# session (either before or after ^D, doesn't matter).

import sys, os, threading, code, time

# Modify the path here to point to where you installed PyDIP:
# TODO: we need to install PyDIP to a place where Python will find it.
dipdir = os.getcwd() + '/../target/dip/lib'
sys.path.append( dipdir )

import PyDIP as dip


def replThread():
   code.interact( local={ 'dip': dip, 'Show': dip.viewer.Show } )


thread = threading.Thread( target=replThread )
thread.start()

while thread.is_alive():
   dip.viewer.Draw()
   time.sleep( 0.01 )

thread.join()

dip.viewer.CloseAll()

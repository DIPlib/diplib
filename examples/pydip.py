#!/usr/bin/env python3

# Interactive PyDIP session. Pre-imports PyDIP as dip.
# Allows interactive plotting using Show()
# You will need to close the windows when exiting the interactive
# session (either before or after ^D, doesn't matter).

import threading, code, time
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

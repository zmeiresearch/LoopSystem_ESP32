#!/usr/bin/env python3
"""Git revision and working tree state"""

import subprocess

REVISION = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()

#if subprocess.check_output(["git", "diff", "--stat"]).strip() == "":
#    STATE = "clean"
#else:
#    STATE = "dirty"


print("-D BUILD_REV='\"{}\"'".format(REVISION.decode("UTF-8")))
#print("-D SRC_STATE='\"{}\"'".format(STATE))


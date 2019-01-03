#!/usr/bin/python

import compileall

compileall.compile_dir(r'./', force=True, legacy=True)

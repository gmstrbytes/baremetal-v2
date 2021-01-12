# houdini.py
# Copyright (c) 2020 J. M. Spivey

# My first Python program.  Don't look.  Really don't! -- Mike

# Python has a config file parsing library, but it just won't do for
# this task because of minor syntax variations.

import sys
import re

# A global database of config file entries (why not?)
dbase = dict()
# The usual dict objects preserve the order keys were added, which is
# helpful for us.

def setopt(section, key, value):
    """Set an option, creating the section if needed."""
    global dbase
    if not (section in dbase): dbase[section] = dict()
    dbase[section][key] = value

def save(fp):
    """Save the database on a file."""
    global dbase
    vierge = True
    for s, d in dbase.items():
        if not vierge: fp.write("\n")
        vierge = False
        fp.write(f"[{s}]\n")
        for k, v in d.items():
            fp.write(f"{k}={v}\n")

def rematch(re, line):
    """Match a regexp and save the result."""
    global mdata
    mdata = re.match(line)
    return mdata

def regroup(n):
    """Retrieve group from previous match."""
    global mdata
    return mdata.group(n)

def parse(fp):
    """Parse a config file and add its content to the database"""

    # These are rather strict, but adequate for the task
    sechdr = re.compile(r"^\[([^]]+)\]$")
    kvline = re.compile(r"^([^=]+)=(.*)$")
    comment = re.compile(r"^[ \t]*(#.*)?$")
    
    cursec = None

    for line in fp:
        if rematch(sechdr, line):
            # A [section] line.  No support for keys before the first section.
            cursec = regroup(1)
        elif rematch(kvline, line):
            # A key=value line.  No spaces around key, but
            # spaces ARE significant before and after value
            key = regroup(1)
            value = regroup(2)
            if cursec == None:
                print("Anon section not allowed"); exit(1)
            setopt(cursec, key, value)
        elif rematch(comment, line):
            # A blank or comment line.  Comments are lost, alas.
            pass
        else:
            print("Syntax error:", line, file=sys.stderr)
            exit(1)

args = sys.argv[1:]

if not ((len(args) == 4 and args[0] == "get")
        or (len(args) == 5 and args[0] == "set")):
    print("Usage: houdini (get|set) file.conf section option [value]",
          file=sys.stderr); exit(1)

cmd = args[0]; fname = args[1];
section = args[2]; option = args[3]
if args[0] == "set": value = args[4]

try:
    with open(fname) as fp: parse(fp)
except FileNotFoundError:
    # Non-existent file is treated as empty
    pass

if cmd == "get":
    # houdini get file.conf section option
    if (section in dbase) and (option in dbase[section]):
        print(dbase[section][option])

if cmd == "set":
    # houdini set file.conf section option value
    setopt(section, option, value)
    with open(fname, 'w') as fp: save(fp)


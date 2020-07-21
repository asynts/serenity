#!/usr/bin/env python3

import pathlib

def collect_filenames(patterns, prefixes):
    filenames = []
    for pattern in patterns:
        filenames.extend(str(path) for path in pathlib.Path(".").rglob(pattern))

    def prefix_filter(filename):
        for prefix in prefixes:
            if filename.startswith(prefix):
                return False
        return True
    filenames = list(filter(prefix_filter, filenames))

    return set(filenames)

filenames = collect_filenames(["*.cpp", "*.h"], ["Toolchain", "Build"])

def replace(before, after):
    for filename in filenames:
        with open(filename, "r+") as fp:
            data = fp.read()
            data = data.replace(before, after)
            fp.seek(0)
            fp.write(data)
            fp.truncate()

def read_multiline(prompt=None):
    if prompt:
        print(prompt)
    
    contents = []
    while True:
        try:
            line = input()
        except EOFError:
            break
        contents.append(line)
    return "\n".join(contents)

while True:
    try:
        before = read_multiline("BEFORE:")
        after = read_multiline("AFTER:")

        if len(before) == 0 or len(after) == 0:
            print("IGNORE")
            continue
        replace(before, after)
    except KeyboardInterrupt:
        print()
        break

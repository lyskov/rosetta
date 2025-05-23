#!/usr/bin/env python
"""update_rdkit.py - Alter the Rosetta distribution for and updated RDKit

Run this program in the main/source/external/rdkit directory

Note that you'll probably have to build RDKit once (or at least run CMake within the RDKit repo)
in order to generate certain required files.

Also, it's possible you may need to update the Rosetta Boost library to include
new libraries that RDKit requires. (See comments in extract_boost_incl.py for details)

Author: Rocco Moretti (rmorettiase@gmail.com)
"""

from __future__ import print_function

import sys, os
import shutil

"""
Details:

The gist of this script is to run through the CMakelists.txt files in the RDKit
distribution, scrape out the C/C++ files. (We do this to avoid getting any files
which aren't actually compiled.) We also Scrape all the header files. Headers &
Source are then copied into the Rosetta directory. This also involves re-writing
some include directives in the files, adjusting them such that the direcory
names now match. We also copy over the data files, and some other files which
aren't recoginized by the simple CMakelists.txt parsing and dir scan. Finally,
we write out a rdkit.external.settings file from the sources list, such that it
can be compiled by the Rosetta scons compiler.
"""

### These directories have issues with the automated scripts,
### and aren't used by the rest of RDKit or (currently) parts of Rosetta
### Feel free to re-enable them, but note you may need to debug the compilation issues.

IGNORED_DIRECTORIES = [ 'GraphMol/CIPLabeler','GraphMol/RGroupDecomposition' ]

def get_cmakes(directory):
    cmakelists = []
    for (dirpath, dirnames, filenames) in os.walk( directory+"/Code" ):
        if "CMakeLists.txt" in filenames:
            cmakelists.append( dirpath )
    return cmakelists

def get_headers(directory):
    """Get all the headers in the tree, not just the ones listed in the
    CMake file. (Which only list the "public" ones.)"""
    headers = []
    for (dirpath, dirnames, filenames) in os.walk( directory+"/Code" ):
        headers.extend( dirpath + "/" + fn for fn in filenames
                     if (fn.endswith(".h") or fn.endswith(".hpp") ) )
    return headers

def get_next_line(string):
    split = string.split("\n",1)
    if len(split) == 0:
        return "", ""
    elif len(split) == 1:
        return split[0], ""
    else:
        return split[0], "\n".join(split[1:])

def parse_cmake(directory):
    with open(directory + "/CMakeLists.txt") as f:
        contents = f.read()
    headers = []
    sources = []
    while len(contents):
        line, contents = get_next_line(contents)
        line = line.strip()
        if not line.startswith("rdkit_library") and not line.startswith("rdkit_headers"):
            continue
        #Need to join lines until we get a closing parenthesis
        while line.find(")") == -1 and len(contents) != 0:
            nextline, contents = get_next_line(contents)
            line = line + " " + nextline
        entries = line[ line.find("(")+1:line.find(")") ].split()
        if line.startswith("rdkit_library"):
            end = len(entries)
            if "LINK_LIBRARIES" in entries:
                end = entries.index("LINK_LIBRARIES")
            if "SHARED" in entries:
                end = entries.index("SHARED")
            sources.extend( entries[1:end] )
        if line.startswith("rdkit_headers"):
            end = len(entries)
            if "DEST" in entries:
                end = entries.index("DEST")
            headers.extend( entries[:end] )
    headers = [ directory + "/" + h for h in headers]
    sources = [ directory + "/" + s for s in sources]
    return headers, sources

def parse_files(directory):
    cmakedirs = get_cmakes(directory)
    # headers = []
    sources = []
    for cmakedir in cmakedirs:
        if any( cmakedir.endswith( p ) for p in IGNORED_DIRECTORIES ): continue
        if cmakedir.endswith("/Wrap"): continue # Python wrapping dirs, not needed for C++ library
        if cmakedir.endswith("/Qt") or cmakedir.endswith("/Qt/Demo"): continue # Qt specific code, not needed for Rosetta library
        print(cmakedir)
        hs, cs = parse_cmake(cmakedir)
        # headers.extend(hs)
        sources.extend(cs)

    headers = get_headers(directory)

    #Now remove some know bad entries which are handled manually.
    cleaned_headers = []
    cleaned_sources = []
    for h in headers:
        cleaned_headers.append(h)
    for s in sources:
        if "RDBoost" in s: continue # Python wrapper code
        if "SLNParse" in s: continue # Alternate parser - not needed, requires regex
        if s.endswith("SLNParse/${BISON_OUTPUT_FILES}"): continue
        if s.endswith("SLNParse/${FLEX_OUTPUT_FILES}"): continue
        if s.endswith("SmilesParse/${BISON_OUTPUT_FILES}"): continue
        if s.endswith("SmilesParse/${FLEX_OUTPUT_FILES}"): continue
        cleaned_sources.append(s)

    return [ fn[len(directory)+1:] for fn in cleaned_headers ], [ fn[len(directory)+1:] for fn in cleaned_sources ]


def add_extras(headers, sources, directory):
    '''Extra files generated by CMake or missed by the copy process.'''
    headers = headers + [
        'Code/GraphMol/SmilesParse/smarts.tab.hpp',
        'Code/GraphMol/SmilesParse/smiles.tab.hpp',
    ]
    sources = sources + [
        'Code/GraphMol/SmilesParse/lex.yysmarts.cpp',
        'Code/GraphMol/SmilesParse/lex.yysmiles.cpp',
        'Code/GraphMol/SmilesParse/smarts.tab.cpp',
        'Code/GraphMol/SmilesParse/smiles.tab.cpp',
    ]
    return headers, sources

def link_subdirs(headers, sources, directory):
    dirs = set()
    for f in headers + sources:
        fsplit = f.split('/')
        if ( len(fsplit) < 3 ):
            print("Error finding subdirectory of file", f)
            continue
        dirs.add(fsplit[1])

    for d in dirs:
        if not os.path.exists(d):
            os.symlink(directory+'/Code/'+d, d)

    # Extra links
    extra = [ ('External/rapidjson-1.1.0/include/rapidjson', 'rapidjson') ]
    for src, dest in extra:
        if not os.path.exists(dest):
            os.symlink(directory+'/'+src, dest)

def make_scons_file(sources,basedir):
    trunc_sources = [ fn[len("Code/"):] for fn in sources ]
    entries = {}
    for src in trunc_sources:
        srcdir, filename = os.path.split(src)
        name, ext = os.path.splitext(filename)
        if ext not in (".c",".cc",".cpp"):
            print( "WARNING: Non-standard extension on source", src, "-Ignoring" )
        else:
            entries.setdefault(srcdir.strip("/"), []).append(filename)

    with open( "../rdkit.external.settings", "w" ) as f:
        f.write(
'''# -*- mode:python;indent-tabs-mode:t;show-trailing-whitespace:t; -*-
#
# Project settings for rosetta sources
# (c) Copyright Rosetta Commons Member Institutions.
# (c) This file is part of the Rosetta software suite and is made available under license.
# (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
# (c) For more information, see http://www.rosettacommons.org. Questions about this can be
# (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

# THIS IS AN AUTOGENERATED FILE. You can edit it, but
# please note any manual alterations in the main/source/external/update_rdkit.py script

sources = {
''')

        for key in sorted( entries.keys() ):
            f.write('\t"rdkit/' +key+ '" :[\n')
            for name in sorted(entries[key]):
                f.write('\t\t"' +name+ '",\n')
            f.write('\t],\n')

        f.write(
'''
}
include_path = [
]
library_path = [
]
libraries = [
]
subprojects = [
]
''')

######## end of make_scons_file() function

def main():
    directory = "rdkit.upstream"
    headers, sources = parse_files(directory)
    headers, sources = add_extras(headers, sources, directory)
    link_subdirs( headers, sources, directory )
    make_scons_file(sources, directory)

if __name__ == "__main__":
    if len(sys.argv) != 1:
        print( __doc__ )
        exit()
    else:
        main()

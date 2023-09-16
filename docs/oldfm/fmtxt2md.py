import re, itertools

#
# Map section headings to markdown file names
#
fileNameFromSec = {
    "Error Handling and Global Library Behavior": "globals",
    "Files and File Structure": "files",
    "Meshes, Variables and Materials": "objects",
    "Multi-Block Objects and Parallel I/O": "parallel",
    "Part Assemblies, AMR, Slide Surfaces,": "subsets",
    "Calculational and Utility": "utility",
    "Optlists": "optlists",
    "User Defined (Generic) Data and Objects": "generic",
    "JSON Interface to Silo Objects": "json",
    "Previously Undocumented Use Conventions": "conventions",
    "Fortran Interface": "fortran",
    "Python Interface": "python",
    "Deprecated Functions": "deprecated",
    "Silo Library Header File": "header"
}

# 
# Get all documentation lines from text file
#
def GetFileLines(fname):
    with open(fname) as txtfile:
        return txtfile.readlines()

def IsMethodHeader(i, lines):
    if i < len(lines)-1 and re.search(r'^—', lines[i+1]):
        return True
    return False

def IsAPISectionHeader(line):
    if re.search(r'^[0-9]*\s*API Section', line):
        return True
    return False

def FormatText(s, kwnames=[]):

    s = re.sub(r'“',r'"',s) # left double curly/smart
    s = re.sub(r'”',r'"',s) # right double curly/smart
    s = re.sub(r'‘',r"'",s) # left single curly/smart
    s = re.sub(r'’',r"'",s) # right single curly/smart
    s = re.sub(r'—',r'-',s) # emdash

    # Backtick any keyword words
    for kw in kwnames:
        s = re.sub(r'(\s+)%s(\s+|[\.?!])'%kw,r'\1`%s`\2'%kw,s)

    # Backtick any DBxxx... words
    s = re.sub(r'(\s+)DB([a-zA-Z0-9_]{3,})(\(\))?(\s+|[\.?!])',r'\1`DB\2\3`\4',s)

    # Backtick any all caps words
    s = re.sub(r'(\s+)([A-Z0-9_]{3,})(\s+|[\.?!])',r'\1`\2`\3',s)

    # Undo all-caps backticking for some special cases
    s = re.sub(r'`HDF(4|5)`',r'HDF\1',s)
    s = re.sub(r'`PDB`',r'PDB',s)
    s = re.sub(r'`NOT`',r'**not**',s)

    return s

#
# Handle various issues with typesetting an output line
#
def OutputLineWithFormatting(mdfile_lines, line, kwnames=[], indent=""):

    # Break text ending at elipsis, dot, question mark, exclamation point followed by
    line = re.sub(r'\(e\.g\. ',r'@e@g@@',line)
    line = re.sub(r'([a-zA-Z0-9_)]\.\.\.|[a-zA-Z0-9_)]\.|[a-zA-Z0-9_)]\?|[a-zA-Z0-9_)]!)\s+',r'\1\n',line).strip()
    line = re.sub(r'@e@g@@',r'(e.g. ',line)
    sentences = line.split('\n')

    for s in sentences:

        s = FormatText(s, kwnames)

        mdfile_lines.append(indent + s.strip())
        mdfile_lines.append("\n")

    mdfile_lines.append("\n")

#
# Begin a new API section (new md file)
#
def StartNewSection(i, s, lines, mdfiles):
    mdfile_lines = []
    hs = re.search(r'^[0-9]*\s*API Section\s*(.*)',lines[i])
    if len(hs.groups()) > 0:
        hdr = "%s"%FormatText(hs.groups()[0]).strip()
    else:
        hdr = "Section %d"%s
    sec = "API Section"
    mdfiles[hdr] = {sec:[]}
    s += 1
    mdfiles[hdr][sec].append("## %s\n"%hdr)
    mdfiles[hdr][sec].append("\n")
    i += 1

    while i < len(lines) and not IsMethodHeader(i, lines):
        OutputLineWithFormatting(mdfiles[hdr][sec], lines[i])
        i += 1

    return i, s, hdr

def ProcessSynopsis(mdfile_lines, i, lines):
    i += 1
    mdfile_lines.append("* **C Signature:**\n\n") # FIXME REMOVE THIS
    mdfile_lines.append("  ```\n")
    mdfile_lines.append('  ' + lines[i].strip())
    mdfile_lines.append("\n")
    i += 1
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         re.search(r'^Fortran Equivalent:$', lines[i]) or \
         re.search(r'^Returns:$', lines[i]) or \
         re.search(r'^Description:$', lines[i]) or \
         re.search(r'^Arguments:$', lines[i])):
        mdfile_lines.append('      ' + lines[i].strip())
        mdfile_lines.append("\n")
        i += 1
    mdfile_lines.append("  ```\n")
    if re.search(r'^Fortran Equivalent:$', lines[i]):
        mdfile_lines.append("\n")
        i += 1
        if re.search(r'^None$', lines[i]):
            mdfile_lines.append("* **Fortran Signature:**\n\n")
            mdfile_lines.append("  ```\n")
            mdfile_lines.append("  None\n")
            mdfile_lines.append("  ```\n")
            i += 1
        else:
            mdfile_lines.append("* **Fortran Signature:**\n\n")
            mdfile_lines.append("  ```\n")
            fort_args = []
            fort_args_done = False
            while i < len(lines) and not \
                (IsMethodHeader(i, lines) or \
                 re.search(r'^Returns:$', lines[i]) or \
                 re.search(r'^Arguments:$', lines[i])):
                fort_args += [w.strip(' \n') for w in lines[i].split(',') if len(w.strip(' \n'))>0]
                if not fort_args_done and re.search(r'[ \t,a-zA-Z0-9_]*\)$', lines[i]):
                    n = 4
                    ident = "  "
                    while len(fort_args):
                        mdfile_lines.append(ident + ', '.join(fort_args[:n]))
                        if fort_args[:n][-1][-1] != ')':
                            mdfile_lines.append(",")
                        mdfile_lines.append("\n")
                        fort_args = fort_args[n:]
                        n = 6
                        ident = "     "
                    fort_args = []
                    fort_args_done = True
                    mdfile_lines.append("  ```\n\n")
                elif fort_args_done:
                    mdfile_lines.append('  ' + FormatText(lines[i].strip(' \n')))
                    mdfile_lines.append("\n")
                i += 1
    mdfile_lines.append("\n")
    return i

def ProcessArgumentListBlock(mdfile_lines, i, lines):
    i += 1
    args = []
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         IsAPISectionHeader(lines[i]) or \
         re.search(r'^Returns:$', lines[i]) or \
         re.search(r'^Description:$', lines[i])):
        if lines[i].strip() != '':
            args += [lines[i]]
        i += 1
    if len(args) < 2:
        mdfile_lines.append("* **Arguments:**\n\n")
        mdfile_lines.append("  `None`\n")
        return i, []
    mdfile_lines.append("* **Arguments:**\n\n")
    if len(args) % 2 != 0:
        print("***ARGS PROBLEM***\n")
        print(args)
    # use non-breaking spaces to enforce min table width for args column
    mdfile_lines.append("  Arg&nbsp;name | Description\n")
    mdfile_lines.append("  :---|:---\n")
    argnames = [args[2*j].strip() for j in range(int(len(args)/2))]
    j = 0
    while j < len(args):
        mdfile_lines.append("  `%s` | %s\n"%(args[j].strip(), FormatText(args[j+1].strip(),argnames) if j+1<len(args) else "ARGS PROBLEM"))
        j += 2
    mdfile_lines.append("\n\n")
    return i, argnames

#https://github.com/LLNL/Silo/blob/mcm86-27jan22-convert-docs-fm2md/src/silo/silo.h.in#L802
#https://github.com/LLNL/Silo/blob/main/src/silo/silo.h.in#L802

def ProcessReturnBlock(mdfile_lines, i, lines):
    i += 1
    retlines = []
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         IsAPISectionHeader(lines[i]) or \
         re.search(r'^Description:$', lines[i])):
        if lines[i].strip() != '' and lines[i].strip().lower()[:4] != 'none':
            retlines += [lines[i]]
        i += 1
    mdfile_lines.append("* **Returned value:**\n\n")
    if len(retlines):
        j = 0
        while j < len(retlines):
            OutputLineWithFormatting(mdfile_lines,retlines[j],[],"  ")
            j += 1
    else:
        mdfile_lines.append("  void");
    mdfile_lines.append("\n\n")
    return i

def ReadLinesInTabBlock(target_ntabs, i, desclines):

    complete_line = ''
    while complete_line.count('\t') < target_ntabs and i < len(desclines):
        complete_line += desclines[i]
        i += 1
    if complete_line.count('\t') != target_ntabs:
        print("Bad Case")
    complete_line = complete_line.strip()
    complete_line = re.sub("\n","<br>", complete_line)
    return i, [complete_line.split('\t')]

def OutputGatheredColumnsAsTable(mdfile_lines, cols, argnames=[], tabtitle=""):

    cols = list(map(list, itertools.zip_longest(*cols, fillvalue=None)))
    mdfile_lines.append("  **%s**\n\n"%(tabtitle+":" if tabtitle else "&nbsp;"))
    mdfile_lines.append('  ' + '|'.join(cols[0]))
    mdfile_lines.append("\n")
    mdfile_lines.append('  ' + ":---|"*(len(cols[0])-1))
    mdfile_lines.append(":---\n")

    for c in cols[1:]:
        c = [x if x else '' for x in c]
        mdfile_lines.append('  ' + FormatText('|'.join(c),argnames)) # FIXME
        mdfile_lines.append("\n")

    mdfile_lines.append("\n\n")

def ProcessDescription(mdfile_lines, i, lines, argnames=[]):

    i += 1
    desclines = []
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        desclines += [lines[i]]
        i += 1

    if not desclines:
        return i

    mdfile_lines.append("* **Description:**\n\n")

    j = 0
    target_ntabs = 0
    tabcols = []
    first_table = True
    while j < len(desclines):

        if target_ntabs == 0:

            if desclines[j].count('\t') == 0:

                # Output a normal "line" (which may be multiple sentences)
                # in description
                OutputLineWithFormatting(mdfile_lines, desclines[j], argnames, "  ")
                j += 1

            else:

                # Begin capturing a new set of lines for current table
                target_ntabs = desclines[j].count('\t')
                tabcols = [desclines[j].strip().split('\t')]
                j += 1

        else:

            if desclines[j].count('\t') == 0:

                # We've come to the end of current tab block.
                # First, output the current table.
                tabtitle = ""
                if first_table and argnames[-1] == 'optlist':
                    tabtitle = "Optlist options"
                OutputGatheredColumnsAsTable(mdfile_lines, tabcols, argnames, tabtitle)
                first_table = False
                tabcols = []

                # Indicate we're done with current table
                target_ntabs = 0

                # Now, output the current line
                OutputLineWithFormatting(mdfile_lines, desclines[j], argnames, "  ")
                j += 1

            else:

                # gather another "line" of current table
                j, newcol = ReadLinesInTabBlock(target_ntabs, j, desclines)
                tabcols += newcol

    if tabcols:
       
        tabtitle = ""
        if first_table and argnames[-1] == 'optlist':
            tabtitle = "Optlist options"
        OutputGatheredColumnsAsTable(mdfile_lines, tabcols, argnames, tabtitle)

    mdfile_lines.append("\n")

    return i

def ProcessMethod(i, lines, sec):
    meth = lines[i][:-1]
    sec[meth] = []
    sec[meth].append("### `%s()`\n\n"%meth)
    sec[meth].append("* **Summary:** %s\n\n"%FormatText(lines[i+1][1:].strip()))
    i += 2
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        argnames = []
        handled_line = False
        if re.search(r'^Synopsis:$', lines[i]):
            i = ProcessSynopsis(sec[meth], i, lines)
            handled_line = True
        if re.search(r'^Arguments:$', lines[i]):
            i, argnames = ProcessArgumentListBlock(sec[meth], i, lines)
            handled_line = True
        if re.search(r'^Returns:$', lines[i]):
            i = ProcessReturnBlock(sec[meth], i, lines)
            handled_line = True
        if re.search(r'^Description:$', lines[i]):
            i = ProcessDescription(sec[meth], i, lines, argnames)
            handled_line = True
        if not handled_line:
            print("Didn't classify line...\"",lines[i],"\"")
            OutputLineWithFormatting(sec[meth], lines[i], argnames, "  ")
            i += 1
    sec[meth].append("---\n")
    sec[meth].append('<br>'*40)
    sec[meth].append("\n")
    return i, meth

#
# main program
#
lines = GetFileLines("Chapter2-man_pages2.txt")

i = 0
s = 0
mdfiles = {}
while i < len(lines):
    handled_line = False
    if IsAPISectionHeader(lines[i]):
        i, s, sec = StartNewSection(i, s, lines, mdfiles)
        handled_line = True
    if IsMethodHeader(i, lines):
        i, m = ProcessMethod(i, lines, mdfiles[sec])
        handled_line = True
    if not handled_line:
        print("Didn't classify line...\"",lines[i],"\"")
        OutputLineWithFormatting(lines[i], mdfiles[sec][m])
        i += 1

#
# Make a pass creating links from text of the form...
#    (See DBOpen)
#    See "DBSet...
#    See `DBReadVar` 
#    See “DBMakeMrgtree” on page 196
#    See below
#    See description
#    See documentation
#    See HDF5 reference manual
#
usingSec=""
def fixlinks(match):

    func = match.groups()[2]+match.groups()[3]
    lfunc = func.lower()

    for sec in mdfiles:
        if lfunc in [meth.lower() for meth in mdfiles[sec].keys()]:
            if sec == usingSec:
                return "See [`"+func+"`](#"+lfunc+")"
            else:
                return "See [`"+func+"`](./"+fileNameFromSec[sec]+".md#"+lfunc+")"

    return "See [`"+func+"`](UNKNOWN)"

for sec in mdfiles:
    for meth in mdfiles[sec]:
        for i in range(len(mdfiles[sec][meth])):

            #
            # skip lines that don't have 'See' indicating a ref
            #
            if 'See' not in mdfiles[sec][meth][i]:
                continue 

            #
            # Handle links to HDF5 docs
            #
            mdfiles[sec][meth][i] = re.sub(r'See HDF5 reference manual',
                r'See [HDF5 reference manual](https://docs.hdfgroup.org/hdf5/develop/)',mdfiles[sec][meth][i])

            #
            # Handle internal links
            #
            usingSec = sec
            mdfiles[sec][meth][i] = re.sub(r'See (the description of |the write-up of |the documentation for |the documentation on |documentation on )?(["`]?)(DB|db)([a-zA-Z0-9_]*)(["`]?)( on page ([0-9-]*))?',
                fixlinks, mdfiles[sec][meth][i])


def ProcessMethodTable(mdfile, methods):

    mdfile.write("### Methods and symbols in this section\n")
    mdfile.write("\n")
    mdfile.write("&nbsp;|&nbsp;|&nbsp;\n")
    mdfile.write(":---|:---|:---\n")

    i = 1
    for m in methods:
        if m == "API Section":
            continue
        mdfile.write("[`"+m+"`](#"+m.lower()+")")
        if i%3 == 0:
            mdfile.write("\n")
        else:
            mdfile.write("|")
        i += 1
    mdfile.write("\n")
    mdfile.write("\n")

    mdfile.write("---\n")
    mdfile.write('<br>'*40)
    mdfile.write("\n")
    
#
# Output the files
#
s = 0
for sec in mdfiles:
#    with open("Chapter2-Section%d.md"%s,"wt") as mdfile:
    with open("%s.md"%fileNameFromSec[sec],"wt") as mdfile:
        for meth in mdfiles[sec]:
            mdfile.writelines(mdfiles[sec][meth])
            if meth == "API Section":
                ProcessMethodTable(mdfile, sorted(mdfiles[sec].keys()))
    s += 1

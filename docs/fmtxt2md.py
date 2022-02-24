import re, itertools

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
    if re.search(r'^[0-9]* API Section', line):
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
def OutputLineWithFormatting(mdfile, line, kwnames=[], indent=""):

    # Break text ending at elipsis, dot, question mark, exclamation point followed by
    line = re.sub(r'\(e\.g\. ',r'@e@g@@',line)
    line = re.sub(r'([a-zA-Z0-9_)]\.\.\.|[a-zA-Z0-9_)]\.|[a-zA-Z0-9_)]\?|[a-zA-Z0-9_)]!)\s+',r'\1\n',line).strip()
    line = re.sub(r'@e@g@@',r'(e.g. ',line)
    sentences = line.split('\n')

    for s in sentences:

        s = FormatText(s, kwnames)

        mdfile.write(indent + s.strip())
        mdfile.write("\n")

    mdfile.write("\n")

#
# Begin a new API section (new md file)
#
def StartNewSection(i, s, lines):
    mdfile = open("Chapter2-Section%d.md"%s,"wt")
    hs = re.search(r'^[0-9]*\s*API Section\s*(.*)',lines[i])
    if len(hs.groups()) > 0:
        mdfile.write("## %s"%FormatText(hs.groups()[0]).strip())
        mdfile.write("\n")
    else:
        mdfile.write("## Section %d\n"%s)
    s += 1
    mdfile.write("\n")
    i += 1

    while i < len(lines) and not IsMethodHeader(i+1, lines):
        OutputLineWithFormatting(mdfile, lines[i])
        i += 1
    mdfile.write("\n\n")

    return i, s, mdfile

def ProcessSynopsis(mdfile, i, lines):
    i += 1
    mdfile.write("* **C Signature:**\n\n") # FIXME REMOVE THIS
    mdfile.write("  ```\n")
    mdfile.write('  ' + lines[i].strip())
    mdfile.write("\n")
    i += 1
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         re.search(r'^Fortran Equivalent:$', lines[i]) or \
         re.search(r'^Returns:$', lines[i]) or \
         re.search(r'^Arguments:$', lines[i])):
        mdfile.write('      ' + lines[i].strip())
        mdfile.write("\n")
        i += 1
    mdfile.write("  ```\n")
    if re.search(r'^Fortran Equivalent:$', lines[i]):
        mdfile.write("\n")
        i += 1
        if re.search(r'^None$', lines[i]):
            mdfile.write("* **Fortran Signature:**\n\n")
            mdfile.write("  ```\n")
            mdfile.write("  None\n")
            mdfile.write("  ```\n")
            i += 1
        else:
            mdfile.write("* **Fortran Signature:**\n\n")
            mdfile.write("  ```\n")
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
                        mdfile.write(ident + ', '.join(fort_args[:n]))
                        if fort_args[:n][-1][-1] != ')':
                            mdfile.write(",")
                        mdfile.write("\n")
                        fort_args = fort_args[n:]
                        n = 6
                        ident = "     "
                    fort_args = []
                    fort_args_done = True
                    mdfile.write("  ```\n\n")
                elif fort_args_done:
                    mdfile.write('  ' + FormatText(lines[i].strip(' \n')))
                    mdfile.write("\n")
                i += 1
    mdfile.write("\n")
    return i

def ProcessArgumentListBlock(mdfile, i, lines):
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
        mdfile.write("* **Arguments:**\n\n")
        mdfile.write("  `None`\n")
        return i, []
    mdfile.write("* **Arguments:**\n\n")
    if len(args) % 2 != 0:
        print("***ARGS PROBLEM***\n")
        print(args)
    # use non-breaking spaces to enforce min table width for args column
    mdfile.write("  Arg&nbsp;name | Description\n")
    mdfile.write("  :---|:---\n")
    argnames = [args[2*j].strip() for j in range(int(len(args)/2))]
    j = 0
    while j < len(args):
        mdfile.write("  `%s` | %s\n"%(args[j].strip(), FormatText(args[j+1].strip(),argnames) if j+1<len(args) else "ARGS PROBLEM"))
        j += 2
    mdfile.write("\n\n")
    return i, argnames

#https://github.com/LLNL/Silo/blob/mcm86-27jan22-convert-docs-fm2md/src/silo/silo.h.in#L802
#https://github.com/LLNL/Silo/blob/main/src/silo/silo.h.in#L802

def ProcessReturnBlock(mdfile, i, lines):
    i += 1
    retlines = []
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         IsAPISectionHeader(lines[i]) or \
         re.search(r'^Description:$', lines[i])):
        if lines[i].strip() != '' and lines[i].strip().lower()[:4] != 'none':
            retlines += [lines[i]]
        i += 1
    mdfile.write("* **Returned value:**\n\n")
    if len(retlines):
        j = 0
        while j < len(retlines):
            OutputLineWithFormatting(mdfile,retlines[j],[],"  ")
            j += 1
    else:
        mdfile.write("  void");
    mdfile.write("\n\n")
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

def OutputGatheredColumnsAsTable(mdfile, cols, argnames=[], tabtitle=""):

    cols = list(map(list, itertools.zip_longest(*cols, fillvalue=None)))
    mdfile.write("  **%s**\n\n"%(tabtitle+":" if tabtitle else "&nbsp;"))
    mdfile.write('  ' + '|'.join(cols[0]))
    mdfile.write("\n")
    mdfile.write('  ' + ":---|"*(len(cols[0])-1))
    mdfile.write(":---\n")

    for c in cols[1:]:
        c = [x if x else '' for x in c]
        mdfile.write('  ' + FormatText('|'.join(c),argnames)) # FIXME
        mdfile.write("\n")

    mdfile.write("\n\n")

def ProcessDescription(mdfile, i, lines, argnames=[]):

    i += 1
    desclines = []
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        desclines += [lines[i]]
        i += 1

    if not desclines:
        return i

    mdfile.write("* **Description:**\n\n")

    j = 0
    target_ntabs = 0
    tabcols = []
    first_table = True
    while j < len(desclines):

        if target_ntabs == 0:

            if desclines[j].count('\t') == 0:

                # Output a normal "line" (which may be multiple sentences)
                # in description
                OutputLineWithFormatting(mdfile, desclines[j], argnames, "  ")
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
                OutputGatheredColumnsAsTable(mdfile, tabcols, argnames, tabtitle)
                first_table = False
                tabcols = []

                # Indicate we're done with current table
                target_ntabs = 0

                # Now, output the current line
                OutputLineWithFormatting(mdfile, desclines[j], argnames, "  ")
                j += 1

            else:

                # gather another "line" of current table
                j, newcol = ReadLinesInTabBlock(target_ntabs, j, desclines)
                tabcols += newcol

    if tabcols:
       
        tabtitle = ""
        if first_table and argnames[-1] == 'optlist':
            tabtitle = "Optlist options"
        OutputGatheredColumnsAsTable(mdfile, tabcols, argnames, tabtitle)

    mdfile.write("\n")

    return i

def ProcessMethod(mdfile, i, lines):
    mdfile.write("### `%s()`\n\n"%lines[i][:-1])
    #mdfile.write("> %s\n"%lines[i+1][1:])
    mdfile.write("* **Summary:** %s\n\n"%FormatText(lines[i+1][1:].strip()))
    i += 2
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        argnames = []
        handled_line = False
        if re.search(r'^Synopsis:$', lines[i]):
            i = ProcessSynopsis(mdfile, i, lines)
            handled_line = True
        if re.search(r'^Arguments:$', lines[i]):
            i, argnames = ProcessArgumentListBlock(mdfile, i, lines)
            handled_line = True
        if re.search(r'^Returns:$', lines[i]):
            i = ProcessReturnBlock(mdfile, i, lines)
            handled_line = True
        if re.search(r'^Description:$', lines[i]):
            i = ProcessDescription(mdfile, i, lines, argnames)
            handled_line = True
        if not handled_line:
            OutputLineWithFormatting(mdfile, lines[i], argnames, "  ")
            i += 1
    mdfile.write("---\n")
    mdfile.write('<br>'*40)
    mdfile.write("\n")
    return i

#
# main program
#
lines = GetFileLines("Chapter2-man_pages2.txt")

i = 0
s = 0
while i < len(lines):
    handled_line = False
    if IsAPISectionHeader(lines[i]):
        i, s, mdfile = StartNewSection(i, s, lines)
        handled_line = True
    if IsMethodHeader(i, lines):
        i = ProcessMethod(mdfile, i, lines)
        handled_line = True
    if not handled_line:
        OutputLineWithFormatting(mdfile, lines[i])
        i += 1

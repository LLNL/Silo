import re, itertools

# 
# Get all documentation lines from text file
#
def GetFileLines(fname):
    with open(fname) as txtfile:
        return txtfile.readlines()

#
# Begin a new API section (new md file)
#
def StartNewSection(i, s, lines):
    mdfile = open("Chapter2-Section%d.md"%s,"wt")
    s += 1
    hs = re.search(r'^[0-9]*\s*API Section\s*(.*)',lines[i])
    if len(hs.groups()) > 0:
        mdfile.write("## %s"%hs.groups()[0])
    else:
        mdfile.write("## Unknown")
    mdfile.write("\n\n")
    i += 1
    while i < len(lines) and \
        not re.search(r'^_visit_defvars\s*275$', lines[i]) and \
        not re.search(r'^dbmkptr\s*283$', lines[i]) and \
        not re.search(r'^Silo.Open$', lines[i]) and \
        not re.search(r'^DBGetComponentNames$', lines[i]) and \
        not re.search(r'^DB[A-Za-z0-9]*\s*[0-9]*$', lines[i]):
        as_sentences = re.sub(r'(?!\.\.)([\.!?]\s*(?!\)))',r'\1\n',lines[i])
        mdfile.writelines(as_sentences)
        i += 1
    mdfile.write("\n")
    return i, s, mdfile

def IsMethodHeader(i, lines):
    if i < len(lines)-1 and re.search(r'^—', lines[i+1]):
        return True
    return False

def IsAPISectionHeader(line):
    if re.search(r'^[0-9]* API Section', line):
        return True
    return False

def ProcessSynopsis(mdfile, i, lines):
    i += 1
    mdfile.write("#### C Signature\n")
    mdfile.write("```\n")
    mdfile.write(lines[i])
    i += 1
    while i < len(lines) and not \
        (IsMethodHeader(i, lines) or \
         re.search(r'^Fortran Equivalent:$', lines[i]) or \
         re.search(r'^Returns:$', lines[i]) or \
         re.search(r'^Arguments:$', lines[i])):
        indented_line = '    ' + lines[i].strip()
        mdfile.write(indented_line)
        mdfile.write("\n")
        i += 1
    mdfile.write("```\n")
    if re.search(r'^Fortran Equivalent:$', lines[i]):
        i += 1
        if re.search(r'^None$', lines[i]):
            mdfile.write("#### Fortran Signature:\n")
            mdfile.write("```\n")
            mdfile.write("None\n")
            mdfile.write("```\n")
            i += 1
        else:
            mdfile.write("#### Fortran Signature\n")
            mdfile.write("```\n")
            fort_args = []
            fort_args_done = False
            while i < len(lines) and not \
                (IsMethodHeader(i, lines) or \
                 re.search(r'^Returns:$', lines[i]) or \
                 re.search(r'^Arguments:$', lines[i])):
                fort_args += [w.strip(' \n') for w in lines[i].split(',') if len(w.strip(' \n'))>0]
                if not fort_args_done and re.search(r'[ \t,a-zA-Z0-9_]*\)$', lines[i]):
                    n = 4
                    ident = ""
                    while len(fort_args):
                        mdfile.write(ident + ', '.join(fort_args[:n]))
                        if fort_args[:n][-1][-1] != ')':
                            mdfile.write(",")
                        mdfile.write("\n")
                        fort_args = fort_args[n:]
                        n = 6
                        ident = "   "
                    fort_args = []
                    fort_args_done = True
                elif fort_args_done:
                    mdfile.write(lines[i].strip(' \n'))
                    mdfile.write("\n")
                i += 1
            mdfile.write("```\n")
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
        mdfile.write("#### Arguments: None\n")
        return i
    if len(args) % 2 != 0:
        print("***ARGS PROBLEM***\n")
        print(args)
    # use non-breaking spaces to enforce table width
    #mdfile.write("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Arg name | Description&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n")
    #mdfile.write("------Arg name | Description--------------------------------------------------------------------------------\n")
    mdfile.write("Arg name | Description\n")
    mdfile.write(":--|:---\n")
    j = 0
    while j < len(args):
#        mdfile.write("* `%s` : %s\n"%(args[j].strip(), args[j+1].strip() if j+1<len(args) else "ARGS PROBLEM"))
#        mdfile.write("\n`%s` : %s\n"%(args[j].strip(), args[j+1].strip() if j+1<len(args) else "ARGS PROBLEM"))
        mdfile.write("`%s` | %s\n"%(args[j].strip(), args[j+1].strip() if j+1<len(args) else "ARGS PROBLEM"))
        j += 2
    mdfile.write("\n")
    return i

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
    mdfile.write("#### Returned value:\n")
    if len(retlines):
        j = 0
        while j < len(retlines):
            mdfile.write(retlines[j].strip())
            mdfile.write("\n")
            j += 1
    else:
        mdfile.write("void");
    mdfile.write("\n")
    return i

def ReadLinesInTabBlock(target_ntabs, i, desclines):

    complete_line = ''
    while complete_line.count('\t') < target_ntabs and i < len(desclines):
        #complete_line += desclines[i].strip(" \n")
        complete_line += desclines[i]
        i += 1
    if complete_line.count('\t') != target_ntabs:
        print("Bad Case")
    complete_line = complete_line.strip()
    complete_line = re.sub("\n","<br>", complete_line)
    return i, [complete_line.split('\t')]

def OutputLineAsSentencePerLine(mdfile, line):

    # break into sentences
    split_line = re.split("\.( ?[A-Z]?)", line.strip())
    split_line = [w.strip() for w in split_line if w.strip()]
    new_split_line = []
    k = 0
    while k < len(split_line):
        if k < len(split_line)-1 and len(split_line[k]) == 1 and split_line[k][0].isupper():
            new_split_line += [split_line[k] + split_line[k+1]]
            k+=1
        else:
            new_split_line += [split_line[k]]
        k+=1

    # output sentences, one per line
    for sent in new_split_line:
        mdfile.write(sent)
        mdfile.write(".\n")

    mdfile.write("\n")

def OutputGatheredColumnsAsTable(mdfile, cols):

    cols = list(map(list, itertools.zip_longest(*cols, fillvalue=None)))
    mdfile.write("\n")
    mdfile.write('|'.join(cols[0]))
    mdfile.write("\n")
    mdfile.write(":---|"*(len(cols[0])-1))
    mdfile.write(":---\n")

    for c in cols[1:]:
        c = [x if x else '' for x in c]
        mdfile.write('|'.join(c))
        mdfile.write("\n")

    mdfile.write("\n")

def ProcessDescription(mdfile, i, lines):
    i += 1
    desclines = []
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        desclines += [lines[i]]
        i += 1

    if not len(desclines):
        return i

    mdfile.write("#### Description:\n\n")

    j = 0
    target_ntabs = 0
    tabcols = []
    while j < len(desclines):

        if target_ntabs == 0:

            if desclines[j].count('\t') == 0:

                # Output a normal "line" (which may be multiple sentences)
                # in description
                OutputLineAsSentencePerLine(mdfile, desclines[j])
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
                OutputGatheredColumnsAsTable(mdfile, tabcols)

                # Indicate we're done with current table
                target_ntabs = 0

                # Now, output the current line
                OutputLineAsSentencePerLine(mdfile, desclines[j])
                j += 1

            else:

                # gather another "line" of current table
                j, newcol = ReadLinesInTabBlock(target_ntabs, j, desclines)
                tabcols += newcol

    if target_ntabs:
        OutputGatheredColumnsAsTable(mdfile, tabcols)

    return i

def ProcessMethod(mdfile, i, lines):
    mdfile.write("### `%s()` - %s\n"%(lines[i][:-1], lines[i+1][1:]))
    i += 2
    while i < len(lines) and \
        not IsMethodHeader(i, lines) and \
        not IsAPISectionHeader(lines[i]):
        if re.search(r'^Synopsis:$', lines[i]):
            i = ProcessSynopsis(mdfile, i, lines)
        if re.search(r'^Arguments:$', lines[i]):
            i = ProcessArgumentListBlock(mdfile, i, lines)
        if re.search(r'^Returns:$', lines[i]):
            i = ProcessReturnBlock(mdfile, i, lines)
        if re.search(r'^Description:$', lines[i]):
            i = ProcessDescription(mdfile, i, lines)
        i += 1
    return i

#
# main program
#
lines = GetFileLines("Chapter2-man_pages2.txt")

s = 0
for i in range(len(lines)):
    if IsAPISectionHeader(lines[i]):
        i, s, mdfile = StartNewSection(i, s, lines)
    if IsMethodHeader(i, lines):
        i = ProcessMethod(mdfile, i, lines)

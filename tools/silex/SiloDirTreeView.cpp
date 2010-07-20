/*****************************************************************************
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*****************************************************************************/

#include "SiloDirTreeView.h"
#include <SiloFile.h>
#include <QPixmap>

#include "folder.xpm"

// ****************************************************************************
//  Constructor:  SiloDirTreeView::SiloDirTreeView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloDirTreeView::SiloDirTreeView(SiloFile *s, QWidget *p)
    : QTreeWidget(p), silo(s)
{
    folder_pixmap = new QPixmap(folder_xpm);
 
    setColumnCount(1);
    headerItem()->setText(0,"Contents");

    total_items = 0;
    QTreeWidgetItem *root = AddDir(silo->root, NULL);
    root->setExpanded(true);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::AddDir
//
//  Purpose:
//    Add a new directory to the DirTree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
QTreeWidgetItem *
SiloDirTreeView::AddDir(SiloDir *d, QTreeWidgetItem *parent)
{
    total_items++;

    QTreeWidgetItem *item;
    if (!parent)
        item = new SiloDirTreeViewItem(d, this, d->name);
    else
        item = new SiloDirTreeViewItem(d, parent, d->name);

    item->setIcon(0,*folder_pixmap);
    for (unsigned int i=0; i<d->subdir.size(); i++)
        AddDir(d->subdir[i], item);

    return item;
}

// ****************************************************************************
//  Method:  SiloDirTreeView::Set
//
//  Purpose:
//    Reset the view to a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 12 20:52:16 PDT 2004
//    Set the internal silo file to the new file.
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloDirTreeView::Set(SiloFile *s)
{
    silo = s;
    clear();
    total_items = 0;
    QTreeWidgetItem *root = AddDir(silo->root, NULL);
    root->setExpanded(true);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::OpenRootDir
//
//  Purpose:
//    Select the root directory of the tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloDirTreeView::OpenRootDir()
{
    setCurrentItem(topLevelItem(0));
}

// ****************************************************************************
//  Method:  SiloDirTreeView::resizeEvent
//
//  Purpose:
//    Make the column header fill the width of the listview.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloDirTreeView::resizeEvent(QResizeEvent *re)
{
    QTreeWidget::resizeEvent(re);
    setColumnWidth(0, width() - 4);
}


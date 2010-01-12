/*****************************************************************************
*
* Copyright (c) 2000 - 2009, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "SiloObjectView.h"
#include <SiloFile.h>
#include <QMessageBox>
#include <iostream>
#include <string>
#include <cstdlib>

// ----------------------------------------------------------------------------
//                            Object View Window
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor:  SiloObjectViewWindow::SiloObjectViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloObjectViewWindow::SiloObjectViewWindow(SiloFile *s, const QString &n, QWidget *p)
    : QMainWindow(p), silo(s), name(n)
{
    setWindowTitle(QString("Object: ")+name);

    SiloObjectView *ov = new SiloObjectView(silo,name,this);
    setCentralWidget(ov);
    connect(ov,   SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(ShowItem(QTreeWidgetItem*,int)));
}

// ****************************************************************************
//  Method:  SiloObjectViewWindow::ShowItem
//
//  Purpose:
//    A slot used to signal a "show var" event.
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
SiloObjectViewWindow::ShowItem(QTreeWidgetItem *i, int)
{
    if (i->text(1) == "var")
        emit showRequested(i->text(2));
}

// ----------------------------------------------------------------------------
//                               Object View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloObjectView::SiloObjectView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon May 17 12:37:32 PDT 2004
//    Added a couple calls to free memory and prevent really big leaks.
//
//    Mark C. Miller, Wed Apr 20 17:08:36 PDT 2005
//    Added code to deal with hdf5 formatted strings in pdbname
//
//    Jeremy Meredith, Wed May 11 12:42:12 PDT 2005
//    Show an error message if we get an invalid object.
//
//    Mark C. Miller, Tue Nov  6 16:08:19 PST 2007
//    Fixed compiler warning.
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
//    Mark C. Miller, Fri Dec  4 15:19:47 PST 2009
//    Adding support for long long type
//
//    Mark C. Miller, Mon Dec  7 09:50:19 PST 2009
//    Conditionally compile long long support only when its
//    different from long.
//
//    Mark C. Miller, Mon Jan 11 16:02:16 PST 2010
//    Made long long support UNconditionally compiled.
// ****************************************************************************
SiloObjectView::SiloObjectView(SiloFile *s, const QString &n, QWidget *p)
    : QTreeWidget(p), silo(s), name(n)
{
    //setSorting(-1);
    setAllColumnsShowFocus(true);
    setColumnCount(3);
    headerItem()->setText(0,"Component");
    headerItem()->setText(1,"Type");
    headerItem()->setText(2,"Value");

    DBobject *object = silo->GetObject(name);
    if (!object)
    {
        QMessageBox::warning(this, "silex", "Could not read this object.\n"
           "The file may have been written using an incomplete driver.", "OK");
        return;
    }

    for (int i=0; i<object->ncomponents; i++)
    {
        QString compname = object->comp_names[i];
        QString pdbname  = object->pdb_names[i];
        void *comp = silo->GetComponent(name, compname);
        if (!comp)
        {
            const char *asciiname = name.toAscii();
            const char *asciicomp = compname.toAscii();
            std::cerr << "ERROR: DBGetComponent failed for object '"
                      << asciiname
                      <<"', component '"
                      << asciicomp
                      <<std::endl;
            continue;
        }
        int   type = silo->GetComponentType(name, compname);
        QString typestr = "";
        char  value[256] = "";
        int ival = -1;
        switch (type)
        {
          case DB_INT:
            typestr = "int";
            sprintf(value, "%d", *((int*)comp));
            ival = *((int*)comp);
            break;
          case DB_SHORT:
            typestr = "short";
            sprintf(value, "%d", *((short*)comp));
            ival = *((short*)comp);
            break;
          case DB_LONG:
            typestr = "long";
            sprintf(value, "%ld", *((long*)comp));
            ival = (int) *((long*)comp);
            break;
          case DB_LONG_LONG:
            typestr = "long long";
            sprintf(value, "%lld", *((long long*)comp));
            ival = (int) *((long long*)comp);
            break;
          case DB_FLOAT:
            typestr = "float";
            sprintf(value, "%g", *((float*)comp));
            break;
          case DB_DOUBLE:
            typestr = "double";
            sprintf(value, "%g", *((double*)comp));
            break;
          case DB_CHAR:
            typestr = "char";
            sprintf(value, "%s", ((char*)comp));
            break;
          case DB_NOTYPE:
            typestr = "notype";
            sprintf(value, "NOTYPE");
            break;
          default:
            typestr = "var";
            std::string valStr = std::string(pdbname.toAscii());
            if (pdbname.indexOf("'<s>") == 0)
            {
                int len = pdbname.length();
                valStr = std::string((const char*)(pdbname.toAscii()),4,len-5);
            }
            sprintf(value, "%s", valStr.c_str());
            break;
        }

        // No such call as "DBFreeComponent".  Maybe there should be one!
        free(comp);
        comp = NULL;

        if (type==DB_INT || type==DB_SHORT || type==DB_LONG)
        {
            if (compname == "coordtype")
            {
                if (ival == DB_COLLINEAR)    strcat(value, " (DB_COLLINEAR)");
                if (ival == DB_NONCOLLINEAR) strcat(value, " (DB_NONCOLLINEAR)");
            }
            if (compname == "centering")
            {
                if (ival == DB_NOTCENT)      strcat(value, " (DB_NOTCENT)");
                if (ival == DB_NODECENT)     strcat(value, " (DB_NODECENT)");
                if (ival == DB_ZONECENT)     strcat(value, " (DB_ZONECENT)");
                if (ival == DB_FACECENT)     strcat(value, " (DB_FACECENT)");
            }
            if (compname == "major_order")
            {
                if (ival == DB_ROWMAJOR)     strcat(value, " (DB_ROWMAJOR)");
                if (ival == DB_COLMAJOR)     strcat(value, " (DB_COLMAJOR)");
            }
            if (compname == "coord_sys")
            {
                if (ival == DB_CARTESIAN)    strcat(value, " (DB_CARTESIAN)");
                if (ival == DB_CYLINDRICAL)  strcat(value, " (DB_CYLINDRICAL)");
                if (ival == DB_SPHERICAL)    strcat(value, " (DB_SPHERICAL)");
                if (ival == DB_NUMERICAL)    strcat(value, " (DB_NUMERICAL)");
                if (ival == DB_OTHER)        strcat(value, " (DB_OTHER)");
            }
            if (compname == "planar")
            {
                if (ival == DB_AREA)         strcat(value, " (DB_AREA)");
                if (ival == DB_VOLUME)       strcat(value, " (DB_VOLUME)");
            }
            if (compname == "facetype")
            {
                if (ival == DB_RECTILINEAR)  strcat(value, " (DB_RECTILINEAR)");
                if (ival == DB_CURVILINEAR)  strcat(value, " (DB_CURVILINEAR)");
            }
            if (compname == "datatype")
            {
                if (ival == DB_INT)          strcat(value, " (DB_INT)");
                if (ival == DB_SHORT)        strcat(value, " (DB_SHORT)");
                if (ival == DB_LONG)         strcat(value, " (DB_LONG)");
                if (ival == DB_FLOAT)        strcat(value, " (DB_FLOAT)");
                if (ival == DB_DOUBLE)       strcat(value, " (DB_DOUBLE)");
                if (ival == DB_CHAR)         strcat(value, " (DB_CHAR)");
                if (ival == DB_NOTYPE)       strcat(value, " (DB_NOTYPE)");
            }
        }
        QStringList sl;
        sl << object->comp_names[i] << typestr << value;
        new QTreeWidgetItem(this, sl);
    }

    total_items = object->ncomponents;
    DBFreeObject(object);
}

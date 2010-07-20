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

#include "SiloDirView.h"
#include <SiloDirTreeView.h>
#include <SiloFile.h>

#include <QPixmap>
#include <QTreeWidgetItem>

#include "mesh.xpm"
#include "mat.xpm"
#include "var.xpm"
#include "object.xpm"
#include "species.xpm"
#include "curve.xpm"
#include "array.xpm"
#include "silovar.xpm"


// ****************************************************************************
//  Constructor:  SiloDirView::SiloDirView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloDirView::SiloDirView(QWidget *p)
    : QTreeWidget(p)
{
    total_items = 0;

    mesh_pixmap   = new QPixmap(mesh_xpm);
    mat_pixmap    = new QPixmap(mat_xpm);
    var_pixmap    = new QPixmap(var_xpm);
    obj_pixmap    = new QPixmap(object_xpm);
    spec_pixmap   = new QPixmap(species_xpm);
    curve_pixmap  = new QPixmap(curve_xpm);
    array_pixmap  = new QPixmap(array_xpm);
    silovar_pixmap= new QPixmap(silovar_xpm);

    setColumnCount(1);
    headerItem()->setText(0,"Objects");
    setRootIsDecorated(true);
}

// ****************************************************************************
//  Method:  SiloDirView::Set
//
//  Purpose:
//    View the TOC of a new directory.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 12 16:57:37 PDT 2002
//    Split the total_items calculation into multiple lines.  g++-2.96
//    was choking on it for some odd reason.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added multimesh-adjacency object support 
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloDirView::Set(SiloDir *d)
{
    clear();

    total_items  = d->array.size()           + (d->array.size()           ? 1:0);
    total_items += d->obj.size()             + (d->obj.size()             ? 1:0);
    total_items += d->var.size()             + (d->var.size()             ? 1:0);
    total_items += d->defvars.size()         + (d->defvars.size()         ? 1:0);
    total_items += d->matspecies.size()      + (d->matspecies.size()      ? 1:0);
    total_items += d->mat.size()             + (d->mat.size()             ? 1:0);
    total_items += d->ptvar.size()           + (d->ptvar.size()           ? 1:0);
    total_items += d->ptmesh.size()          + (d->ptmesh.size()          ? 1:0);
    total_items += d->ucdvar.size()          + (d->ucdvar.size()          ? 1:0);
    total_items += d->ucdmesh.size()         + (d->ucdmesh.size()         ? 1:0);
    total_items += d->qvar.size()            + (d->qvar.size()            ? 1:0);
    total_items += d->qmesh.size()           + (d->qmesh.size()           ? 1:0);
    total_items += d->multimatspecies.size() + (d->multimatspecies.size() ? 1:0);
    total_items += d->multimat.size()        + (d->multimat.size()        ? 1:0);
    total_items += d->multivar.size()        + (d->multivar.size()        ? 1:0);
    total_items += d->multimesh.size()       + (d->multimesh.size()       ? 1:0);
    total_items += d->multimeshadj.size()    + (d->multimeshadj.size()    ? 1:0);
    total_items += d->curve.size()           + (d->curve.size()           ? 1:0);
    total_items += d->csgvar.size()          + (d->csgvar.size()          ? 1:0);
    total_items += d->csgmesh.size()         + (d->csgmesh.size()         ? 1:0);

    bool expandVars = true;
    if (d->curve.size())
    {
        SiloDirViewItem *curve = new SiloDirViewItem(NULL,this, "Curves");
        curve->setIcon(0, *curve_pixmap);
        for (unsigned int i=0; i<d->curve.size(); i++)
            new SiloDirViewItem(d,curve, d->curve[i]);
        curve->setExpanded(true);
    }

    if (d->multimesh.size())
    {
        SiloDirViewItem *multimesh = new SiloDirViewItem(NULL,this, "MultiMeshes");
        multimesh->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->multimesh.size(); i++)
            new SiloDirViewItem(d,multimesh, d->multimesh[i]);
        multimesh->setExpanded(true);
        expandVars = false;
    }

    if (d->multivar.size())
    {
        SiloDirViewItem *multivar = new SiloDirViewItem(NULL,this, "MultiVars");
        multivar->setIcon(0, *var_pixmap);
        for (unsigned int i=0; i<d->multivar.size(); i++)
            new SiloDirViewItem(d,multivar, d->multivar[i]);
        multivar->setExpanded(true);
        expandVars = false;
    }

    if (d->multimeshadj.size())
    {
        SiloDirViewItem *multimeshadj = new SiloDirViewItem(NULL,this, "MultiMesheadjs");
        multimeshadj->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->multimeshadj.size(); i++)
            new SiloDirViewItem(d,multimeshadj, d->multimeshadj[i]);
        multimeshadj->setExpanded(true);
        expandVars = false;
    }


    if (d->multimat.size())
    {
        SiloDirViewItem *multimat = new SiloDirViewItem(NULL,this, "MultiMats");
        multimat->setIcon(0, *mat_pixmap);
        for (unsigned int i=0; i<d->multimat.size(); i++)
            new SiloDirViewItem(d,multimat, d->multimat[i]);
        multimat->setExpanded(true);
        expandVars = false;
    }

    if (d->multimatspecies.size())
    {
        SiloDirViewItem *multimatspecies = new SiloDirViewItem(NULL,this, "MultiSpecies");
        multimatspecies->setIcon(0, *spec_pixmap);
        for (unsigned int i=0; i<d->multimatspecies.size(); i++)
            new SiloDirViewItem(d,multimatspecies, d->multimatspecies[i]);
        multimatspecies->setExpanded(true);
        expandVars = false;
    }

    if (d->qmesh.size())
    {
        SiloDirViewItem *qmesh = new SiloDirViewItem(NULL,this, "QuadMeshes");
        qmesh->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->qmesh.size(); i++)
            new SiloDirViewItem(d,qmesh, d->qmesh[i]);
        qmesh->setExpanded(true);
        expandVars = false;
    }

    if (d->qvar.size())
    {
        SiloDirViewItem *qvar = new SiloDirViewItem(NULL,this, "QuadVars");
        qvar->setIcon(0, *var_pixmap);
        for (unsigned int i=0; i<d->qvar.size(); i++)
            new SiloDirViewItem(d,qvar, d->qvar[i]);
        qvar->setExpanded(true);
        expandVars = false;
    }

    if (d->ucdmesh.size())
    {
        SiloDirViewItem *ucdmesh = new SiloDirViewItem(NULL,this, "UCDMeshes");
        ucdmesh->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->ucdmesh.size(); i++)
            new SiloDirViewItem(d,ucdmesh, d->ucdmesh[i]);
        ucdmesh->setExpanded(true);
        expandVars = false;
    }

    if (d->ucdvar.size())
    {
        SiloDirViewItem *ucdvar = new SiloDirViewItem(NULL,this, "UCDVars");
        ucdvar->setIcon(0, *var_pixmap);
        for (unsigned int i=0; i<d->ucdvar.size(); i++)
            new SiloDirViewItem(d,ucdvar, d->ucdvar[i]);
        ucdvar->setExpanded(true);
        expandVars = false;
    }

    if (d->ptmesh.size())
    {
        SiloDirViewItem *ptmesh = new SiloDirViewItem(NULL,this, "PointMeshes");
        ptmesh->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->ptmesh.size(); i++)
            new SiloDirViewItem(d,ptmesh, d->ptmesh[i]);
        ptmesh->setExpanded(true);
        expandVars = false;
    }

    if (d->ptvar.size())
    {
        SiloDirViewItem *ptvar = new SiloDirViewItem(NULL,this, "PointVars");
        ptvar->setIcon(0, *var_pixmap);
        for (unsigned int i=0; i<d->ptvar.size(); i++)
            new SiloDirViewItem(d,ptvar, d->ptvar[i]);
        ptvar->setExpanded(true);
        expandVars = false;
    }

    if (d->csgmesh.size())
    {
        SiloDirViewItem *csgmesh = new SiloDirViewItem(NULL,this, "CSGMeshes");
        csgmesh->setIcon(0, *mesh_pixmap);
        for (unsigned int i=0; i<d->csgmesh.size(); i++)
            new SiloDirViewItem(d,csgmesh, d->csgmesh[i]);
        csgmesh->setExpanded(true);
        expandVars = false;
    }

    if (d->csgvar.size())
    {
        SiloDirViewItem *csgvar = new SiloDirViewItem(NULL,this, "CSGVars");
        csgvar->setIcon(0, *var_pixmap);
        for (unsigned int i=0; i<d->csgvar.size(); i++)
            new SiloDirViewItem(d,csgvar, d->csgvar[i]);
        csgvar->setExpanded(true);
        expandVars = false;
    }

    if (d->mat.size())
    {
        SiloDirViewItem *mat = new SiloDirViewItem(NULL,this, "Materials");
        mat->setIcon(0, *mat_pixmap);
        for (unsigned int i=0; i<d->mat.size(); i++)
            new SiloDirViewItem(d,mat, d->mat[i]);
        mat->setExpanded(true);
        expandVars = false;
    }

    if (d->matspecies.size())
    {
        SiloDirViewItem *matspecies = new SiloDirViewItem(NULL,this, "Species");
        matspecies->setIcon(0, *spec_pixmap);
        for (unsigned int i=0; i<d->matspecies.size(); i++)
            new SiloDirViewItem(d,matspecies, d->matspecies[i]);
        matspecies->setExpanded(true);
        expandVars = false;
    }

    if (d->obj.size())
    {
        SiloDirViewItem *obj = new SiloDirViewItem(NULL,this, "Objects");
        obj->setIcon(0, *obj_pixmap);
        for (unsigned int i=0; i<d->obj.size(); i++)
            new SiloDirViewItem(d,obj, d->obj[i]);
        obj->setExpanded(true);
    }

    if (d->array.size())
    {
        SiloDirViewItem *array = new SiloDirViewItem(NULL,this, "Arrays");
        array->setIcon(0, *array_pixmap);
        for (unsigned int i=0; i<d->array.size(); i++)
            new SiloDirViewItem(d,array, d->array[i]);
        array->setExpanded(true);
    }

    if (d->var.size())
    {
        SiloDirViewItem *var = new SiloDirViewItem(NULL,this, "Vars");
        var->setIcon(0, *silovar_pixmap);
        for (unsigned int i=0; i<d->var.size(); i++)
            new SiloDirViewItem(d,var, d->var[i]);
        if (expandVars)
            var->setExpanded(true);
    }
}

// ****************************************************************************
//  Method:  SiloDirView::ChangeDir
//
//  Purpose:
//    Wrapper for "Set" which is suitable for a QTreeWidgetItem callback.
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
SiloDirView::ChangeDir(QTreeWidgetItem *i)
{
    SiloDirTreeViewItem *item = (SiloDirTreeViewItem*)i;
    headerItem()->setText(0, QString("Contents of ") + item->dir->path);
    Set(item->dir);
}

// ****************************************************************************
//  Method:  SiloDirView::resizeEvent
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
SiloDirView::resizeEvent(QResizeEvent *re)
{
    QTreeWidget::resizeEvent(re);
    setColumnWidth(0, width() - 4);
}

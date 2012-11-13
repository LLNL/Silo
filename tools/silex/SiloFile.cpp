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

#include "SiloFile.h"

#include <silo.h>

// ----------------------------------------------------------------------------
//                             Silo Dir
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  SiloDir::SiloDir
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Nov 18 12:02:13 PDT 2002
//    Ported to Windows.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Made it able to compile with different versions of Silo
//
//    Mark C. Miller, Tue Oct  7 21:22:04 PDT 2008
//    Quieted frequent warnings of DBFreeCompression not being implemented.
//    This can be a common situation.
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloDir::SiloDir(DBfile *db, const QString &name_, const QString &path_)
{
    name = name_;
    path = path_;
    static bool canCallFreeCompression[20] = {true, true, true, true, true,
                                              true, true, true, true, true,
                                              true, true, true, true, true,
                                              true, true, true, true, true};

    int drt = DBGetDriverType(db);
    if (canCallFreeCompression[drt] && DBFreeCompressionResources(db,0) == -1)
        canCallFreeCompression[drt] = false;
    DBSetDir(db, (const char*)path.toAscii());
    DBtoc *toc = DBGetToc(db);

    int i;
    for (i=0; i<toc->ncurve; i++)
        curve.push_back(toc->curve_names[i]);
#ifdef DBCSG_QUADRIC_G
    for (i=0; i<toc->ncsgmesh; i++)
        csgmesh.push_back(toc->csgmesh_names[i]);
    for (i=0; i<toc->ncsgvar; i++)
        csgvar.push_back(toc->csgvar_names[i]);
#endif
#ifdef DB_VARTYPE_SCALAR
    for (i=0; i<toc->ndefvars; i++)
        defvars.push_back(toc->defvars_names[i]);
#endif
    for (i=0; i<toc->nmultimesh; i++)
        multimesh.push_back(toc->multimesh_names[i]);
    for (i=0; i<toc->nmultivar; i++)
        multivar.push_back(toc->multivar_names[i]);
#ifdef DBCSG_QUADRIC_G // mmadj came into Silo same time as CSG stuff
    for (i=0; i<toc->nmultimeshadj; i++)
        multimeshadj.push_back(toc->multimeshadj_names[i]);
#endif
    for (i=0; i<toc->nmultimat; i++)
        multimat.push_back(toc->multimat_names[i]);
    for (i=0; i<toc->nmultimatspecies; i++)
        multimatspecies.push_back(toc->multimatspecies_names[i]);
    for (i=0; i<toc->nqmesh; i++)
        qmesh.push_back(toc->qmesh_names[i]);
    for (i=0; i<toc->nqvar; i++)
        qvar.push_back(toc->qvar_names[i]);
    for (i=0; i<toc->nucdmesh; i++)
        ucdmesh.push_back(toc->ucdmesh_names[i]);
    for (i=0; i<toc->nucdvar; i++)
        ucdvar.push_back(toc->ucdvar_names[i]);
    for (i=0; i<toc->nptmesh; i++)
        ptmesh.push_back(toc->ptmesh_names[i]);
    for (i=0; i<toc->nptvar; i++)
        ptvar.push_back(toc->ptvar_names[i]);
    for (i=0; i<toc->nmat; i++)
        mat.push_back(toc->mat_names[i]);
    for (i=0; i<toc->nmatspecies; i++)
        matspecies.push_back(toc->matspecies_names[i]);
    for (i=0; i<toc->nvar; i++)
        var.push_back(toc->var_names[i]);
    for (i=0; i<toc->nobj; i++)
        obj.push_back(toc->obj_names[i]);
    for (i=0; i<toc->narray; i++)
        array.push_back(toc->array_names[i]);
    for (i=0; i<toc->ndir; i++)
        dir.push_back(toc->dir_names[i]);
    for (i=0; i<toc->nmrgtree; i++)
        dir.push_back(toc->mrgtree_names[i]);
    for (i=0; i<toc->nmrgvar; i++)
        dir.push_back(toc->mrgvar_names[i]);
    for (i=0; i<toc->ngroupelmap; i++)
        dir.push_back(toc->groupelmap_names[i]);

    for (unsigned int i=0; i<dir.size(); i++)
    {
        if (name == "/")
            subdir.push_back(new SiloDir(db, dir[i], path + dir[i]));
        else
            subdir.push_back(new SiloDir(db, dir[i], path + "/" + dir[i]));
    }
}

// ****************************************************************************
//  Destructor:  SiloDir::~SiloDir
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloDir::~SiloDir()
{
    for (unsigned int i=0; i<subdir.size(); i++)
    {
        delete subdir[i];
    }
}




// ----------------------------------------------------------------------------
//                              Silo File
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  SiloFile::SiloFile
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Made it more graceful on failure to open
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloFile::SiloFile(const QString &name)
{
    db = DBOpen((const char*)name.toAscii(), DB_UNKNOWN, DB_READ);
    if (db)
        root = new SiloDir(db, "/", "/");
    else
        root = 0;
}


// ****************************************************************************
//  Destructor:  SiloFile::~SiloFile
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Added some missing delete calls
//
// ****************************************************************************
SiloFile::~SiloFile()
{
    if (root) delete root;
    root = 0;
    if (db) DBClose(db);
    db = 0;
}


// ****************************************************************************
//  Methods:     Silo API Functions
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************

void*
SiloFile::GetVar(const QString &name)
{
    return DBGetVar(db, (const char*)name.toAscii());
}

int
SiloFile::GetVarType(const QString &name)
{
    return DBGetVarType(db, (const char*)name.toAscii());
}

int
SiloFile::GetVarLength(const QString &name)
{
    return DBGetVarLength(db, (const char*)name.toAscii());
}

DBobject*
SiloFile::GetObject(const QString &name)
{
    return DBGetObject(db, (const char*)name.toAscii());
}

void*
SiloFile::GetComponent(const QString &oname, const QString &cname)
{
    return DBGetComponent(db, (const char*)oname.toAscii(), (const char*)cname.toAscii());
}

int
SiloFile::GetComponentType(const QString &oname, const QString &cname)
{
    return DBGetComponentType(db, (const char*)oname.toAscii(), (const char*)cname.toAscii());
}

DBObjectType
SiloFile::InqVarType(const QString &name)
{
    return DBInqVarType(db, (const char*)name.toAscii());
}

bool
SiloFile::InqVarExists(const QString &name)
{
    return DBInqVarExists(db, (const char*)name.toAscii());
}

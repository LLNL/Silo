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

#ifndef SILOFILE_H
#define SILOFILE_H

#include <silo.h>
#include <config.h>
#include <QString>
#include <vector>
using std::vector;

// ****************************************************************************
//  Class:  SiloDir
//
//  Purpose:
//    Encapsulation of a silo directory (TOC).
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added support for multimesh adjacency object
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
class SiloDir
{
  public:
    QString          name;
    QString          path;
    vector<SiloDir*> subdir;

    vector<QString>  curve;
    vector<QString>  csgmesh;
    vector<QString>  csgvar;
    vector<QString>  defvars;
    vector<QString>  multimesh;
    vector<QString>  multimeshadj;
    vector<QString>  multivar;
    vector<QString>  multimat;
    vector<QString>  multimatspecies;
    vector<QString>  qmesh;
    vector<QString>  qvar;
    vector<QString>  ucdmesh;
    vector<QString>  ucdvar;
    vector<QString>  ptmesh;
    vector<QString>  ptvar;
    vector<QString>  mat;
    vector<QString>  matspecies;
    vector<QString>  var;
    vector<QString>  obj;
    vector<QString>  array;
    vector<QString>  dir;
  public:
    SiloDir(DBfile *db, const QString &name, const QString &path);
    ~SiloDir();
};

// ****************************************************************************
//  Class:  SiloFile
//
//  Purpose:
//    Encapsulation of a Silo file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Added IsOpen()
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
//    Mark C. Miller, Wed May 26 18:19:11 PDT 2010
//    Added InqVarExists.
// ****************************************************************************
class SiloFile
{
  public:
    SiloDir *root;

  public:
    SiloFile(const QString &name);
    ~SiloFile();

    bool          IsOpen() const { return db != 0; };
    void         *GetVar(const QString &name);
    int           GetVarType(const QString &name);
    int           GetVarLength(const QString &name);
    DBobject     *GetObject(const QString &name);
    void         *GetComponent(const QString &oname, const QString &cname);
    int           GetComponentType(const QString &oname, const QString &cname);
    DBObjectType  InqVarType(const QString &name);
    bool          InqVarExists(const QString &name);

  private:
    DBfile *db;
};

#endif

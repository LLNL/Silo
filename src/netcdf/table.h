#ifndef TABLE_H
#define TABLE_H

/*

                           Copyright (c) 1991 - 2009
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

/*==================================================
 *
 * Data Structure Definitions
 *
 *==================================================
 *                      W A R N I N G
 *
 *  If any of these data structures are changed,
 *  they must also be changed in the lite_PD_defstr
 *  invocations in silo_Setup.
 *
 *                      W A R N I N G
 *==================================================*/

typedef struct {
    int            relid;       /* Relative ID within directory */
    int            parent;      /* Directory ID of parent */
    int            size;        /* Dimension size */
    char          *name;        /* Name by which entity is known */
} DimEnt;

/*==================================================
 * Table declarations.
 *==================================================*/

typedef struct {
    DirEnt       **ent;
    int            num_alloced;
    int            num_used;
} DirTable;

typedef struct {
    DimEnt       **ent;
    int            num_alloced;
    int            num_used;
} DimTable;

typedef struct {
    AttEnt       **ent;
    int            num_alloced;
    int            num_used;
} AttTable;

typedef struct {
    VarEnt       **ent;
    int            num_alloced;
    int            num_used;
} VarTable;

typedef struct {
    ObjEnt       **ent;
    int            num_alloced;
    int            num_used;
} ObjTable;

typedef struct {
    void         **ent;
    int            num_alloced;
    int            num_used;
} Table;

/*
 *  Structure containing list of active DB's, as well as the association
 *   between PDB and SILO Id's. For internal use only.
 */

typedef struct {
    int            dbid;        /* Database identifier */
    int            modified;    /* Sentinel indicating file was modified */
    PDBfile       *pdbfile;     /* PDB file pointer */
    int            curr_dir;    /* Current active directory in SILO */
} SILOTable;

extern DirTable *dirTable[MAX_SILO];
extern DimTable *dimTable[MAX_SILO];
extern AttTable *attTable[MAX_SILO];
extern VarTable *varTable[MAX_SILO];
extern ObjTable *objTable[MAX_SILO];
extern SILOTable silo_table[MAX_SILO];

#endif /* TABLE_H */

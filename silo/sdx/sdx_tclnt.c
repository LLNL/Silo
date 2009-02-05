/*

                           Copyright 1991 - 1995
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

#define DB_SDX
#define DB_MAIN
#include <silo.h>
#include <sdx.h>

#ifdef ALLOC
#undef ALLOC
#endif
#ifdef ALLOC_N
#undef ALLOC_N
#endif
#ifdef REALLOC
#undef REALLOC
#endif
#ifdef FREE
#undef FREE
#endif

#define ALLOC(x)         (x *) malloc(sizeof(x))
#define ALLOC_N(x,n)     (x *) calloc (n, sizeof (x))
#define REALLOC(p,x,n)   (x *) realloc (p, (n)*sizeof(x))
#define FREE(x)          if ( (x) != NULL) {free(x);(x)=NULL;}

int
main()
{
    int            i;
    DBfile        *dbfile;
    float         *time;
    int           *cycle;
    char          *info;
    DBquadmesh    *qm;
    DBquadvar     *qv;
    DBmaterial    *mat;
    DBmultimesh   *mm;
    char           meshname[256];
    int            meshtype;

    printf("Waiting for a connection.\n");
    dbfile = DBOpen("rect_2ds", DB_SDX);
    printf("Connection is established.\n");

    if ((info = (char *)DBGetVar(dbfile, "_fileinfo")) != NULL) {
        printf("The info is '%s'\n", info);
        FREE(info);
    }

    if ((cycle = (int *)DBGetVar(dbfile, "cycle")) != NULL) {
        printf("The cycle is %8d\n", *cycle);
        FREE(cycle);
    }

    if ((time = (float *)DBGetVar(dbfile, "time")) != NULL) {
        printf("The time is %8.3f\n", *time);
        FREE(time);
    }

    if ((mm = DBGetMultimesh(dbfile, "mesh1")) != NULL) {
        print_mm(mm);
        free_mm(mm);
    }

    if ((DBInqMeshname(dbfile, "d", meshname)) >= 0) {
        printf("The mesh name is '%s'\n", meshname);
    }

    if ((meshtype = DBInqMeshtype(dbfile, "mesh1")) != NULL) {
        if (meshtype == DB_QUAD_RECT)
            printf("The mesh type for mesh1 is DB_QUAD_RECT.\n");
        else
            printf("Bad mesh type for mesh1.\n");
    }

    if ((meshtype = DBInqMeshtype(dbfile, "mat1")) != NULL) {
        if (meshtype == DB_MATERIAL)
            printf("The mesh type for mat1 is DB_MATERIAL.\n");
        else
            printf("Bad mesh type for mat1.\n");
    }

    if ((meshtype = DBInqMeshtype(dbfile, "d")) != NULL) {
        if (meshtype == DB_QUADVAR)
            printf("The mesh type for d is DB_QUADVAR.\n");
        else
            printf("Bad mesh type for d.\n");
    }

    DBContinue(dbfile);

    for (i = 0; i < 4; i++) {
        printf("Delaying for 9 seconds.\n");
        SDXWait(9);

        if ((cycle = (int *)DBGetVar(dbfile, "cycle")) != NULL) {
            printf("The cycle is %8d\n", *cycle);
            FREE(cycle);
        }

        if ((time = (float *)DBGetVar(dbfile, "time")) != NULL) {
            printf("The time is %8.3f\n", *time);
            FREE(time);
        }

        if ((qm = DBGetQuadmesh(dbfile, "mesh1")) != NULL) {
            print_qm(qm);
            free_qm(qm);
        }

        if ((mat = DBGetMaterial(dbfile, "mat1")) != NULL) {
            print_mat(mat);
            free_mat(mat);
        }

        if ((qv = DBGetQuadvar(dbfile, "d")) != NULL) {
            print_qv(qv);
            free_qv(qv);
        }

        SDXContinue(SDXID(dbfile));
    }

    DBClose(dbfile);

    return(0);
}

free_qm(qm)
    DBquadmesh    *qm;
{
    int            i;

    FREE(qm->name);
    for (i = 0; i < 3; i++) {
        FREE(qm->coords[i]);
        FREE(qm->labels[i]);
        FREE(qm->units[i]);
    }
    FREE(qm);
}

free_qv(qv)
    DBquadvar     *qv;
{
    int            i;

    FREE(qv->name);
    FREE(qv->units);
    FREE(qv->label);
    for (i = 0; i < qv->nvals; i++) {
        FREE(qv->vals[i]);
    }
    FREE(qv->vals);
    FREE(qv);
}

free_mat(mat)
    DBmaterial    *mat;
{

    FREE(mat->name);
    FREE(mat->matnos);
    FREE(mat->matlist);
    FREE(mat->mix_vf);
    FREE(mat->mix_next);
    FREE(mat->mix_mat);
    FREE(mat->mix_zone);
    FREE(mat);
}

free_mm(mm)
    DBmultimesh   *mm;
{
    int            i;

    FREE(mm->meshids);
    for (i = 0; i < mm->nblocks; i++) {
        FREE(mm->meshnames[i]);
    }
    FREE(mm->meshnames);
    FREE(mm->meshtypes);
    FREE(mm->dirids);
    FREE(mm);
}

print_qm(qm)
    DBquadmesh    *qm;
{
    int            i;
    char          *carray;
    int           *iarray;
    float         *farray;
    double        *darray;

    printf("The quad mesh is:\n");
    printf("   id          = %d\n", qm->id);
    printf("   block_no    = %d\n", qm->block_no);
    printf("   name        = '%s'\n", qm->name);
    printf("   cycle       = %d\n", qm->cycle);
    printf("   time        = %f\n", qm->time);
    printf("   coord_sys   = %d\n", qm->coord_sys);
    printf("   major_order = %d\n", qm->major_order);
    printf("   stride      = %d, %d, %d\n",
           qm->stride[0], qm->stride[1], qm->stride[2]);
    printf("   coordtype   = %d\n", qm->coordtype);
    printf("   facetype    = %d\n", qm->facetype);
    printf("   planar      = %d\n", qm->planar);

    for (i = 0; i < qm->ndims; i++) {
        switch (qm->datatype) {
            case DB_CHAR:
                carray = (char *)qm->coords[i];
                printf("   coords [%d]  = %c, %c, %c, %c, %c\n", i,
                       carray[0], carray[1], carray[2],
                       carray[3], carray[4]);
                break;
            case DB_INT:
                iarray = (int *)qm->coords[i];
                printf("   coords [%d]  = %d, %d, %d, %d, %d\n", i,
                       iarray[0], iarray[1], iarray[2],
                       iarray[3], iarray[4]);
                break;
            case DB_FLOAT:
                farray = (float *)qm->coords[i];
                printf("   coords [%d]  = %f, %f, %f, %f, %f\n", i,
                       farray[0], farray[1], farray[2],
                       farray[3], farray[4]);
                break;
            case DB_DOUBLE:
                darray = (double *)qm->coords[i];
                printf("   coords [%d]  = %f, %f, %f, %f, %f\n", i,
                       darray[0], darray[1], darray[2],
                       darray[3], darray[4]);
                break;
        }
    }
    switch (qm->datatype) {
        case DB_CHAR:
            printf("   datatype    = DB_CHAR\n");
            break;
        case DB_INT:
            printf("   datatype    = DB_INT\n");
            break;
        case DB_FLOAT:
            printf("   datatype    = DB_FLOAT\n");
            break;
        case DB_DOUBLE:
            printf("   datatype    = DB_DOUBLE\n");
            break;
        default:
            printf("   datatype    = UNKNOWN\n");
            break;
    }
    printf("   min_extents = %f, %f, %f\n",
           qm->min_extents[0], qm->min_extents[1], qm->min_extents[2]);
    printf("   max_extents = %f, %f, %f\n",
           qm->max_extents[0], qm->max_extents[1], qm->max_extents[2]);
    printf("   labels      = '%s', '%s', '%s'\n",
           qm->labels[0], qm->labels[1], qm->labels[2]);
    printf("   units       = '%s', '%s', '%s'\n",
           qm->units[0], qm->units[1], qm->units[2]);
    printf("   ndims       = %d\n", qm->ndims);
    printf("   nspace      = %d\n", qm->nspace);
    printf("   nnodes      = %d\n", qm->nnodes);

    printf("   dims        = %d, %d, %d\n",
           qm->dims[0], qm->dims[1], qm->dims[2]);
    printf("   origin      = %d\n", qm->origin);
    printf("   min_index   = %d, %d, %d\n",
           qm->min_index[0], qm->min_index[1], qm->min_index[2]);
    printf("   max_index   = %d, %d, %d\n",
           qm->max_index[0], qm->max_index[1], qm->max_index[2]);
}

print_mm(mm)
    DBmultimesh   *mm;
{

    printf("The multimesh is:\n");
    printf("   id            = %d\n", mm->id);
    printf("   nblocks       = %d\n", mm->nblocks);
    printf("   meshids [0]   = %d\n", mm->meshids[0]);
    printf("   meshnames [0] = '%s'\n", mm->meshnames[0]);
    printf("   meshtypes [0] = %d\n", mm->meshtypes[0]);
    printf("   dirids [0]    = 0x%x\n", mm->dirids[0]);
}

print_mat(mat)
    DBmaterial    *mat;
{
    int            i;
    char          *carray;
    int           *iarray;
    float         *farray;
    double        *darray;

    printf("The material is:\n");
    printf("   id          = %d\n", mat->id);
    printf("   name        = '%s'\n", mat->name);
    printf("   ndims       = %d\n", mat->ndims);
    printf("   origin      = %d\n", mat->origin);
    printf("   dims        = %d, %d, %d\n",
           mat->dims[0], mat->dims[1], mat->dims[2]);
    printf("   major_order = %d\n", mat->major_order);
    printf("   stride      = %d, %d, %d\n",
           mat->stride[0], mat->stride[1], mat->stride[2]);
    printf("   nmat        = %d\n", mat->nmat);
    printf("   matnos [0]  = %d, %d, %d, %d, %d\n",
           mat->matnos[0], mat->matnos[1], mat->matnos[2],
           mat->matnos[3], mat->matnos[4]);
    printf("   matlist [0] = %d, %d, %d, %d, %d\n",
           mat->matlist[0], mat->matlist[1], mat->matlist[2],
           mat->matlist[3], mat->matlist[4]);
    printf("   mixlen      = %d\n", mat->mixlen);
    switch (mat->datatype) {
        case DB_CHAR:
            printf("   datatype    = DB_CHAR\n");
            break;
        case DB_INT:
            printf("   datatype    = DB_INT\n");
            break;
        case DB_FLOAT:
            printf("   datatype    = DB_FLOAT\n");
            break;
        case DB_DOUBLE:
            printf("   datatype    = DB_DOUBLE\n");
            break;
        default:
            printf("   datatype    = UNKNOWN\n");
            break;
    }
    if (mat->mixlen > 0) {
        switch (mat->datatype) {
            case DB_CHAR:
                carray = (char *)mat->mix_vf;
                printf("   mix_vf [0]  = %c, %c, %c, %c, %c\n",
                       carray[0], carray[1], carray[2],
                       carray[3], carray[4]);
                break;
            case DB_INT:
                iarray = (int *)mat->mix_vf;
                printf("   mix_vf [0]  = %d, %d, %d, %d, %d\n",
                       iarray[0], iarray[1], iarray[2],
                       iarray[3], iarray[4]);
                break;
            case DB_FLOAT:
                farray = (float *)mat->mix_vf;
                printf("   mix_vf [0]  = %f, %f, %f, %f, %f\n",
                       farray[0], farray[1], farray[2],
                       farray[3], farray[4]);
                break;
            case DB_DOUBLE:
                darray = (double *)mat->mix_vf;
                printf("   mix_vf [0]  = %f, %f, %f, %f, %f\n",
                       darray[0], darray[1], darray[2],
                       darray[3], darray[4]);
                break;
        }
        printf("   mix_next [0]= %d, %d, %d, %d, %d\n",
               mat->mix_next[0], mat->mix_next[1], mat->mix_next[2],
               mat->mix_next[3], mat->mix_next[4]);
        printf("   mix_mat [0] = %d, %d, %d, %d, %d\n",
               mat->mix_mat[0], mat->mix_mat[1], mat->mix_mat[2],
               mat->mix_mat[3], mat->mix_mat[4]);
        if (mat->mix_zone == NULL) {
            printf("   mix_zone    = NULL\n");
        }
        else {
            printf("   mix_zone [0]= %d, %d, %d, %d, %d\n",
                   mat->mix_zone[0], mat->mix_zone[1], mat->mix_zone[2],
                   mat->mix_zone[3], mat->mix_zone[4]);
        }
    }
}

print_qv(qv)
    DBquadvar     *qv;
{
    int            i;
    char          *carray;
    int           *iarray;
    float         *farray;
    double        *darray;

    printf("The quad variable is:\n");
    printf("   id          = %d\n", qv->id);
    printf("   name        = '%s'\n", qv->name);
    printf("   units       = '%s'\n", qv->units);
    printf("   label       = '%s'\n", qv->label);
    printf("   cycle       = %d\n", qv->cycle);
    printf("   time        = %f\n", qv->time);
    printf("   meshid      = %d\n", qv->meshid);

    switch (qv->datatype) {
        case DB_CHAR:
            carray = (char *)qv->vals[0];
            printf("   vals   [0]  = %c, %c, %c, %c, %c\n",
                   carray[0], carray[1], carray[2],
                   carray[3], carray[4]);
            break;
        case DB_INT:
            iarray = (int *)qv->vals[0];
            printf("   vals   [0]  = %d, %d, %d, %d, %d\n",
                   iarray[0], iarray[1], iarray[2],
                   iarray[3], iarray[4]);
            break;
        case DB_FLOAT:
            farray = (float *)qv->vals[0];
            printf("   vals   [0]  = %f, %f, %f, %f, %f\n",
                   farray[0], farray[1], farray[2],
                   farray[3], farray[4]);
            break;
        case DB_DOUBLE:
            darray = (double *)qv->vals[0];
            printf("   vals   [0]  = %f, %f, %f, %f, %f\n",
                   darray[0], darray[1], darray[2],
                   darray[3], darray[4]);
            break;
    }
    switch (qv->datatype) {
        case DB_CHAR:
            printf("   datatype    = DB_CHAR\n");
            break;
        case DB_INT:
            printf("   datatype    = DB_INT\n");
            break;
        case DB_FLOAT:
            printf("   datatype    = DB_FLOAT\n");
            break;
        case DB_DOUBLE:
            printf("   datatype    = DB_DOUBLE\n");
            break;
        default:
            printf("   datatype    = UNKNOWN\n");
            break;
    }
    printf("   nels        = %d\n", qv->nels);
    printf("   nvals       = %d\n", qv->nvals);
    printf("   ndims       = %d\n", qv->ndims);
    printf("   dims        = %d, %d, %d\n",
           qv->dims[0], qv->dims[1], qv->dims[2]);

    printf("   major_order = %d\n", qv->major_order);
    printf("   stride      = %d, %d, %d\n",
           qv->stride[0], qv->stride[1], qv->stride[2]);
    printf("   min_index   = %d, %d, %d\n",
           qv->min_index[0], qv->min_index[1], qv->min_index[2]);
    printf("   max_index   = %d, %d, %d\n",
           qv->max_index[0], qv->max_index[1], qv->max_index[2]);
    printf("   origin      = %d\n", qv->origin);
    printf("   align       = %f, %f, %f\n",
           qv->align[0], qv->align[1], qv->align[2]);
}

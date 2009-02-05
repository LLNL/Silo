/*
 * Copyright © 1999 Regents of the University of California
 *                  All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Tuesday, February  9, 1999
 *
 * Purpose:     A very incomplete and simple hdf5 driver so that pmesh can
 *              write >2GB of data to SAMI files which can later be read by
 *              ALEC.
 *
 *              This has evolved into a complete HDF5 driver for SILO.
 *
 * Note:        This file can be compiled even when HDF5 is not available.
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   Fixed several issues with this driver so that Ale3d can
 *   now restart from it. Added dataReadMask functionality
 *
 */
#include <assert.h>
#if HAVE_STDLIB_H
#include <stdlib.h> /*missing from silo header files*/
#endif
#include "silo_hdf5_private.h"
#if defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5)

/* Defining these to check overhead of PROTECT */
#if 0
#   define PROTECT      if(1)
#   define CLEANUP      else
#   define END_PROTECT  /*void*/
#   define UNWIND()     abort()
#endif

#define FALSE           0
#define TRUE            1

#define LINKGRP         "/.silo/"       /*name of link group            */
#define MAX_VARS        8               /*max vars per DB*var object    */
#define OPTDUP(S)       ((S)&&*(S)?strdup(S):NULL)
#define BASEDUP(S)       ((S)&&*(S)?strdup(db_FullName2BaseName(S)):NULL)
#define ALIGN(ADDR,N)   (((ADDR)+(N)-1)&~((N)-1))

/* to encode the version of the hdf5 library in any silo executable */
char SILO_built_with_H5_lib_vers_info_g[] = "SILO built with "
#if (H5_VERS_MAJOR>=1 && H5_VERS_MINOR>=4 && H5_VERS_RELEASE>=2) || \
    (H5_VERS_MAJOR>=1 && H5_VERS_MINOR>4) || \
    (H5_VERS_MAJOR>1)
H5_VERS_INFO;
#else
"HDF5 library: Version 1.4.1 or earlier";
#endif

/* Use `float' for all memory floating point values? */
static int              force_single_g;

/* used to control behavior of GetZonelist */
static int              calledFromGetUcdmesh = 0;

/* Struct used when building the CWD name */
typedef struct silo_hdf5_comp_t {
    char                *name;
    unsigned long       objno[2];
} silo_hdf5_comp_t;

/* Attributes for various types of objects */
typedef struct DBcurve_mt {
    int                 npts;
    int                 guihide;
    char                xvarname[256];
    char                yvarname[256];
    char                label[256];
    char                xlabel[256];
    char                ylabel[256];
    char                xunits[256];
    char                yunits[256];
    char                reference[256];
} DBcurve_mt;
static hid_t DBcurve_mt5 = -1;

typedef struct DBcsgmesh_mt {
    int            block_no;
    int            group_no;
    int            cycle;
    int            nbounds;
    int            lcoeffs;
    float          time;
    double         dtime;
    int            ndims;
    int            origin;
    int            guihide;
    double         min_extents[3];
    double         max_extents[3];
    char           units[3][256];
    char           labels[3][256];
    char           name[256];
    char           typeflags[256];
    char           bndids[256];
    char           coeffs[256];
    char           zonel_name[256];
    char           bndnames[256];
} DBcsgmesh_mt;
static hid_t DBcsgmesh_mt5;

typedef struct DBcsgvar_mt {
    int            cycle;
    float          time;
    double         dtime;
    int            datatype;
    int            nels;
    int            nvals;
    int            centering;
    int            use_specmf;
    int            ascii_labels;
    int            guihide;
    char           name[256];
    char           units[256];
    char           label[256];
    char           vals[MAX_VARS][256];
    char           meshname[256];
} DBcsgvar_mt;
static hid_t DBcsgvar_mt5;

typedef struct DBcsgzonelist_mt {
    int            nregs;
    int            origin;
    int            lxform;
    int            datatype;
    int            nzones;
    int            min_index;
    int            max_index;
    char           typeflags[256];
    char           leftids[256];
    char           rightids[256];
    char           xform[256];
    char           zonelist[256];
    char           regnames[256];
    char           zonenames[256];
} DBcsgzonelist_mt;
static hid_t DBcsgzonelist_mt5;

typedef struct DBdefvars_mt {
    int            ndefs;
    char           names[256];
    char           types[256];
    char           defns[256];
    char           guihides[256];
} DBdefvars_mt;
static hid_t DBdefvars_mt5;

typedef struct DBquadmesh_mt {
    char                coord[3][256];
    double              min_extents[3];
    double              max_extents[3];
    int                 ndims;
    int                 coordtype;
    int                 nspace;
    int                 nnodes;
    int                 facetype;
    int                 major_order;
    int                 cycle;
    int                 coord_sys;
    int                 planar;
    int                 origin;
    int                 group_no;
    int                 dims[3];
    int                 min_index[3];
    int                 max_index[3];
    int                 baseindex[3];
    float               time;
    double              dtime;
    int                 guihide;
    char                label[3][256];
    char                units[3][256];
} DBquadmesh_mt;
static hid_t DBquadmesh_mt5;

typedef struct DBquadvar_mt {
    char                value[MAX_VARS][256];
    char                mixed_value[MAX_VARS][256];
    char                meshid[256];
    int                 ndims;
    int                 nvals;
    int                 nels;
    int                 origin;
    int                 mixlen;
    int                 major_order;
    int                 datatype;
    int                 cycle;
    float               time;
    double              dtime;
    int                 use_specmf;
    int                 ascii_labels;
    int                 dims[3];
    int                 zones[3];
    int                 min_index[3];
    int                 max_index[3];
    float               align[3];
    int                 guihide;
    char                label[256];
    char                units[256];
} DBquadvar_mt;
static hid_t    DBquadvar_mt5;

typedef struct DBucdmesh_mt {
    char                coord[3][256];
    int                 ndims;
    int                 nnodes;
    int                 nzones;
    int                 facetype;
    int                 cycle;
    int                 coord_sys;
    int                 topo_dim;
    int                 planar;
    int                 origin;
    int                 group_no;
    float               time;
    double              dtime;
    int                 guihide;
    char                facelist[256];
    char                zonelist[256];
    char                gnodeno[256];
    double              min_extents[3];
    double              max_extents[3];
    char                label[3][256];
    char                units[3][256];
    char                phzonelist[256];
} DBucdmesh_mt;
static hid_t    DBucdmesh_mt5;

typedef struct DBucdvar_mt {
    char                value[MAX_VARS][256];
    char                mixed_value[MAX_VARS][256];
    char                meshid[256];
    int                 ndims;
    int                 nvals;
    int                 nels;
    int                 centering;
    int                 origin;
    int                 mixlen;
    int                 datatype;
    int                 cycle;
    int                 use_specmf;
    int                 ascii_labels;
    float               time;
    double              dtime;
    int                 lo_offset;
    int                 hi_offset;
    int                 guihide;
    char                label[256];
    char                units[256];
} DBucdvar_mt;
static hid_t    DBucdvar_mt5;

typedef struct DBfacelist_mt {
    int                 ndims;
    int                 nfaces;
    int                 nshapes;
    int                 ntypes;
    int                 lnodelist;
    int                 origin;
    char                nodelist[256];
    char                shapecnt[256];
    char                shapesize[256];
    char                typelist[256];
    char                types[256];
    char                zoneno[256];
} DBfacelist_mt;
static hid_t    DBfacelist_mt5;

typedef struct DBzonelist_mt {
    int                 ndims;
    int                 nzones;
    int                 nshapes;
    int                 lnodelist;
    int                 origin;
    int                 lo_offset;
    int                 hi_offset;
    char                nodelist[256];
    char                shapecnt[256];
    char                shapesize[256];
    char                shapetype[256];
    char                gzoneno[256];
} DBzonelist_mt;
static hid_t    DBzonelist_mt5;

typedef struct DBphzonelist_mt {
    int                 nfaces;
    int                 lnodelist;
    int                 nzones;
    int                 lfacelist;
    int                 origin;
    int                 lo_offset;
    int                 hi_offset;
    char                nodecnt[256];
    char                nodelist[256];
    char                extface[256];
    char                facecnt[256];
    char                facelist[256];
    char                gzoneno[256];
} DBphzonelist_mt;
static hid_t    DBphzonelist_mt5;

typedef struct DBmaterial_mt {
    int                 ndims;
    int                 nmat;
    int                 mixlen;
    int                 origin;
    int                 major_order;
    int                 datatype;
    int                 dims[3];
    int                 allowmat0;
    int                 guihide;
    char                meshid[256];
    char                matlist[256];
    char                matnos[256];
    char                mix_vf[256];
    char                mix_next[256];
    char                mix_mat[256];
    char                mix_zone[256];
    char                matnames[256];
    char                matcolors[256];
} DBmaterial_mt;
static hid_t    DBmaterial_mt5;

typedef struct DBmultimesh_mt {
    int                 nblocks;
    int                 cycle;
    int                 ngroups;
    int                 blockorigin;
    int                 grouporigin;
    float               time;
    double              dtime;
    int                 guihide;
    int                 extentssize;
    char                meshtypes[256];
    char                meshnames[256];
    char                extents[256];
    char                zonecounts[256];
    char                has_external_zones[256];
    int                 lgroupings;
    char                groupings[256];
    char                groupnames[256];
} DBmultimesh_mt;
static hid_t    DBmultimesh_mt5;

typedef struct DBmultimeshadj_mt {
    int                 nblocks;
    int                 blockorigin;
    int                 lneighbors;
    int                 totlnodelists;
    int                 totlzonelists;
    char                meshtypes[256];
    char                nneighbors[256];
    char                neighbors[256];
    char                back[256];
    char                lnodelists[256];
    char                nodelists[256];
    char                lzonelists[256];
    char                zonelists[256];
} DBmultimeshadj_mt;
static hid_t DBmultimeshadj_mt5;

typedef struct DBmultivar_mt {
    int                 nvars;
    int                 cycle;
    int                 ngroups;
    int                 blockorigin;
    int                 grouporigin;
    float               time;
    double              dtime;
    int                 extentssize;
    int                 guihide;
    char                vartypes[256];
    char                varnames[256];
    char                extents[256];
} DBmultivar_mt;
static hid_t    DBmultivar_mt5;

typedef struct DBmultimat_mt {
    int                 nmats;
    int                 cycle;
    int                 ngroups;
    int                 blockorigin;
    int                 grouporigin;
    float               time;
    double              dtime;
    int                 allowmat0;
    int                 guihide;
    char                matnames[256];
    char                matnos[256];
    char                mixlens[256];
    char                matcounts[256];
    char                matlists[256];
    int                 nmatnos;
    char                material_names[256];
    char                mat_colors[256];
} DBmultimat_mt;
static hid_t    DBmultimat_mt5;

typedef struct DBmultimatspecies_mt {
    int                 nspec;
    int                 nmat;
    int                 cycle;
    int                 ngroups;
    int                 blockorigin;
    int                 grouporigin;
    float               time;
    double              dtime;
    int                 guihide;
    char                specnames[256];
    char                nmatspec[256];
    char                matname[256];
} DBmultimatspecies_mt;
static hid_t    DBmultimatspecies_mt5;

typedef struct DBmatspecies_mt {
    int                 ndims;
    int                 nmat;
    int                 nspecies_mf;
    int                 mixlen;
    int                 major_order;
    int                 datatype;
    int                 dims[3];
    int                 guihide;
    char                matname[256];
    char                speclist[256];
    char                nmatspec[256];
    char                species_mf[256];
    char                mix_speclist[256];
} DBmatspecies_mt;
static hid_t    DBmatspecies_mt5;

typedef struct DBpointmesh_mt {
    int                 ndims;
    int                 nspace;
    int                 nels;
    int                 cycle;
    int                 group_no;
    float               time;
    double              dtime;
    int                 origin;
    int                 min_index;
    int                 max_index;
    double              min_extents[3];
    double              max_extents[3];
    int                 guihide;
    char                coord[3][256];
    char                label[3][256];
    char                units[3][256];
    char                gnodeno[256];
} DBpointmesh_mt;
static hid_t    DBpointmesh_mt5;

typedef struct DBpointvar_mt {
    int                 nvals;
    int                 nels;
    int                 nspace;
    int                 origin;
    int                 min_index;
    int                 max_index;
    int                 datatype;
    int                 cycle;
    float               time;
    double              dtime;
    int                 guihide;
    int                 ascii_labels;
    char                meshid[256];
    char                label[256];
    char                units[256];
    char                data[MAX_VARS][256];
} DBpointvar_mt;
static hid_t    DBpointvar_mt5;

typedef struct DBcompoundarray_mt {
    int                 nelems;
    int                 nvalues;
    int                 datatype;
    char                values[256];
    char                elemnames[256];
    char                elemlengths[256];
} DBcompoundarray_mt;
static hid_t    DBcompoundarray_mt5;

static hid_t    T_char = -1;
static hid_t    T_short = -1;
static hid_t    T_int = -1;
static hid_t    T_long = -1;
static hid_t    T_float = -1;
static hid_t    T_double = -1;
static hid_t    T_str256 = -1;
static hid_t    SCALAR = -1;
static hid_t    P_crprops = -1;
static hid_t    P_rdprops = -1;
static hid_t    P_ckcrprops = -1;
static hid_t    P_ckrdprops = -1;

#define OPT(V)          ((V)?(V):"")
#define OFFSET(P,F)     ((char*)&((P).F)-(char*)&(P))
#define ENDOF(S)        ((S)+strlen(S))

/*
 * Use these macros to define compound data types. The following form defines
 * an hdf5 data type called `DBcurve_mt5' which describes the C data type
 * `DBcurve_mt'.
 *
 *      STRUCT(DBcurve) {
 *          MEMBER(int,         npts);
 *          MEMBER(int,         datatype);
 *          MEMBER(str256,      label);
 *      } DEFINE;
 *
 * The following construct creates a temporary hdf5 data type `_mt' that
 * which is based on the C data type `DBcurve_mt' but which has minimal sizes
 * for character strings and may be missing character string fields which are
 * empty.
 *
 *      DBcurve_mt m;
 *      STRUCT(DBcurve) {
 *          MEMBER(int,         npts);
 *          m.npts = npts;
 *          MEMBER(int,         datatype);
 *          m.datatype = dtype;
 *          MEMBER(str(xname),  xvarname);
 *          strcpy(m.xvarname, OPT(xname));
 *      } OUTPUT(dbfile, DB_CURVE, "objname", &m);
 */
#define STRUCT(S) {                                                           \
    int         _i, _j=0;       /*counters*/                                  \
    hsize_t     _size;          /*number of repeated components*/             \
    size_t      _f_off=0;       /*offset into file data type*/                \
    DBfile_hdf5 *_f=NULL;       /*file for target types*/                     \
    S##_mt      _m;             /*temp to calculate offsets*/                 \
    hid_t       _tmp_m, _tmp_f; /*memory and file temporaries*/               \
    hid_t       _mt=-1, _ft=-1; /*memory and file data types*/                \
    char        _fullname[256]; /*name for repeated members*/                 \
                                                                              \
    for (_i=0; _i<3; _i++) {                                                  \
        switch (_i) {                                                         \
        case 0:                                                               \
            /*                                                                \
             * Fall through to pick up closing arguments. Touch local         \
             * variables so compiler doesn't complain about them not being    \
             * used.                                                          \
             */                                                               \
            _size = sprintf(_fullname, "%d", _j);                             \
            break;                                                            \
                                                                              \
        case 2:                                                               \
            /* Define global DB*_mt5 data type for memory */                  \
            S##_mt5 = _mt;                                                    \
            break;                                                            \
                                                                              \
        case 1:                                                               \
            /* Build data types, file multiplier is arbitrary */              \
            _mt = H5Tcreate(H5T_COMPOUND, sizeof _m);                         \
            if (_f) _ft = H5Tcreate(H5T_COMPOUND, 3*sizeof _m);               \
            /* MEMBER DEFINITIONS HERE... */

#define MEMBER_S(TYPE,NAME) {                                                 \
    _tmp_m = T_##TYPE; /*possible function call*/                             \
    if (_tmp_m>=0) {                                                          \
        db_hdf5_put_cmemb(_mt, #NAME, OFFSET(_m, NAME), 0, NULL, _tmp_m);     \
        if (_f && (_tmp_f=_f->T_##TYPE)>=0) {                                 \
            db_hdf5_put_cmemb(_ft, #NAME, _f_off, 0, NULL, _tmp_f);           \
            _f_off += H5Tget_size(_tmp_f);                                    \
        }                                                                     \
    }                                                                         \
}

#if H5_VERS_MAJOR>=1 && H5_VERS_MINOR>=4
#define MEMBER_3(TYPE,NAME) {                                                 \
    _tmp_m = T_##TYPE; /*possible function call*/                             \
    if (_tmp_m>=0) {                                                          \
        hid_t _m_ary;                                                         \
        _size = 3;                                                            \
        _m_ary = H5Tarray_create(_tmp_m, 1, &_size, NULL);                    \
        db_hdf5_put_cmemb(_mt, #NAME, OFFSET(_m, NAME), 0, NULL, _m_ary);     \
        if (_f && (_tmp_f=_f->T_##TYPE)>=0) {                                 \
            hid_t _f_ary = H5Tarray_create(_tmp_f, 1, &_size, NULL);          \
            db_hdf5_put_cmemb(_ft, #NAME, _f_off, 0, NULL, _f_ary);           \
            _f_off += 3*H5Tget_size(_f_ary);                                  \
        }                                                                     \
    }                                                                         \
}
#else
#define MEMBER_3(TYPE,NAME) {                                                 \
    _tmp_m = T_##TYPE; /*possible function call*/                             \
    if (_tmp_m>=0) {                                                          \
        _size = 3;                                                            \
        db_hdf5_put_cmemb(_mt, #NAME, OFFSET(_m, NAME), 1, &_size, _tmp_m);   \
        if (_f && (_tmp_f=_f->T_##TYPE)>=0) {                                 \
            db_hdf5_put_cmemb(_ft, #NAME, _f_off, 1, &_size, _tmp_f);         \
            _f_off += 3*H5Tget_size(_tmp_f);                                  \
        }                                                                     \
    }                                                                         \
}
#endif

#define MEMBER_R(TYPE,NAME,N) {                                               \
    for (_j=0; _j<N; _j++) {                                                  \
        _tmp_m = T_##TYPE; /*possible function call*/                         \
        if (_tmp_m>=0) {                                                      \
            sprintf(_fullname, "%s%d", #NAME, _j);                            \
            db_hdf5_put_cmemb(_mt, _fullname, OFFSET(_m, NAME[_j]), 0, NULL,  \
                              _tmp_m);                                        \
            if (_f && (_tmp_f=_f->T_##TYPE)>=0) {                             \
                db_hdf5_put_cmemb(_ft, _fullname, _f_off, 0, NULL, _tmp_f);   \
                _f_off += H5Tget_size(_tmp_f);                                \
            }                                                                 \
        }                                                                     \
    }                                                                         \
}
 
#define OUTPUT(DBFILE,DBTYPE,NAME,MEM)                                        \
            break;                                                            \
        }                                                                     \
        if (0==_i) _f=(DBFILE);                                               \
        else if (1==_i) break;                                                \
    }                                                                         \
    H5Tpack(_ft);                                                             \
    db_hdf5_hdrwr(DBFILE, (char*)NAME, _mt, _ft, MEM, DBTYPE);                \
    H5Tclose(_mt);                                                            \
    H5Tclose(_ft);                                                            \
    suppress_set_but_not_used_warning(&_size);                                \
}

#define DEFINE                                                                \
            break;                                                            \
        }                                                                     \
    }                                                                         \
    suppress_set_but_not_used_warning(&_size);                                \
}

/*ARGSUSED*/
INTERNAL void
suppress_set_but_not_used_warning(const void *ptr)
{}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_file
 *
 * Purpose:     Returns the hdf5 file ID associated with the DBfile. The
 *              hdf5 file is not reopened; the caller should not close the
 *              file id.
 *
 * Return:      Success:        HDF5 file id
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL hid_t
db_hdf5_file(DBfile *_dbfile)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    return dbfile->fid;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_cwg
 *
 * Purpose:     Returns the hdf5 group ID for the current working group of
 *              the DBfile. The group is not reopened; the caller should not
 *              close the group ID.
 *
 * Return:      Success:        HDF5 current working group ID
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL hid_t
db_hdf5_cwg(DBfile *_dbfile)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    return dbfile->cwg;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_get_cmemb
 *
 * Purpose:     Returns information about a member of a compound type. If
 *              the member is an array then it returns information about
 *              the array instead.
 *
 * Return:      Success:        HDF5 datatype of member. The number of
 *                              items is returned in SIZE.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke, 2001-01-26
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL hid_t
db_hdf5_get_cmemb(hid_t compound_type, int membno, int *ndims/*out*/,
                  int size[3]/*out*/)
{
    hid_t       type;

    if ((type=H5Tget_member_type(compound_type, membno))<0) return -1;
    
#if (H5_VERS_MAJOR==1 && H5_VERS_MINOR>=4) || H5_VERS_MAJOR>1
    if (H5T_ARRAY==H5Tget_class(type)) {
        hsize_t bigdims[3];
        int i;
        *ndims = H5Tget_array_ndims(type);
        assert(*ndims<=3);
        H5Tget_array_dims(type, bigdims, NULL);
        for (i=0; i<*ndims; i++) size[i] = bigdims[i];
        type = H5Tget_super(type);
    } else {
        *ndims = 0;
    }
#else
    *ndims = H5Tget_member_dims(compound_type, membno, size, NULL);
#endif
    return type;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_put_cmemb
 *
 * Purpose:     
 *
 * Return:      Success:
 *                      
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke, 2001-01-26
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_put_cmemb(hid_t compound_type, const char *name, size_t offset,
                  int ndims, const int *dim, hid_t type)
{
    int         retval;
    
#if (H5_VERS_MAJOR==1 && H5_VERS_MINOR>=4) || H5_VERS_MAJOR>1
    if (ndims) {
        hsize_t bigdims[16];
        int i;
        for (i=0; i<ndims; i++) bigdims[i] = dim[i];
        type = H5Tarray_create(type, ndims, bigdims, NULL);
    }
    retval = H5Tinsert(compound_type, name, offset, type);
#else
    retval = H5Tinsert_array(compound_type, name, offset, ndims, dim, NULL,
                             type);
#endif
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    T_str
 *
 * Purpose:     Returns fixed-length hdf5 string data type which has just
 *              enough space to store the specified string.
 *
 * Return:      Success:        An hdf5 data type which will be closed on the
 *                              next call to this function.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE hid_t
T_str(char *s)
{
    static hid_t        stype = -1;

    if (!s || !*s) return -1;
    if (stype>=0) H5Tclose(stype);
    stype = H5Tcopy(H5T_C_S1);
    H5Tset_size(stype, strlen(s)+1);
    return stype;
}

/*-------------------------------------------------------------------------
 * Function:    silo_walk_cb 
 *
 * Purpose:     Error stack walk callback.
 *              Currently, only detects checksum errors.
 *
 * Return:      SUCCESS 
 *
 * Programmer:  Mark C. Miller 
 *              Tuesday, May 2, 2006 
 *
 *-------------------------------------------------------------------------
 */
PRIVATE herr_t
silo_walk_cb(int n, H5E_error_t *err_desc, void *client_data) 
{
    int *silo_error_code_p = (int *) client_data;

    if (strstr(err_desc->desc, "letcher32") != 0)
        *silo_error_code_p = E_CHECKSUM;

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    hdf5_to_silo_error 
 *
 * Purpose:     Convert HDF5 error stack to a silo error code. 
 *              Currently, only detects checksum errors.
 *
 * Return:      SUCCESS 
 *
 * Programmer:  Mark C. Miller 
 *              Tuesday, May 2, 2006 
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
hdf5_to_silo_error(const char *vname, const char *fname)
{
    int silo_error_code = E_NOERROR;

    H5Ewalk(H5E_WALK_UPWARD, silo_walk_cb, &silo_error_code);

    if (silo_error_code == E_NOERROR)
        silo_error_code = E_CALLFAIL;

    db_perror((char*)vname, silo_error_code, (char*)fname);
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_init
 *
 * Purpose:     One-time initializations for this driver.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Monday, March 22, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
db_hdf5_init(void)
{
    static int          ncalls;

    if (ncalls++) return;               /*already initialized*/

    /* Turn off error messages from the hdf5 library */
    H5Eset_auto(NULL, NULL);

    /* Define a scalar data space */
    SCALAR = H5Screate(H5S_SCALAR);

    /* Define atomic data types */
    T_char = H5T_NATIVE_UCHAR;
    T_short = H5T_NATIVE_SHORT;
    T_int = H5T_NATIVE_INT;
    T_long = H5T_NATIVE_LONG;
    T_float = H5T_NATIVE_FLOAT;
    T_double = H5T_NATIVE_DOUBLE;
    
    T_str256 = H5Tcopy(H5T_C_S1);       /*this is never freed!*/
    H5Tset_size(T_str256, 256);

    /* property lists necessary to support checksumming */
    P_ckcrprops = H5Pcreate(H5P_DATASET_CREATE); /* never freed */
    H5Pset_fletcher32(P_ckcrprops);

    /* for H5Dread calls, H5P_DEFAULT results in *enabled*
       checksums. So, we build the DISabled version here. */
    P_ckrdprops = H5Pcreate(H5P_DATASET_XFER);   /* never freed */
    H5Pset_edc_check(P_ckrdprops, H5Z_DISABLE_EDC);

    /* Define compound data types */
    STRUCT(DBcurve) {
        MEMBER_S(int,           npts);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        xvarname);
        MEMBER_S(str256,        yvarname);
        MEMBER_S(str256,        label);
        MEMBER_S(str256,        xlabel);
        MEMBER_S(str256,        ylabel);
        MEMBER_S(str256,        xunits);
        MEMBER_S(str256,        yunits);
        MEMBER_S(str256,        reference);
    } DEFINE;

    STRUCT(DBcsgmesh) {
        MEMBER_S(int,           block_no);
        MEMBER_S(int,           group_no);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           nbounds);
        MEMBER_S(int,           lcoeffs);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           origin);
        MEMBER_3(double,        min_extents);
        MEMBER_3(double,        max_extents);
        MEMBER_S(int,           guihide);
        MEMBER_R(str256,        units, 3);
        MEMBER_R(str256,        labels, 3);
        MEMBER_S(str256,        name);
        MEMBER_S(str256,        typeflags);
        MEMBER_S(str256,        bndids);
        MEMBER_S(str256,        coeffs);
        MEMBER_S(str256,        zonel_name);
        MEMBER_S(str256,        bndnames);
    } DEFINE;

    STRUCT(DBcsgvar) {
        MEMBER_S(int,           cycle);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           datatype);
        MEMBER_S(int,           nels);
        MEMBER_S(int,           nvals);
        MEMBER_S(int,           centering);
        MEMBER_S(int,           use_specmf);
        MEMBER_S(int,           ascii_labels);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        name);
        MEMBER_S(str256,        units);
        MEMBER_S(str256,        label);
        MEMBER_R(str256,        vals,           MAX_VARS);
        MEMBER_S(str256,        meshname);
    } DEFINE;

    STRUCT(DBcsgzonelist) {
        MEMBER_S(int,           nregs);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           lxform);
        MEMBER_S(int,           datatype);
        MEMBER_S(int,           nzones);
        MEMBER_S(int,           min_index);
        MEMBER_S(int,           max_index);
        MEMBER_S(str256,        typeflags);
        MEMBER_S(str256,        leftids);
        MEMBER_S(str256,        rightids);
        MEMBER_S(str256,        xform);
        MEMBER_S(str256,        zonelist);
        MEMBER_S(str256,        regnames);
        MEMBER_S(str256,        zonenames);
    } DEFINE;

    STRUCT(DBdefvars) {
        MEMBER_S(int,           ndefs);
        MEMBER_S(str256,        names);
        MEMBER_S(str256,        types);
        MEMBER_S(str256,        defns);
        MEMBER_S(str256,        guihides);
    } DEFINE;

    STRUCT(DBquadmesh) {
        MEMBER_R(str256,        coord,          3);
        MEMBER_3(double,        min_extents);
        MEMBER_3(double,        max_extents);
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           coordtype);
        MEMBER_S(int,           nspace);
        MEMBER_S(int,           nnodes);
        MEMBER_S(int,           facetype);
        MEMBER_S(int,           major_order);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           coord_sys);
        MEMBER_S(int,           planar);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           group_no);
        MEMBER_3(int,           dims);
        MEMBER_3(int,           min_index);
        MEMBER_3(int,           max_index);
        MEMBER_3(int,           baseindex);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           guihide);
        MEMBER_R(str256,        label,          3);
        MEMBER_R(str256,        units,          3);
    } DEFINE;
    
    STRUCT(DBquadvar) {
        MEMBER_R(str256,        value,          MAX_VARS);
        MEMBER_R(str256,        mixed_value,    MAX_VARS);
        MEMBER_S(str256,        meshid);
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nvals);
        MEMBER_S(int,           nels);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           mixlen);
        MEMBER_S(int,           major_order);
        MEMBER_S(int,           datatype);
        MEMBER_S(int,           cycle);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           use_specmf);
        MEMBER_S(int,           ascii_labels);
        MEMBER_3(int,           dims);
        MEMBER_3(int,           zones);
        MEMBER_3(int,           min_index);
        MEMBER_3(int,           max_index);
        MEMBER_3(float,         align);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        label);
        MEMBER_S(str256,        units);
    } DEFINE;

    STRUCT(DBucdmesh) {
        MEMBER_R(str256,        coord,          3);
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nnodes);
        MEMBER_S(int,           nzones);
        MEMBER_S(int,           facetype);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           coord_sys);
        MEMBER_S(int,           topo_dim);
        MEMBER_S(int,           planar);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           group_no);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(str256,        facelist);
        MEMBER_S(str256,        zonelist);
        MEMBER_S(str256,        phzonelist);
        MEMBER_S(str256,        gnodeno);
        MEMBER_3(double,        min_extents);
        MEMBER_3(double,        max_extents);
        MEMBER_S(int,           guihide);
        MEMBER_R(str256,        label,          3);
        MEMBER_R(str256,        units,          3);
    } DEFINE;
    
    STRUCT(DBucdvar) {
        MEMBER_R(str256,        value,          MAX_VARS);
        MEMBER_R(str256,        mixed_value,    MAX_VARS);
        MEMBER_S(str256,        meshid);
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nvals);
        MEMBER_S(int,           nels);
        MEMBER_S(int,           centering);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           mixlen);
        MEMBER_S(int,           datatype);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           use_specmf);
        MEMBER_S(int,           ascii_labels);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           lo_offset);
        MEMBER_S(int,           hi_offset);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        label);
        MEMBER_S(str256,        units);
    } DEFINE;

    STRUCT(DBfacelist) {
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nfaces);
        MEMBER_S(int,           nshapes);
        MEMBER_S(int,           ntypes);
        MEMBER_S(int,           lnodelist);
        MEMBER_S(int,           origin);
        MEMBER_S(str256,        nodelist);
        MEMBER_S(str256,        shapecnt);
        MEMBER_S(str256,        shapesize);
        MEMBER_S(str256,        typelist);
        MEMBER_S(str256,        types);
        MEMBER_S(str256,        zoneno);
    } DEFINE;

    STRUCT(DBzonelist) {
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nzones);
        MEMBER_S(int,           nshapes);
        MEMBER_S(int,           lnodelist);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           lo_offset);
        MEMBER_S(int,           hi_offset);
        MEMBER_S(str256,        nodelist);
        MEMBER_S(str256,        shapecnt);
        MEMBER_S(str256,        shapesize);
        MEMBER_S(str256,        shapetype);
        MEMBER_S(str256,        gzoneno);
    } DEFINE;

    STRUCT(DBphzonelist) {
        MEMBER_S(int,           nfaces);
        MEMBER_S(int,           lnodelist);
        MEMBER_S(int,           nzones);
        MEMBER_S(int,           lfacelist);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           lo_offset);
        MEMBER_S(int,           hi_offset);
        MEMBER_S(str256,        nodecnt);
        MEMBER_S(str256,        nodelist);
        MEMBER_S(str256,        extface);
        MEMBER_S(str256,        facecnt);
        MEMBER_S(str256,        facelist);
    } DEFINE;

    STRUCT(DBmaterial) {
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nmat);
        MEMBER_S(int,           mixlen);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           major_order);
        MEMBER_S(int,           datatype);
        MEMBER_3(int,           dims);
        MEMBER_S(int,           allowmat0);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        meshid);
        MEMBER_S(str256,        matlist);
        MEMBER_S(str256,        matnos);
        MEMBER_S(str256,        mix_vf);
        MEMBER_S(str256,        mix_next);
        MEMBER_S(str256,        mix_mat);
        MEMBER_S(str256,        mix_zone);
        MEMBER_S(str256,        matnames);
        MEMBER_S(str256,        matcolors);
    } DEFINE;

    STRUCT(DBmultimesh) {
        MEMBER_S(int,           nblocks);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           ngroups);
        MEMBER_S(int,           blockorigin);
        MEMBER_S(int,           grouporigin);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           extentssize);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        meshtypes);
        MEMBER_S(str256,        meshnames);
        MEMBER_S(str256,        extents);
        MEMBER_S(str256,        zonecounts);
        MEMBER_S(str256,        has_external_zones);
        MEMBER_S(int,           lgroupings);
        MEMBER_S(str256,        groupings);
        MEMBER_S(str256,        groupnames);
    } DEFINE;

    STRUCT(DBmultimeshadj) {
        MEMBER_S(int,           nblocks);
        MEMBER_S(int,           blockorigin);
        MEMBER_S(int,           lneighbors);
        MEMBER_S(int,           totlnodelists);
        MEMBER_S(int,           totlzonelists);
        MEMBER_S(str256,        meshtypes);
        MEMBER_S(str256,        nneighbors);
        MEMBER_S(str256,        neighbors);
        MEMBER_S(str256,        back);
        MEMBER_S(str256,        lnodelists);
        MEMBER_S(str256,        nodelists);
        MEMBER_S(str256,        lzonelists);
        MEMBER_S(str256,        zonelists);
    } DEFINE;

    STRUCT(DBmultivar) {
        MEMBER_S(int,           nvars);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           ngroups);
        MEMBER_S(int,           blockorigin);
        MEMBER_S(int,           grouporigin);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           extentssize);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        vartypes);
        MEMBER_S(str256,        varnames);
        MEMBER_S(str256,        extents);
    } DEFINE;

    STRUCT(DBmultimat) {
        MEMBER_S(int,           nmats);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           ngroups);
        MEMBER_S(int,           blockorigin);
        MEMBER_S(int,           grouporigin);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           allowmat0);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        matnames);
        MEMBER_S(str256,        matnos);
        MEMBER_S(str256,        mixlens);
        MEMBER_S(str256,        matcounts);
        MEMBER_S(str256,        matlists);
        MEMBER_S(int,           nmatnos);
        MEMBER_S(str256,        material_names);
        MEMBER_S(str256,        mat_colors);
    } DEFINE;

    STRUCT(DBmultimatspecies) {
        MEMBER_S(int,           nspec);
        MEMBER_S(int,           nmat);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           ngroups);
        MEMBER_S(int,           blockorigin);
        MEMBER_S(int,           grouporigin);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        specnames);
        MEMBER_S(str256,        nmatspec);
        MEMBER_S(str256,        matname);
    } DEFINE;

    STRUCT(DBmatspecies) {
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nmat);
        MEMBER_S(int,           nspecies_mf);
        MEMBER_S(int,           mixlen);
        MEMBER_S(int,           major_order);
        MEMBER_S(int,           datatype);
        MEMBER_3(int,           dims);
        MEMBER_S(int,           guihide);
        MEMBER_S(str256,        matname);
        MEMBER_S(str256,        speclist);
        MEMBER_S(str256,        nmatspec);
        MEMBER_S(str256,        species_mf);
        MEMBER_S(str256,        mix_speclist);
    } DEFINE;

    STRUCT(DBpointmesh) {
        MEMBER_S(int,           ndims);
        MEMBER_S(int,           nspace);
        MEMBER_S(int,           nels);
        MEMBER_S(int,           cycle);
        MEMBER_S(int,           group_no);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           min_index);
        MEMBER_S(int,           max_index);
        MEMBER_3(double,        min_extents);
        MEMBER_3(double,        max_extents);
        MEMBER_S(int,           guihide);
        MEMBER_R(str256,        coord,          3);
        MEMBER_R(str256,        label,          3);
        MEMBER_R(str256,        units,          3);
        MEMBER_S(str256,        gnodeno);
    } DEFINE;

    STRUCT(DBpointvar) {
        MEMBER_S(int,           nvals);
        MEMBER_S(int,           nels);
        MEMBER_S(int,           nspace);
        MEMBER_S(int,           origin);
        MEMBER_S(int,           min_index);
        MEMBER_S(int,           max_index);
        MEMBER_S(int,           datatype);
        MEMBER_S(int,           cycle);
        MEMBER_S(float,         time);
        MEMBER_S(double,        dtime);
        MEMBER_S(int,           guihide);
        MEMBER_S(int,           ascii_labels);
        MEMBER_S(str256,        meshid);
        MEMBER_S(str256,        label);
        MEMBER_S(str256,        units);
        MEMBER_R(str256,        data,           MAX_VARS);
    } DEFINE;

    STRUCT(DBcompoundarray) {
        MEMBER_S(int,           nelems);
        MEMBER_S(int,           nvalues);
        MEMBER_S(int,           datatype);
        MEMBER_S(str256,        values);
        MEMBER_S(str256,        elemnames);
        MEMBER_S(str256,        elemlengths);
    } DEFINE;
    
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_InitCallbacks
 *
 * Purpose:     Initialize the callbacks in a DBfile structure.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *    Jeremy Meredith, Wed Oct 25 16:16:59 PDT 2000
 *    Added DB_INTEL so we had a little-endian target.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
db_hdf5_InitCallbacks(DBfile *_dbfile, int target)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_InitCallbacks";
    
    /* Initialize the driver global data structures */
    db_hdf5_init();

    /* Target data types */
    switch (target) {
    case DB_LOCAL:
        dbfile->T_char   = T_char;
        dbfile->T_short  = T_short;
        dbfile->T_int    = T_int;
        dbfile->T_long   = T_long;
        dbfile->T_float  = T_float;
        dbfile->T_double = T_double;
        dbfile->T_str    = T_str;
        break;

    case DB_SGI:
    case DB_SUN3:
    case DB_SUN4:
    case DB_RS6000:
        dbfile->T_char   = H5T_STD_I8BE;
        dbfile->T_short  = H5T_STD_I16BE;
        dbfile->T_int    = H5T_STD_I32BE;
        dbfile->T_long   = H5T_STD_I32BE;
        dbfile->T_float  = H5T_IEEE_F32BE;
        dbfile->T_double = H5T_IEEE_F64BE;
        dbfile->T_str    = T_str;
        break;

    case DB_CRAY:
        dbfile->T_char   = H5T_STD_I8BE;
        dbfile->T_short  = H5T_STD_I64BE;
        dbfile->T_int    = H5T_STD_I64BE;
        dbfile->T_long   = H5T_STD_I64BE;
        dbfile->T_float  = H5T_IEEE_F64BE; /*assuming new cray*/
        dbfile->T_double = H5T_IEEE_F64BE; /*assuming new cray*/
        dbfile->T_str    = T_str;
        break;

    case DB_INTEL:
        dbfile->T_char   = H5T_STD_I8LE;
        dbfile->T_short  = H5T_STD_I16LE;
        dbfile->T_int    = H5T_STD_I32LE;
        dbfile->T_long   = H5T_STD_I32LE;
        dbfile->T_float  = H5T_IEEE_F32LE;
        dbfile->T_double = H5T_IEEE_F64LE;
        dbfile->T_str    = T_str;
        break;

    default:
        db_perror("target data type", E_BADARGS, me);
        return;
    }

    /* Properties of the driver */
    dbfile->pub.pathok = TRUE;

    /* File operations */
    dbfile->pub.close = db_hdf5_Close;
    dbfile->pub.module = db_hdf5_Filters;

    /* Directory operations */
    dbfile->pub.cd = db_hdf5_SetDir;
    dbfile->pub.g_dir = db_hdf5_GetDir;
    dbfile->pub.newtoc = db_hdf5_NewToc;
    dbfile->pub.cdid = NULL;            /*DBSetDirID() not supported    */
    dbfile->pub.mkdir = db_hdf5_MkDir;

    /* Variable inquiries */
    dbfile->pub.exist = db_hdf5_InqVarExists;
    dbfile->pub.g_varlen = db_hdf5_GetVarLength;
    dbfile->pub.g_varbl = db_hdf5_GetVarByteLength;
    dbfile->pub.g_vartype = db_hdf5_GetVarType;
    dbfile->pub.g_vardims = db_hdf5_GetVarDims;
    dbfile->pub.r_var1 = NULL;          /*DBReadVar1() not supported    */
    dbfile->pub.g_attr = NULL;          /*DBGetAtt() not implemented yet*/
    dbfile->pub.r_att = NULL;           /*DBReadAtt() not implemented yet*/

    /* Variable I/O operations */
    dbfile->pub.g_var = db_hdf5_GetVar;
    dbfile->pub.r_var = db_hdf5_ReadVar;
    dbfile->pub.r_varslice = db_hdf5_ReadVarSlice;
    dbfile->pub.write = db_hdf5_Write;
    dbfile->pub.writeslice = db_hdf5_WriteSlice;

    /* Low-level object functions */
    dbfile->pub.g_obj = db_hdf5_GetObject;
    dbfile->pub.inqvartype = db_hdf5_InqVarType;
    dbfile->pub.i_meshtype = db_hdf5_InqVarType; /*yes, Vartype*/
    dbfile->pub.i_meshname = db_hdf5_InqMeshName;
    dbfile->pub.g_comp = db_hdf5_GetComponent;
    dbfile->pub.g_comptyp = db_hdf5_GetComponentType;
    dbfile->pub.g_compnames = db_hdf5_GetComponentNames;
    dbfile->pub.c_obj = db_hdf5_WriteObject; /*DBChangeObject==DBWriteObject*/
    dbfile->pub.w_obj = db_hdf5_WriteObject;
    dbfile->pub.w_comp = db_hdf5_WriteComponent;
    
    /* Curve functions */
    dbfile->pub.g_cu = db_hdf5_GetCurve;
    dbfile->pub.p_cu = db_hdf5_PutCurve;

    /* CSG mesh functions */
    dbfile->pub.p_csgm = db_hdf5_PutCsgmesh;
    dbfile->pub.g_csgm = db_hdf5_GetCsgmesh;
    dbfile->pub.p_csgzl = db_hdf5_PutCSGZonelist;
    dbfile->pub.g_csgzl = db_hdf5_GetCSGZonelist;
    dbfile->pub.p_csgv = db_hdf5_PutCsgvar;
    dbfile->pub.g_csgv = db_hdf5_GetCsgvar;

    /* Defvars functions */
    dbfile->pub.g_defv = db_hdf5_GetDefvars;
    dbfile->pub.p_defv = db_hdf5_PutDefvars;

    /* Quadmesh functions */
    dbfile->pub.g_qm = db_hdf5_GetQuadmesh;
    dbfile->pub.g_qv = db_hdf5_GetQuadvar;
    dbfile->pub.p_qm = db_hdf5_PutQuadmesh;
    dbfile->pub.p_qv = db_hdf5_PutQuadvar;

    /* Unstructured mesh functions */
    dbfile->pub.g_um = db_hdf5_GetUcdmesh;
    dbfile->pub.g_uv = db_hdf5_GetUcdvar;
    dbfile->pub.g_fl = db_hdf5_GetFacelist;
    dbfile->pub.g_zl = db_hdf5_GetZonelist;
    dbfile->pub.g_phzl = db_hdf5_GetPHZonelist;
    dbfile->pub.p_um = db_hdf5_PutUcdmesh;
    dbfile->pub.p_sm = db_hdf5_PutUcdsubmesh;
    dbfile->pub.p_uv = db_hdf5_PutUcdvar;
    dbfile->pub.p_fl = db_hdf5_PutFacelist;
    dbfile->pub.p_zl = db_hdf5_PutZonelist;
    dbfile->pub.p_zl2 = db_hdf5_PutZonelist2;
    dbfile->pub.p_phzl = db_hdf5_PutPHZonelist;

    /* Material functions */
    dbfile->pub.g_ma = db_hdf5_GetMaterial;
    dbfile->pub.g_ms = db_hdf5_GetMatspecies;
    dbfile->pub.p_ma = db_hdf5_PutMaterial;
    dbfile->pub.p_ms = db_hdf5_PutMatspecies;

    /* Pointmesh functions */
    dbfile->pub.g_pm = db_hdf5_GetPointmesh;
    dbfile->pub.g_pv = db_hdf5_GetPointvar;
    dbfile->pub.p_pm = db_hdf5_PutPointmesh;
    dbfile->pub.p_pv = db_hdf5_PutPointvar;

    /* Multiblock functions */
    dbfile->pub.g_mm = db_hdf5_GetMultimesh;
    dbfile->pub.g_mmadj = db_hdf5_GetMultimeshadj;
    dbfile->pub.g_mv = db_hdf5_GetMultivar;
    dbfile->pub.g_mt = db_hdf5_GetMultimat;
    dbfile->pub.g_mms = db_hdf5_GetMultimatspecies;
    dbfile->pub.p_mm = db_hdf5_PutMultimesh;
    dbfile->pub.p_mmadj = db_hdf5_PutMultimeshadj;
    dbfile->pub.p_mv = db_hdf5_PutMultivar;
    dbfile->pub.p_mt = db_hdf5_PutMultimat;
    dbfile->pub.p_mms = db_hdf5_PutMultimatspecies;

    /* Compound arrays */
    dbfile->pub.g_ca = db_hdf5_GetCompoundarray;
    dbfile->pub.p_ca = db_hdf5_PutCompoundarray;
}


/*-------------------------------------------------------------------------
 * Function:    build_fspace
 *
 * Purpose:     Build a file data space selection based on the silo OFFSET,
 *              LENGTH, and STRIDE. The COUNT for hdf5 hyperslabs is the
 *              number of times to stride whereas the LENGTH argument
 *              passed in is the number of elements over which we stride.
 *              An example:
 *              
 *                0 1 2 3 4 5 6 7 8 9 10
 *               +-+-+-+-+-+-+-+-+-+-+-+  Silo: offset=2, stride=2, length=7
 *               | | |X| |X| |X| |X| | |  HDF5: offset=2, stride=2, count=4
 *               +-+-+-+-+-+-+-+-+-+-+-+
 *
 * Return:      Success:        Data space to be closed later. If SIZE is
 *                              non-null then the number of elements selected
 *                              in each dimension is written to that array.
 *
 *              Failure:        Negative, SIZE is undefined.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE hid_t
build_fspace(hid_t dset, int ndims, int *offset, int *length, int *stride,
             hsize_t *size/*out*/)
{
    hid_t       fspace = -1;
    int         i;
    hssize_t    hs_offset[H5S_MAX_RANK];
    hsize_t     hs_count[H5S_MAX_RANK], hs_stride[H5S_MAX_RANK];
    
    if (ndims>H5S_MAX_RANK) return -1;
    for (i=0; i<ndims; i++) {
        hs_offset[i] = offset[i];
        hs_stride[i] = stride[i];
        
        if (stride[i]) {
            hs_count[i] = (length[i]+stride[i]-1)/stride[i];
        } else {
            hs_count[i] = 1;
        }
        if (size) size[i] = hs_count[i];
    }

    if ((fspace=H5Dget_space(dset))<0) return -1;
    if (H5Sselect_hyperslab(fspace, H5S_SELECT_SET, hs_offset, hs_stride,
                            hs_count, NULL)<0) {
        H5Sclose(fspace);
        return -1;
    }
    return fspace;
}


/*-------------------------------------------------------------------------
 * Function:    silom2hdfm_type
 *
 * Purpose:     Return the hdf5 data type for memory that is equivalent to
 *              the specified silo memory data type.
 *
 * Return:      Success:        An hdf5 data type. Do not close it (well, if
 *                              you must, but you'll get an error from hdf5).
 *
 *              Failure:        Negative
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE hid_t
silom2hdfm_type(int datatype)
{
    hid_t       mtype = -1;

    switch (datatype) {
    case DB_INT:
        mtype = H5T_NATIVE_INT;
        break;
    case DB_SHORT:
        mtype = H5T_NATIVE_SHORT;
        break;
    case DB_LONG:
        mtype = H5T_NATIVE_LONG;
        break;
    case DB_FLOAT:
        mtype = H5T_NATIVE_FLOAT;
        break;
    case DB_DOUBLE:
        mtype = H5T_NATIVE_DOUBLE;
        break;
    case DB_CHAR:
        mtype = H5T_NATIVE_UCHAR;
        break;
    }
    return mtype;
}


/*-------------------------------------------------------------------------
 * Function:    silof2hdff_type
 *
 * Purpose:     Return the hdf5 data type for the file that is equivalent to
 *              the specified silo file data type.
 *
 * Return:      Success:        An hdf5 data type. Do not close this type.
 *
 *              Failure:        Negative
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 15, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE hid_t
silof2hdff_type(DBfile_hdf5 *dbfile, int datatype)
{
    hid_t       ftype = -1;

    switch (datatype) {
    case DB_INT:
        ftype = dbfile->T_int;
        break;
    case DB_SHORT:
        ftype = dbfile->T_short;
        break;
    case DB_LONG:
        ftype = dbfile->T_long;
        break;
    case DB_FLOAT:
        ftype = dbfile->T_float;
        break;
    case DB_DOUBLE:
        ftype = dbfile->T_double;
        break;
    case DB_CHAR:
        ftype = dbfile->T_char;
        break;
    }
    return ftype;
}


/*-------------------------------------------------------------------------
 * Function:    hdf2silo_type
 *
 * Purpose:     Given an HDF5 file data type return and appropriate memory
 *              silo data type.
 *
 * Return:      Success:        Silo data type
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Monday, April 12, 1999
 *
 * Modifications:
 *
 *   Mark C. Miler, made it return DB_FLOAT or DB_DOUBLE based on
 *   data type passed in and NOT on current forceSingle mode
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
hdf2silo_type(hid_t type)
{
    size_t      size = H5Tget_size(type);
    int         retval = -1;
    
    switch (H5Tget_class(type)) {
    case H5T_INTEGER:
        if (sizeof(char)>=size) {
            retval = DB_CHAR;
        } else if (sizeof(int)!=sizeof(short) && sizeof(short)>=size) {
            retval = DB_SHORT;
        } else if (sizeof(int)>=size) {
            retval = DB_INT;
        } else {
            retval = DB_LONG;
        }
        break;

    case H5T_FLOAT:
        if (sizeof(double)!=sizeof(float) && sizeof(float)>=size) {
            retval = DB_FLOAT;
        } else if (sizeof(double)>=size) {
            retval = DB_DOUBLE;
        }
        break;

    default:
        /* Silo doesn't handle other types */
        break;
    }
    return retval;
}


/*-------------------------------------------------------------------------
 * Function:    silo2silo_type
 *
 * Purpose:     Translate a file silo data type to a memory silo data type.
 *              If the file data type is zero then we use either DB_FLOAT or
 *              DB_DOUBLE.
 *
 * Return:      Success:        silo data type
 *
 *              Failure:        never fails
 *
 * Programmer:  Robb Matzke
 *              Monday, April 12, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
silo2silo_type(int datatype)
{
    switch (datatype) {
    case DB_CHAR:
        return DB_CHAR;
    case 0:
    case DB_FLOAT:
    case DB_DOUBLE:
        return force_single_g ? DB_FLOAT : DB_DOUBLE;
    }
    return DB_INT;
}


/*-------------------------------------------------------------------------
 * Function:    hdf2hdf_type
 *
 * Purpose:     Given a file data type choose an appropriate memory data
 *              type.
 *
 * Return:      Success:        Memory data type which should not be closed
 *                              later (well, if you must, but hdf5 will
 *                              report an error).
 *
 *              Failure:        Negative
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   Made it return NATIVE_FLOAT or NATIVE_DOUBLE NOT depending on
 *   current force_single_g mode
 *
 *-------------------------------------------------------------------------
 */
PRIVATE hid_t
hdf2hdf_type(hid_t ftype)
{
    hid_t       mtype=-1;
    
    switch (H5Tget_class(ftype)) {
    case H5T_INTEGER:
        if (sizeof(char)>=H5Tget_size(ftype)) {
            mtype = H5T_NATIVE_UCHAR;
        } else if (sizeof(short)>=H5Tget_size(ftype)) {
            mtype = H5T_NATIVE_SHORT;
        } else if (sizeof(int)>=H5Tget_size(ftype)) {
            mtype = H5T_NATIVE_INT;
        } else {
            mtype = H5T_NATIVE_LONG;
        }
        break;
    case H5T_FLOAT:
        if (sizeof(double)!=sizeof(float) && sizeof(float)>=H5Tget_size(ftype)) {
            mtype = H5T_NATIVE_FLOAT;
        } else if (sizeof(double)>=H5Tget_size(ftype)) {
            mtype = H5T_NATIVE_DOUBLE;
        } else {
            mtype = H5T_NATIVE_FLOAT;
        }
        break;
    default:
        break;
    }
    return mtype;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_get_comp_var 
 *
 * Purpose:     Given a file handle and varname containing at least one
 *              underscore, attempts to get size and, optionally, read
 *              the corresponding HDF5 datatype name and member
 *              name pair, if it exists, formed by splitting the given
 *              varname at the underscore character. If multiple splits
 *              could possibly match with an HDF5 datatype name and member
 *              name pair in the file, only the first found is returned.
 *
 *              This function is necessary to support the old pdb-ish way
 *              of permitting a Silo client to request raw data arrays
 *              for the component of an object by the naming convention
 *              "<objname>_<compname>"
 *
 * Return:      If a valid split exists:                1
 *
 *              If a valid split does not exist:        0 
 *
 * Programmer:  Mark C. Miller 
 *              October 11, 2005 
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_get_comp_var(hid_t fileid, const char *name, hsize_t *nelmts,
    size_t *elsize, hid_t *datatype, void **buf)
{
    hid_t typeid = -1, stypeid = -1, attr = -1, comptype, memtype;
    int membno = -1;
    int retval = 0;
    hsize_t numvals;
    size_t valsize;

    /* loop trying different typename, member name combinations */
    char *tmpname = STRDUP(name);
    char *p = strrchr(tmpname, '_');
    char *typename, *memname;
    while (p != 0 && *p != '\0')
    {
        char *tmpp = p;
        *p = '\0';
        typename = tmpname;
        memname = p+1;

        stypeid = attr = typeid = -1;
        if ((typeid=H5Topen(fileid, typename))>=0 &&
            (attr=H5Aopen_name(typeid, "silo"))>=0 &&
            (stypeid=H5Aget_type(attr))>=0 &&
            (membno=H5Tget_member_index(stypeid, memname))>=0)
            retval = 1;

        if (retval == 1) break;

        if (attr != -1) H5Aclose(attr);
        if (stypeid != -1) H5Tclose(stypeid);
        if (typeid != -1) H5Tclose(typeid);

        p = strrchr(tmpname, '_');
        *tmpp = '_';
    }

    if (retval == 1)
    {
        hid_t mbtype = H5Tget_member_type(stypeid, membno);
        H5T_class_t mbclass = H5Tget_class(mbtype);

        switch (mbclass) {
            case H5T_INTEGER:
                numvals = 1;
                comptype = H5T_NATIVE_INT;
                break;
            case H5T_FLOAT:
                numvals = 1;
                comptype = H5T_NATIVE_FLOAT;
                break;
            case H5T_ARRAY:
                {
                    int i, ndims, size[3], len = 1;
                    comptype = db_hdf5_get_cmemb(stypeid, membno, &ndims, size);
                    for (i = 0; i < ndims; i++)
                        len *= size[i];
                    numvals = len;
                    break;
                }
            case H5T_STRING:
                numvals = 1;
                comptype = T_str256; /* it may be an indirect dataset */
                break;
            default:
                numvals = 0;
                break;
        }
        valsize = H5Tget_size(comptype);

        /* note, a comptype of T_str256 means either that the component
           contains the name of a dataset (indirect) case which we will
           need to read or the component is some other 256 character
           string */

        /* read the component data only if caller requested it */
        if ((numvals && buf) || comptype == T_str256)
        {
            char tmp[256];

            if (comptype != T_str256 && *buf == 0)
                *buf = malloc(*nelmts * *elsize);

            /* create a component type with just one member,
               the one we're interested in */
            memtype = H5Tcreate(H5T_COMPOUND, H5Tget_size(comptype));
            H5Tinsert(memtype, H5Tget_member_name(stypeid, membno), 0, comptype);

            /* read attribute for the silo object data */
            H5Aread(attr, memtype, comptype==T_str256?tmp:*buf);
            H5Tclose(memtype);

            /* do the indirection if necessary */
            if (comptype == T_str256 &&
                strncmp(tmp,"/.silo/#",8) == 0) /* indirect case */
            {
                hid_t d, fspace, ftype, mtype;

                d = H5Dopen(fileid, tmp);
                fspace = H5Dget_space(d);
                numvals = H5Sget_simple_extent_npoints(fspace);
                ftype = H5Dget_type(d);
                mtype = hdf2hdf_type(ftype);
                valsize = H5Tget_size(mtype);
                comptype = mtype;
                if (buf)
                {
                    if (*buf == 0)
                        *buf = malloc(numvals*H5Tget_size(mtype));

                    P_rdprops = H5P_DEFAULT;
                    if (!SILO_Globals.enableChecksums)
                        P_rdprops = P_ckrdprops;

                    if (H5Dread(d, mtype, H5S_ALL, H5S_ALL, P_rdprops, *buf)<0) {
                        hdf5_to_silo_error(name, "db_hdf5_get_comp_var");
                        retval = 0;
                    }
                }
                H5Tclose(ftype);
                H5Dclose(d);
                H5Sclose(fspace);
            }
            else if (comptype == T_str256) /* other string case */
            {
                if (buf)
                {
                    int n = strlen(tmp)+1;
                    if (*buf == 0)
                        *buf = malloc(n);
                    strncpy(*buf, tmp, n);
                    valsize = n;
                }
            }
            else
            {
                /* any other case, the component's data will have already
                   been read in to *buf in the H5Aread call, above */
            }
        }

        if (retval == 1)
        {
            if (nelmts)
                *nelmts = numvals;
            if (elsize)
                *elsize = valsize; 
            if (datatype)
                *datatype = comptype;
        }
    }

    if (attr != -1)    H5Aclose(attr);
    if (stypeid != -1) H5Tclose(stypeid);
    if (typeid != -1)  H5Tclose(typeid);

    FREE(tmpname);
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    load_toc
 *
 * Purpose:     Add an object to the table of contents
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb  1 13:48:33 PST 2005
 *   Made it deal with case of QUAD_RECT or QUAD_CURV
 *
 *-------------------------------------------------------------------------
 */
PRIVATE herr_t
load_toc(hid_t grp, const char *name, void *_toc)
{
    DBtoc               *toc = (DBtoc*)_toc;
    H5G_stat_t          sb;
    DBObjectType        objtype = DB_INVALID_OBJECT;
    int                 n, *nvals=NULL, _objtype;
    char                ***names=NULL;
    hid_t               obj=-1, attr=-1;

    if (H5Gget_objinfo(grp, name, TRUE, &sb)<0) return -1;
    switch (sb.type) {
    case H5G_GROUP:
        /*
         * Any group which has a `..' entry is a silo directory. The `..'
         * names do not appear in the silo table of contents.
         */
        if (!strcmp(name, "..") || (obj=H5Gopen(grp, name))<0) break;
        H5E_BEGIN_TRY {
            if (H5Gget_objinfo(obj, "..", FALSE, NULL)>=0) objtype = DB_DIR;
        } H5E_END_TRY;
        H5Gclose(obj);
        break;

    case H5G_TYPE:
        if ((obj=H5Topen(grp, name))<0) break;
        if ((attr=H5Aopen_name(obj, "silo_type"))<0) break;
        if (H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0) break;
        objtype = (DBObjectType)_objtype;
        H5Aclose(attr);
        H5Tclose(obj);
        break;

    case H5G_DATASET:
        objtype = DB_VARIABLE;
        break;

    default:
        /*ignore*/
        break;
    }

    /* What table of contents field does this object belong to? */
    switch (objtype) {
    case DB_INVALID_OBJECT:
        break;
    case DB_QUADMESH:
    case DB_QUAD_RECT:
    case DB_QUAD_CURV:
        names = &(toc->qmesh_names);
        nvals = &(toc->nqmesh);
        break;
    case DB_QUADVAR:
        names = &(toc->qvar_names);
        nvals = &(toc->nqvar);
        break;
    case DB_CSGMESH:
        names = &(toc->csgmesh_names);
        nvals = &(toc->ncsgmesh);
        break;
    case DB_CSGVAR:
        names = &(toc->csgvar_names);
        nvals = &(toc->ncsgvar);
        break;
    case DB_UCDMESH:
        names = &(toc->ucdmesh_names);
        nvals = &(toc->nucdmesh);
        break;
    case DB_UCDVAR:
        names = &(toc->ucdvar_names);
        nvals = &(toc->nucdvar);
        break;
    case DB_MULTIMESH:
        names = &(toc->multimesh_names);
        nvals = &(toc->nmultimesh);
        break;
    case DB_MULTIMESHADJ:
        names = &(toc->multimeshadj_names);
        nvals = &(toc->nmultimeshadj);
        break;
    case DB_MULTIVAR:
        names = &(toc->multivar_names);
        nvals = &(toc->nmultivar);
        break;
    case DB_MULTIMAT:
        names = &(toc->multimat_names);
        nvals = &(toc->nmultimat);
        break;
    case DB_MULTIMATSPECIES:
        names = &(toc->multimatspecies_names);
        nvals = &(toc->nmultimatspecies);
        break;
    case DB_MATERIAL:
        names = &(toc->mat_names);
        nvals = &(toc->nmat);
        break;
    case DB_MATSPECIES:
        names = &(toc->matspecies_names);
        nvals = &(toc->nmatspecies);
        break;
    case DB_CURVE:
        names = &(toc->curve_names);
        nvals = &(toc->ncurve);
        break;
    case DB_DEFVARS:
        names = &(toc->defvars_names);
        nvals = &(toc->ndefvars);
        break;
    case DB_POINTMESH:
        names = &(toc->ptmesh_names);
        nvals = &(toc->nptmesh);
        break;
    case DB_POINTVAR:
        names = &(toc->ptvar_names);
        nvals = &(toc->nptvar);
        break;
    case DB_ARRAY:
        names = &(toc->array_names);
        nvals = &(toc->narrays);
        break;
    case DB_DIR:
        names = &(toc->dir_names);
        nvals = &(toc->ndir);
        break;
    case DB_VARIABLE:
        names = &(toc->var_names);
        nvals = &(toc->nvar);
        break;
    case DB_USERDEF:  /*fall through*/
    case DB_FACELIST: /*fall through*/
    case DB_EDGELIST: /*fall through*/
    case DB_ZONELIST:
    case DB_PHZONELIST:
    case DB_CSGZONELIST:
        names = &(toc->obj_names);
        nvals = &(toc->nobj);
        break;
    }

    /* Append to table of contents */
    if (names && nvals) {
        n = (*nvals)++;
        *names = realloc(*names, *nvals*sizeof(char*));
        (*names)[n] = STRDUP(name);
    }

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    find_objno
 *
 * Purpose:     Determines if the specified object has the same object ID as
 *              what is stored in comp->objno and if so copies NAME to
 *              comp->name.
 *
 * Return:      Success:        1 if objno's are the same
 *
 *              Failure:        0
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE herr_t
find_objno(hid_t grp, const char *name, void *_comp)
{
    silo_hdf5_comp_t    *comp = (silo_hdf5_comp_t*)_comp;
    H5G_stat_t          sb;

    if (H5Gget_objinfo(grp, name, TRUE, &sb)<0) return -1;
    if (sb.objno[0]!=comp->objno[0] || sb.objno[1]!=comp->objno[1]) return 0;
    comp->name = STRDUP(name);
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_compname
 *
 * Purpose:     Returns a new name relative to the link directory. The name
 *              is generated by reading the `nlinks' attribute of the link
 *              directory, incrementing it, creating a file name, and saving
 *              it back to the file.
 *
 * Return:      Success:        0, A new link name not more than 8 characters
 *                              long counting the null terminator is returned
 *                              through the NAME argument.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_hdf5_compname(DBfile_hdf5 *dbfile, char name[8]/*out*/)
{
    static char *me = "db_hdf5_compname";
    hid_t       attr=-1;
    int         nlinks;

    PROTECT {
        /* Open or create the `nlinks' attribute of the link group */
        H5E_BEGIN_TRY {
            attr = H5Aopen_name(dbfile->link, "nlinks");
        } H5E_END_TRY;
        if (attr<0 && (attr=H5Acreate(dbfile->link, "nlinks", H5T_NATIVE_INT,
                                      SCALAR, H5P_DEFAULT))<0) {
            db_perror("nlinks attribute", E_CALLFAIL, me);
            UNWIND();
        }

        /* Increment the nlinks value */
        if (H5Aread(attr, H5T_NATIVE_INT, &nlinks)<0) {
            db_perror("nlinks attribute", E_CALLFAIL, me);
            UNWIND();
        }
        nlinks++;
        if (H5Awrite(attr, H5T_NATIVE_INT, &nlinks)<0) {
            db_perror("nlinks attribute", E_CALLFAIL, me);
            UNWIND();
        }
        H5Aclose(attr);

        /* Create a name */
        sprintf(name, "#%06d", nlinks);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
        } H5E_END_TRY;
    } END_PROTECT;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_compwr
 *
 * Purpose:     Creates a new dataset in the link group.
 *
 * Return:      Success:        >=0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver 
 *
 *   Mark C. Miller, Thu Sep  8 12:54:39 PDT 2005
 *   Made it permit buf to be NULL in which case it will only create the
 *   dataset but not attempt to write to it. Also, made it return hid_t of
 *   created dataset.
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_hdf5_compwr(DBfile_hdf5 *dbfile, int dtype, int rank, int _size[],
               void *buf, char *name/*in,out*/)
{
    static char *me = "db_hdf5_compwr";
    hid_t       dset=-1, mtype=-1, ftype=-1, space=-1;
    int         i, nels;
    hsize_t     size[8];
    int         alloc = 0;

    if (rank < 0)
    {
        rank = -rank;
        alloc = 1;
    }

    /* Not an error if there is no data */
    for (i=0, nels=1; i<rank; i++) nels *= _size[i];
    if ((!buf || !nels) && !alloc) {
        *name = '\0';
        return 0;
    }
    PROTECT {
        /* Obtain a unique name for the dataset or use the name supplied */
        if (!*name) {
            strcpy(name, LINKGRP);
            if (db_hdf5_compname(dbfile, ENDOF(name)/*out*/)<0) {
                db_perror("compname", E_CALLFAIL, me);
                UNWIND();
            }
        }
        
        /* Obtain the memory and file types for the dataset */
        if ((mtype=silom2hdfm_type(dtype))<0 ||
            (ftype=silof2hdff_type(dbfile, dtype))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create the dataset and write data */
        assert(rank>0 && (size_t)rank<=NELMTS(size));
        for (i=0; i<rank; i++) size[i] = _size[i];
        if ((space=H5Screate_simple(rank, size, size))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        P_crprops = H5P_DEFAULT;
        if (SILO_Globals.enableChecksums)
        {
            H5Pset_chunk(P_ckcrprops, rank, size);
            P_crprops = P_ckcrprops;
        }

        if ((!strcmp(name,"time") && dtype == DB_FLOAT && rank == 1 && _size[0] == 1) ||
            (!strcmp(name,"dtime") && dtype == DB_DOUBLE && rank == 1 && _size[0] == 1) ||
            (!strcmp(name,"cycle") && dtype == DB_INT && rank == 1 && _size[0] == 1)) {
            if ((dset=H5Dcreate(dbfile->cwg, name, ftype, space, P_crprops))<0 &&
                (dset=H5Dopen(dbfile->cwg, name))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
        } else {
            if ((dset=H5Dcreate(dbfile->link, name, ftype, space, P_crprops))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
        }
        if (buf && H5Dwrite(dset, mtype, space, space, H5P_DEFAULT, buf)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Release resources */
        H5Dclose(dset);
        H5Sclose(space);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Dclose(dset);
            H5Sclose(space);
        } H5E_END_TRY;
    } END_PROTECT;
    return dset; 
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_comprd
 *
 * Purpose:     Reads a dataset from the file into memory.
 *
 * Return:      Success:        Pointer to dataset values.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 31, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-10-13
 *              Uses the current working directory instead of the root
 *              directory.
 *-------------------------------------------------------------------------
 */
PRIVATE void *
db_hdf5_comprd(DBfile_hdf5 *dbfile, char *name, int ignore_force_single)
{
    static char *me = "db_hdf5_comprd";
    void        *buf = NULL;
    hid_t       d=-1, fspace=-1, ftype=-1, mtype=-1;
    int         i, nelmts;
    void       *retval = NULL;
    
    PROTECT {
        if (name && *name) {
            if ((d=H5Dopen(dbfile->cwg, name))<0) {
                db_perror(name, E_NOTFOUND, me);
                UNWIND();
            }
            if ((fspace=H5Dget_space(d))<0 || (ftype=H5Dget_type(d))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            nelmts = H5Sget_simple_extent_npoints(fspace);

            /* Choose a memory type based on the file type */
            mtype = hdf2hdf_type(ftype);

            /* if we decided on a memory type of double but
             * we are forcing single precision, then select
             * a memory type of float */
            if (mtype == H5T_NATIVE_DOUBLE &&
                force_single_g && !ignore_force_single)
                mtype = H5T_NATIVE_FLOAT;

            /* Read the data */
            if (NULL==(buf=malloc(nelmts*H5Tget_size(mtype)))) {
                db_perror(name, E_NOMEM, me);
                UNWIND();
            }

            P_rdprops = H5P_DEFAULT;
            if (!SILO_Globals.enableChecksums)
                P_rdprops = P_ckrdprops;

            if (H5Dread(d, mtype, H5S_ALL, H5S_ALL, P_rdprops, buf)<0) {
                hdf5_to_silo_error(name, me);
                UNWIND();
            }

            /* Free resources */
            H5Dclose(d);
            H5Tclose(ftype);
            H5Sclose(fspace);

            /* Setup return value */
            retval = buf;
            
            /* Convert to float if necessary */
            /* With newer versions of HDF5, this could have been done
             * automatically in the H5Dread call, above by selecting
             * the appropriate memory type. However, the current version
             * of HDF5 does not support conversion from integral types
             * to float (or double) */
            if (force_single_g && !ignore_force_single &&
                mtype != H5T_NATIVE_FLOAT)
            {
                float *newbuf;

                /* allocate a new buffer */
                if (NULL==(newbuf=malloc(nelmts*sizeof(float)))) {
                    db_perror(name, E_NOMEM, me);
                    UNWIND();
                }

                /* do the conversion */
                if      (mtype == H5T_NATIVE_UCHAR)
                {
                    char *cbuf = (char *) buf;
                    for (i = 0; i < nelmts; i++)
                        newbuf[i] = (float)(cbuf[i]);
                }
                else if (mtype == H5T_NATIVE_SHORT)
                {
                    short *sbuf = (short *) buf;
                    for (i = 0; i < nelmts; i++)
                        newbuf[i] = (float)(sbuf[i]);
                }
                else if (mtype == H5T_NATIVE_INT)
                {
                    int *ibuf = (int *) buf;
                    for (i = 0; i < nelmts; i++)
                        newbuf[i] = (float)(ibuf[i]);
                }
                else if (mtype == H5T_NATIVE_LONG)
                {
                    long *lbuf = (long *) buf;
                    for (i = 0; i < nelmts; i++)
                        newbuf[i] = (float)(lbuf[i]);
                }

                /* Free old buffer and setup return value */
                free(buf);
                retval = newbuf;
            }
        }
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Dclose(d);
            H5Tclose(ftype);
            H5Sclose(fspace);
        } H5E_END_TRY;
        FREE(buf);
    } END_PROTECT;

    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_fullname
 *
 * Purpose:     Given the current working directory name, the parent object
 *              name as passed by the Silo library client and the child
 *              object name as stored in the parent object, determine the
 *              absolute, full path name of the child object.
 *
 * Return:      Success:     fully resolved full path name of child object
 *
 *              Failure:     0 
 *-------------------------------------------------------------------------
 */
PRIVATE char * 
db_hdf5_resolvename(DBfile *_dbfile,
                    const char *parent_objname,
                    const char *child_objname)
{
    static char cwgname[4096];
    static char result[4096];
    char *parent_objdirname;
    char *parent_fullname;
    char *child_fullname;

    db_hdf5_GetDir(_dbfile, cwgname);
    parent_objdirname = db_dirname(parent_objname);
    parent_fullname = db_join_path(cwgname, parent_objdirname);
    child_fullname = db_join_path(parent_fullname, child_objname);
    strcpy(result, child_fullname);
    free(parent_objdirname);
    free(parent_fullname);
    free(child_fullname);
    return result;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_fullname
 *
 * Purpose:     Given a name return the corresponding absolute name. If NAME
 *              is the empty string then the FULL output value will also be
 *              the empty string.
 *
 * Return:      Success:        0, full name returned through FULL argument.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_hdf5_fullname(DBfile_hdf5 *dbfile, char *name, char *full/*out*/)
{
    if (!name || !*name) {
        *full = '\0';
    } else if ('/'==*name) {
        strcpy(full, name);
    } else {
        db_hdf5_GetDir((DBfile*)dbfile, full);
        if (strcmp(full, "/")) strcat(full, "/");
        strcat(full, name);
    }
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_hdrwr
 *
 * Purpose:     Writes BUF as the `silo' attribute of an object.  If the
 *              object does not exist then it is created as a named data type
 *              (because a named data type has less overhead than an empty
 *              dataset in hdf5).
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
PRIVATE int
db_hdf5_hdrwr(DBfile_hdf5 *dbfile, char *name, hid_t mtype, hid_t ftype,
              void *buf, DBObjectType objtype)
{
    static char *me = "db_hdf5_hdrwr";
    hid_t       obj=-1, attr=-1;
    int         _objtype = (int)objtype;
    int         created = FALSE;
    
    PROTECT {
        /* Open an existing object or create a named type */
        H5E_BEGIN_TRY {
            obj = H5Topen(dbfile->cwg, name);
        } H5E_END_TRY;
        if (obj<0) {
            obj = H5Tcopy(H5T_NATIVE_INT);
            if (H5Tcommit(dbfile->cwg, name, obj)<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            created = TRUE;
        }

        /* Open or create the `silo' attribute */
        if (created) {
            attr = -1;
        } else {
            H5E_BEGIN_TRY {
                attr = H5Aopen_name(obj, "silo");
            } H5E_END_TRY;
        }
        if (attr<0 && (attr=H5Acreate(obj, "silo", ftype, SCALAR,
                                      H5P_DEFAULT))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Write data to the attribute */
        if (H5Awrite(attr, mtype, buf)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        H5Aclose(attr);

        /* Open or create the `silo_type' attribute */
        if (created) {
            attr = -1;
        } else {
            H5E_BEGIN_TRY {
                attr = H5Aopen_name(obj, "silo_type");
            } H5E_END_TRY;
        }
        if (attr<0 && (attr=H5Acreate(obj, "silo_type", H5T_NATIVE_INT, SCALAR,
                                      H5P_DEFAULT))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (H5Awrite(attr, H5T_NATIVE_INT, &_objtype)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        H5Aclose(attr);
        H5Tclose(obj);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(obj);
        } H5E_END_TRY;
    } END_PROTECT;
    return 0;
}



/*-------------------------------------------------------------------------
 * Function:    db_hdf5_ForceSingle
 *
 * Purpose:     If STATUS is non-zero then all floating point raw data values
 *              transferred between the application and the silo API will be
 *              of type `float'; otherwise type `double' is assumed.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  Robb Matzke
 *              Monday, March 22, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_ForceSingle(int status)
{
    force_single_g = status;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_file_accprops 
 *
 * Purpose:     Create file access property lists 
 *
 * Programmer:  Mark C. Miller, Aug 1, 2006 
 *-------------------------------------------------------------------------
 */
INTERNAL hid_t 
db_hdf5_file_accprops(int subtype)
{
    hid_t retval = -1;
    int vfd = subtype & 0x00000007;
    int inc = ((subtype & 0x7FFFFFF8) >> 3) * 1024;

    /* default properties */
    retval = H5Pcreate(H5P_FILE_ACCESS);

    /* this property makes it so closing the file automatically closes
       all open objects in the file. This is just in case the HDF5
       driver is failing to close some object */
    H5Pset_fclose_degree(retval, H5F_CLOSE_STRONG);

    switch (vfd)
    {
        case 0: break; /* this is the default case */
        case 1: H5Pset_fapl_sec2(retval); break;
        case 2: H5Pset_fapl_stdio(retval); break;
        case 3: H5Pset_fapl_core(retval, inc, TRUE); break;
#ifdef PARALLEL
        case 4:
        {
            MPI_Info info;
            MPI_Info_create(&info);
            H5Pset_fapl_mpio(retval, MPI_COMM_SELF, info);
            MPI_Info_free(&info);
            break;
        }
        case 5: H5Pset_fapl_mpiposix(retval, MPI_COMM_SELF); break;
#endif
    }

    return retval;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_finish_open
 *
 * Purpose:     Completes opening of a Silo/HDF5 file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, March 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_finish_open(DBfile *_dbfile)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_finish_open";
    hid_t       cwg=-1, link=-1, attr=-1;
    int         tmp, target=DB_LOCAL;
    
    PROTECT {
        /* Open "/" as current working group */
        if ((cwg=H5Gopen(dbfile->fid, "/"))<0) {
            db_perror("root group", E_CALLFAIL, me);
            UNWIND();
        }

        /*
         * Open the link directory. If it doesn't exist then create one (it
         * might not exist in old SAMI files).
         */
        H5E_BEGIN_TRY {
            link = H5Gopen(dbfile->fid, LINKGRP);
        } H5E_END_TRY;
        if (link<0 && (link=H5Gcreate(dbfile->fid, LINKGRP, 0))<0) {
            db_perror("link group", E_CALLFAIL, me);
            UNWIND();
        }

        /*
         * Read the targetting information from the `target' attribute
         * of the link group if there is one, otherwise assume DB_LOCAL
         */
        H5E_BEGIN_TRY {
            attr = H5Aopen_name(link, "target");
        } H5E_END_TRY;
        if (attr>=0 &&
            H5Aread(attr, H5T_NATIVE_INT, &tmp)>=0 &&
            H5Aclose(attr)>=0) {
            target = tmp;
        }

        /*
         * Initialize the file struct. Use the same target architecture that
         * was specified when the file was created.
         */
        dbfile->cwg = cwg;
        dbfile->link = link;
        db_hdf5_InitCallbacks((DBfile*)dbfile, target);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Gclose(cwg);
            H5Gclose(link);
        } H5E_END_TRY;
        dbfile->cwg = -1;
        dbfile->link = -1;
    } END_PROTECT;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_finish_create
 *
 * Purpose:     Finish creating a file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, March 26, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-08-17
 *              The file information string is written as a variable in the
 *              file instead of as a comment on the root group.
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_finish_create(DBfile *_dbfile, int target, char *finfo)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_finish_create";
    hid_t       attr=-1;
    int         size;
    char        hdf5VString[32];
    
    PROTECT {
        /* Open root group as CWG */
        if ((dbfile->cwg=H5Gopen(dbfile->fid, "/"))<0) {
            db_perror("root group", E_CALLFAIL, me);
            UNWIND();
        }

        /* Create the link group */
        if ((dbfile->link=H5Gcreate(dbfile->fid, LINKGRP, 0))<0) {
            db_perror("link group", E_CALLFAIL, me);
            UNWIND();
        }

        /* Callbacks */
        db_hdf5_InitCallbacks((DBfile*)dbfile, target);
        
        /*
         * Write the target architecture into the `target' attribute of the
         * link group so we can retrieve it later when the file is reopened.
         */
        if ((attr=H5Acreate(dbfile->link, "target", dbfile->T_int, SCALAR,
                            H5P_DEFAULT))<0 ||
            H5Awrite(attr, H5T_NATIVE_INT, &target)<0 ||
            H5Aclose(attr)<0) {
            db_perror("targetinfo", E_CALLFAIL, me);
            UNWIND();
        }

        if (finfo) {
            /* Write file info as a variable in the file */
            size = strlen(finfo)+1;
            if (db_hdf5_Write(_dbfile, "_fileinfo", finfo, &size, 1,
                              DB_CHAR)<0) {
                db_perror("fileinfo", E_CALLFAIL, me);
                UNWIND();
            }
        }

        /*
         * Write HDF5 library version information to the file 
         */
        sprintf(hdf5VString, "hdf5-%d.%d.%d%s%s", H5_VERS_MAJOR,
            H5_VERS_MINOR, H5_VERS_RELEASE,
            strlen(H5_VERS_SUBRELEASE) ? "-" : "", H5_VERS_SUBRELEASE);
        size = strlen(hdf5VString)+1;
        if (db_hdf5_Write(_dbfile, "_hdf5libinfo", hdf5VString, &size, 1,
                              DB_CHAR)<0) {
                db_perror("_hdf5libinfo", E_CALLFAIL, me);
                UNWIND();
        }

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Gclose(dbfile->cwg);
            H5Gclose(dbfile->link);
        } H5E_END_TRY;
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_initiate_close
 *
 * Purpose:     Start closing the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, March 26, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
INTERNAL int
db_hdf5_initiate_close(DBfile *_dbfile)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_initiate_close";
    int         i;

    /* Close all datasets in the circular buffer */
    for (i=0; i<NDSETTAB; i++) {
        FREE(dbfile->dsettab[i]);
        dbfile->dsettab[i] = NULL;
    }
    dbfile->dsettab_ins = dbfile->dsettab_rem = 0;
    
    /* Close current working group and link group */
    if (H5Gclose(dbfile->cwg)<0 || H5Gclose(dbfile->link)<0) {
        return db_perror("closing", E_CALLFAIL, me);
    }
    dbfile->cwg = -1;
    dbfile->link = -1;

    if (dbfile->cwg_name)
        free(dbfile->cwg_name);
    dbfile->cwg_name = NULL;
    
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_Open
 *
 * Purpose:     Opens an hdf5 file that already exists.
 *
 * Return:      Success:        Ptr to the file struct
 *
 *              Failure:        NULL, db_errno set.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb  1 18:13:28 PST 2005
 *   Added call to H5Eset_auto(). Open can be called outside of init
 *
 *-------------------------------------------------------------------------
 */
INTERNAL DBfile *
db_hdf5_Open(char *name, int mode, int subtype)
{
    DBfile_hdf5 *dbfile=NULL;
    hid_t       fid=-1, faprops=-1;
    unsigned    hmode;
    static char *me = "db_hdf5_Open";

    PROTECT {
        /* Turn off error messages from the hdf5 library */
        H5Eset_auto(NULL, NULL);

        /* File access mode */
        if (DB_READ==mode) {
            hmode = H5F_ACC_RDONLY;
        } else if (DB_APPEND==mode) {
            hmode = H5F_ACC_RDWR;
        } else {
            db_perror("mode", E_INTERNAL, me);
            UNWIND();
        }

        faprops = db_hdf5_file_accprops(subtype); 

        /* Open existing hdf5 file */
        if ((fid=H5Fopen(name, hmode, faprops))<0) {
            H5Pclose(faprops);
            db_perror(name, E_NOFILE, me);
            UNWIND();
        }
        H5Pclose(faprops);

        /* Create silo file struct */
        if (NULL==(dbfile=calloc(1, sizeof(DBfile_hdf5)))) {
            db_perror(name, E_NOMEM, me);
            UNWIND();
        }
        dbfile->pub.name = STRDUP(name);
        dbfile->pub.type = DB_HDF5;
        dbfile->fid = fid;
        db_hdf5_finish_open((DBfile*)dbfile);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Fclose(fid);
        } H5E_END_TRY;
        if (dbfile) {
            if (dbfile->pub.name) free(dbfile->pub.name);
            free(dbfile);
        }
    } END_PROTECT;
    
    return (DBfile*)dbfile;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_Create
 *
 * Purpose:     Creates an hdf5 file and begins the process of writing mesh
 *              and mesh-related data into that file.
 *
 *              The `target' is always assumed to be the local architecture.
 *
 * Return:      Success:        Pointer to a new file
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb  1 18:13:28 PST 2005
 *   Added call to H5Eset_auto(). Create can be called outside of init
 *
 *-------------------------------------------------------------------------
 */
INTERNAL DBfile *
db_hdf5_Create(char *name, int mode, int target, int subtype, char *finfo)
{
    DBfile_hdf5 *dbfile=NULL;
    hid_t       fid=-1, faprops=-1;
    static char *me = "db_hdf5_Create";

    PROTECT {
        /* Turn off error messages from the hdf5 library */
        H5Eset_auto(NULL, NULL);

        faprops = db_hdf5_file_accprops(subtype);

        /* Create or open hdf5 file */
        if (DB_CLOBBER==mode) {
            /* If we ever use checksumming (which requires chunked datasets),
             * HDF5's BTree's will effect storage overhead. Since Silo really
             * doesn't support growing/shrinking datasets, we just use a value
             * of '1' for istore_k */
            hid_t fcprops = H5Pcreate(H5P_FILE_CREATE);
            H5Pset_istore_k(fcprops, 1);
            fid = H5Fcreate(name, H5F_ACC_TRUNC, fcprops, faprops);
            H5Pclose(fcprops);
            H5Glink(fid, H5G_LINK_HARD, "/", ".."); /*don't care if fails*/
        } else if (DB_NOCLOBBER==mode) {
            fid = H5Fopen(name, H5F_ACC_RDWR, faprops);
        } else {
            H5Pclose(faprops);
            db_perror("mode", E_BADARGS, me);
            UNWIND();
        }
        if (fid<0) {
            H5Pclose(faprops);
            db_perror(name, E_NOFILE, me);
            UNWIND();
        }

        H5Pclose(faprops);

        /* Create silo file struct */
        if (NULL==(dbfile=calloc(1, sizeof(DBfile_hdf5)))) {
            db_perror(name, E_NOMEM, me);
            UNWIND();
        }
        dbfile->pub.name = STRDUP(name);
        dbfile->pub.type = DB_HDF5;
        dbfile->fid = fid;
        db_hdf5_finish_create((DBfile*)dbfile, target, finfo);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Fclose(fid);
        } H5E_END_TRY;
        if (dbfile) {
            if (dbfile->pub.name) free(dbfile->pub.name);
            free(dbfile);
        }
    } END_PROTECT;

    return (DBfile*)dbfile;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_Close
 *
 * Purpose:     Closes an hdf5 file and frees memory associated with the
 *              file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_Close(DBfile *_dbfile)
{
   DBfile_hdf5    *dbfile = (DBfile_hdf5*)_dbfile;

   if (dbfile) {
       /* Free the private parts of the file */
       if (db_hdf5_initiate_close((DBfile*)dbfile)<0) return -1;
       if (H5Fclose(dbfile->fid)<0) return -1;
       dbfile->fid = -1;

       /* Free the public parts of the file */
       db_close(_dbfile);
   }
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_Filters
 *
 * Purpose:     Output the name of this device driver to the specified stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        never fails
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 17, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_Filters(DBfile *_dbfile, FILE *stream)
{
    fprintf(stream, "HDF5 Device Driver\n");
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_MkDir
 *
 * Purpose:     Create a new directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, February 10, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-10-13
 *              Uses the current working group instead of the root group when
 *              adding the `..' entry.
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_MkDir(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_MkDir";
    char        *dotdot = NULL,  *parent=NULL, *t=NULL;
    hid_t       grp = -1;

    PROTECT {
        /* Create the new group */
        if ((grp=H5Gcreate(dbfile->cwg, name, 0))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* What is the name of the parent directory of the new directory? */
        parent = STRDUP(name);
        t = parent+strlen(parent);
        while (t>parent && '/'==t[-1]) *(--t) = '\0';   /*trailing slashes*/
        while (t>parent && '/'!=t[-1]) *(--t) = '\0';   /*last component*/
        if (!*parent) strcpy(parent, '/'==*name?"/":".");

        /* What is the name of the `..' entry? */
        dotdot = malloc(strlen(name)+4);
        strcpy(dotdot, name);
        strcat(dotdot, "/..");

        /* Make the `..' point to the parent */
        if (H5Glink(dbfile->cwg, H5G_LINK_HARD, parent, dotdot)<0) {
            db_perror(dotdot, E_CALLFAIL, me);
            UNWIND();
        }

        /* Close everything */
        H5Gclose(grp);
        free(dotdot);
        free(parent);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Gclose(grp);
        } H5E_END_TRY;
        if (dotdot) free(dotdot);
        if (parent) free(parent);
    } END_PROTECT;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_SetDir
 *
 * Purpose:     Set the current working directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, February 10, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_SetDir(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_SetDir";
    hid_t       newdir = -1;
    
    PROTECT {
        if ((newdir=H5Gopen(dbfile->cwg, name))<0 ||
            H5Gget_objinfo(newdir, "..", FALSE, NULL)<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }

        H5Gclose(dbfile->cwg);
        dbfile->cwg = newdir;
        if (dbfile->cwg_name) {
            free(dbfile->cwg_name);
            dbfile->cwg_name = NULL;
        }
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Gclose(newdir);
        } H5E_END_TRY;
    } END_PROTECT;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetDir
 *
 * Purpose:     Writes the absolute name of the current working directory
 *              into the NAME argument without checking for overflow (if NAME
 *              is not large enough then a core dump is likely).
 *
 *              The name is computed by following the `..' entries out of
 *              each directory and then looking for the entry in the parent
 *              that matches the object id of the current directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetDir(DBfile *_dbfile, char *name/*out*/)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_SetDir";
    hid_t       cwg = -1, parent = -1;
    H5G_stat_t  cur_sb, par_sb;
    int         i, ncomps=0;
    silo_hdf5_comp_t    comp[100];

    /* Return quickly if name is cached */
    if (dbfile->cwg_name) {
        strcpy(name, dbfile->cwg_name);
        return 0;
    }
    
    PROTECT {
        memset(comp, 0, sizeof comp);
        cwg = H5Gopen(dbfile->cwg, ".");
        if (H5Gget_objinfo(cwg, ".", TRUE, &cur_sb)<0) {
            db_perror("stat(\".\")", E_CALLFAIL, me);
            UNWIND();
        }

        while ((size_t)ncomps<NELMTS(comp)) {
            /*
             * Get info about parent. If parent object ID is the same as
             * current group object ID then we must be at the root.
             */
            if (H5Gget_objinfo(cwg, "..", TRUE, &par_sb)<0) {
                db_perror("stat(\"..\")", E_CALLFAIL, me);
                UNWIND();
            }
            if (cur_sb.objno[0]==par_sb.objno[0] &&
                cur_sb.objno[1]==par_sb.objno[1]) break;

            /*
             * Iterate over entries in parent to find first name that has the
             * same object ID as the current group and use that as the
             * component of the name
             */
            if ((parent=H5Gopen(cwg, ".."))<0) {
                db_perror("no `..' entry", E_NOTFOUND, me);
                UNWIND();
            }
            comp[ncomps].objno[0] = cur_sb.objno[0];
            comp[ncomps].objno[1] = cur_sb.objno[1];
            if (H5Giterate(parent, ".", NULL, find_objno, comp+ncomps)<=0) {
                db_perror("inconsistent directory structure", E_CALLFAIL, me);
                UNWIND();
            }

            /* Move upward in the directory try */
            H5Gclose(cwg);
            cwg = parent;
            parent = -1;
            cur_sb = par_sb;
            ncomps++;
        }

        /* Build the name */
        if (0==ncomps) {
            strcpy(name, "/");
        } else {
            name[0] = '\0';
            for (i=ncomps-1; i>=0; --i) {
                strcat(name, "/");
                strcat(name, comp[i].name);
                free(comp[i].name);
            }
        }

        /* Close everything */
        H5Gclose(cwg);
        if (parent>=0) H5Gclose(parent);

        /* Cache the name for later */
        if (dbfile->cwg_name)
            free(dbfile->cwg_name);
        dbfile->cwg_name = strdup(name);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Gclose(cwg);
            H5Gclose(parent);
        } H5E_END_TRY;
        for (i=0; i<=ncomps && (size_t)i<NELMTS(comp); i++) {
            if (comp->name) free(comp->name);
        }
    } END_PROTECT;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_NewToc
 *
 * Purpose:     Destroy the previous table of contents and replace it with a
 *              new table of contents for the current working directory.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_NewToc(DBfile *_dbfile)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    DBtoc       *toc=NULL;
    
    db_FreeToc(_dbfile);
    dbfile->pub.toc = toc = db_AllocToc();

    if (H5Giterate(dbfile->cwg, ".", NULL, load_toc, toc)<0) return -1;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetComponentType
 *
 * Purpose:     Return the data type of the component 
 *
 * Programmer:  Mark C. Miller
 *              Wednesday, April 20, 2005
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetComponentType(DBfile *_dbfile, char *objname, char *compname)
{
    int datatype = DB_NOTYPE;
    db_hdf5_GetComponentStuff(_dbfile, objname, compname, &datatype);
    return datatype;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetComponent
 *
 * Purpose:     Reads the component COMPNAME of object OBJNAME, allocates
 *              memory and copies the value into it to return.
 *
 * Warning:     The names of components in an object in an HDF5 file may not
 *              be the same as the names of components in a PDB file.
 *
 * Programmer:  Mark C. Miller
 *              Wednesday, April 20, 2005
 *-------------------------------------------------------------------------
 */
CALLBACK void *
db_hdf5_GetComponent(DBfile *_dbfile, char *objname, char *compname)
{
    return db_hdf5_GetComponentStuff(_dbfile, objname, compname, 0);
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetComponentStuff
 *
 * Purpose:     Reads the component COMPNAME of object OBJNAME, allocates
 *              memory and copies the value into it to return.
 *
 * Warning:     The names of components in an object in an HDF5 file may not
 *              be the same as the names of components in a PDB file.
 *
 * Return:      Success:        Pointer to allocated memory which contains
 *                              the component value. Floating-point
 *                              components are returned in either single or
 *                              double precision according to the
 *                              DBForceSingle() setting.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August 17, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   This routine had assumed that any string-valued data member was
 *   specifying the name of a dataset to read. That is not always true.
 *   I modified it to attempt to read a dataset of the given name or
 *   just return the string.
 *
 *   Mark C. Miller, Wed Apr 20 15:09:41 PDT 2005
 *   Renamed from db_hdf5_GetComponent. Added just_get_dataype argument
 *   and logic to just return data type when requested and avoid 
 *   actual read of data
 *
 *-------------------------------------------------------------------------
 */
CALLBACK void *
db_hdf5_GetComponentStuff(DBfile *_dbfile, char *objname, char *compname,
    int *just_get_datatype)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetComponent";
    hid_t       o=-1, attr=-1, atype=-1, ftype=-1, mtype=-1, dset=-1;
    int         datatype, mno, n, ndims, i, dim[3], mult;
    void        *retval=NULL;
    
    PROTECT {
        /* Open the object as a named data type */
        if ((o=H5Topen(dbfile->cwg, objname))<0) {
            db_perror(objname, E_NOTFOUND, me);
            UNWIND();
        }

        /*
         * Open the `silo' attribute (all silo objects have one), and
         * retrieve its data type (a single-level H5T_COMPOUND type).
         */
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            (atype=H5Aget_type(attr))<0) {
            db_perror(objname, E_CALLFAIL, me);
            UNWIND();
        }

        /* Scan through the compound type to find the requested component. */
        n = H5Tget_nmembers(atype);
        for (mno=0; mno<n; mno++) {
            char *memb_name = H5Tget_member_name(atype, mno);
            if (memb_name && !strcmp(memb_name, compname)) {
                free(memb_name);
                break;
            }
            if (memb_name) free(memb_name);
        }
        if (mno>=n) {
            db_perror(compname, E_NOTFOUND, me);
            UNWIND();
        }

        /* Get the datatype and multiplicity for the component */
        if ((ftype=db_hdf5_get_cmemb(atype, mno, &ndims, dim))<0) {
            db_perror(compname, E_CALLFAIL, me);
            UNWIND();
        }

        if (H5T_STRING==H5Tget_class(ftype)) {

            if (just_get_datatype == 0)
            {
                /*
                 * A string is usually a pointer to some other dataset. So, try
                 * read that dataset instead.
                 */
                char dataset_name[] =
                    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

                /* Read the dataset name from the attribute */
                if ((H5Tget_size(ftype)+1<sizeof dataset_name) &&
                    (mtype=H5Tcreate(H5T_COMPOUND, H5Tget_size(ftype)))>=0 &&
                    db_hdf5_put_cmemb(mtype, compname, 0, 0, NULL, ftype)>=0 && 
                    H5Aread(attr, mtype, dataset_name)>=0) {
                    if ((dset=H5Dopen(dbfile->cwg, dataset_name))>=0) {
                        retval = db_hdf5_comprd(dbfile, dataset_name, 1);
                        H5Dclose(dset);
                    }
                    else {
                        retval = STRDUP(dataset_name);
                    }
                }
            }
            else
            {
                *just_get_datatype = DB_VARIABLE;
            }
        }
        else
        {
            /* Read a single component of the compound attribute */
            if ((datatype=hdf2silo_type(ftype))<0) {
                db_perror(compname, E_CALLFAIL, me);
                UNWIND();
            }

            if (just_get_datatype == 0)
            {
                for (i=0, mult=1; i<ndims; i++) mult *= dim[i];

                /* Allocate the return value */
                if (NULL==(retval=calloc(mult, db_GetMachDataSize(datatype)))) {
                    db_perror(compname, E_CALLFAIL, me);
                    UNWIND();
                }

                /* Build the hdf5 data type to read */
                mtype = H5Tcreate(H5T_COMPOUND, mult * db_GetMachDataSize(datatype));
                db_hdf5_put_cmemb(mtype, compname, 0, ndims, dim,
                                  hdf2hdf_type(ftype));

                /* Read the data into the output buffer */
                if (H5Aread(attr, mtype, retval)<0) {
                    db_perror(compname, E_CALLFAIL, me);
                    UNWIND();
                }
            }
            else
            {
                *just_get_datatype = datatype;
            }
        }
        
        /* Release objects */
        H5Tclose(o);
        H5Aclose(attr);
        H5Tclose(atype);
        H5Tclose(ftype);
        H5Tclose(mtype);
    } CLEANUP {
        /* Release objects */
        if (retval) {
            free(retval);
            retval = NULL;
        }
        H5Tclose(o);
        H5Aclose(attr);
        H5Tclose(atype);
        H5Tclose(ftype);
        H5Tclose(mtype);
    } END_PROTECT;

    return retval;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetComponentNames
 *
 * Purpose:     Returns the component names for the specified object. The
 *              COMP_NAMES and FILE_NAMES output arguments will point to
 *              an array of pointers to names. Each name as well as the
 *              two arrays will be allocated with `malloc'.  This is
 *              essentially the same as DBGetObject().
 *
 * Return:      Success:        Number of components found for the
 *                              specified object.
 *
 *              Failure:        zero
 *
 * Programmer:  Robb Matzke, 2001-02-06
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetComponentNames(DBfile *_dbfile, char *objname, char ***comp_names,
                          char ***file_names)
{
    DBobject    *obj;
    int         n;

    if (NULL==(obj=db_hdf5_GetObject(_dbfile, objname))) return 0;
    if (comp_names) {
        *comp_names = obj->comp_names;
    } else {
        free(obj->comp_names);
    }
    if (file_names) {
        *file_names = obj->pdb_names;
    } else {
        free(obj->pdb_names);
    }
    n = obj->ncomponents;
    obj->ncomponents = 0;
    DBFreeObject(obj);
    return n;
}
    

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_WriteObject
 *
 * Purpose:     Write/overwrite a DBobject into the given file.

 *
 * Return:      Success:        Non-negative
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke, 2001-02-05
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   I made it support writing a DBZonelist object. I also fixed an
 *   off-by-one error in indexing of string valued component names
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_WriteObject(DBfile *_dbfile,    /*File to write into */
                    DBobject *obj,      /*Object description to write out */
                    int flags)          /*1=>free associated memory */
{
    DBfile_hdf5         *dbfile=(DBfile_hdf5*)_dbfile;
    static char         *me="db_hdf5_WriteObject";
    size_t              msize=0, fsize=0, moffset=0, foffset=0;
    unsigned char       *object=NULL;
    hid_t               mtype=-1, ftype=-1;
    int                 i;

    PROTECT {
        /* If flags is set then delete any existing object by the same
         * name, ignoring failures. */
        if (flags) {
            H5E_BEGIN_TRY {
                H5Gunlink(dbfile->cwg, obj->name);
            } H5E_END_TRY;
        }
        
        /* With the exception of a zonelist object, the HDF5 driver
         * only handles user-defined objects. Otherwise,
         * if the user were allowed to use DBMakeObject() to bybass the
         * normal object creation functions, the user would need to know
         * implementation details of the driver. Some of the silo
         * confidence tests think they know implementation details when in
         * fact they don't -- those tests will fail. */
        if (!strcmp(obj->type, "zonelist")) {

            /* make sure we recognize every component name in this object */
            int recognizeComponent = 1;
            for (i=0; (i<obj->ncomponents) && recognizeComponent; i++)
            {
                recognizeComponent = 0;
                if ((strcmp(obj->comp_names[i], "ndims") == 0) ||
                    (strcmp(obj->comp_names[i], "nzones") == 0) ||
                    (strcmp(obj->comp_names[i], "nshapes") == 0) ||
                    (strcmp(obj->comp_names[i], "lnodelist") == 0) ||
                    (strcmp(obj->comp_names[i], "origin") == 0) ||
                    (strcmp(obj->comp_names[i], "lo_offset") == 0) ||
                    (strcmp(obj->comp_names[i], "hi_offset") == 0) ||
                    (strcmp(obj->comp_names[i], "nodelist") == 0) ||
                    (strcmp(obj->comp_names[i], "shapecnt") == 0) ||
                    (strcmp(obj->comp_names[i], "shapesize") == 0) ||
                    (strcmp(obj->comp_names[i], "shapetype") == 0) ||
                    (strcmp(obj->comp_names[i], "gzoneno") == 0))
                {
                    recognizeComponent = 1;
                }
            }

            if (!recognizeComponent)
            {
                char msg[256];
                sprintf(msg, "Unrecognized component, \"%s\", in zonelist object",
                    obj->comp_names[i]);
                db_perror(msg, E_BADARGS, me);
                UNWIND();
            }

        } else if (strcmp(obj->type, "unknown")) {
            db_perror("DBobject is not type DB_USERDEF", E_BADARGS, me);
            UNWIND();
        }

        /* How much memory do we need? Align all components */
        for (i=0, msize=fsize=0; i<obj->ncomponents; i++) {
            if (!strncmp(obj->pdb_names[i], "'<i>", 4)) {
                msize = ALIGN(msize, sizeof(int)) + sizeof(int);
                fsize += H5Tget_size(dbfile->T_int);
            } else if (!strncmp(obj->pdb_names[i], "'<f>", 4)) {
                msize = ALIGN(msize, sizeof(float)) + sizeof(float);
                fsize += H5Tget_size(dbfile->T_float);
            } else if (!strncmp(obj->pdb_names[i], "'<d>", 4)) {
                msize = ALIGN(msize, sizeof(double)) + sizeof(double);
                fsize += H5Tget_size(dbfile->T_double);
            } else if (!strncmp(obj->pdb_names[i], "'<s>", 4)) {
                msize += strlen(obj->pdb_names[i]+4);
                fsize += strlen(obj->pdb_names[i]+4);
            } else if (obj->pdb_names[i][0]=='\'') {
                /* variable has invalid name or we don't handle type */
                db_perror(obj->pdb_names[i], E_INVALIDNAME, me);
                UNWIND();
            } else {
                /* variable added by DBAddVarComponent() */
                msize += strlen(obj->pdb_names[i]) + 1;
                fsize += strlen(obj->pdb_names[i]) + 1;
            }
        }

        /* Create the object and initialize it */
        if (NULL==(object=calloc(1, msize))) {
            db_perror(NULL, E_NOMEM, me);
            UNWIND();
        }
        if ((mtype=H5Tcreate(H5T_COMPOUND, msize))<0 ||
            (ftype=H5Tcreate(H5T_COMPOUND, fsize))<0) {
            db_perror("H5Tcreate", E_CALLFAIL, me);
            UNWIND();
        }
        for (i=0, moffset=foffset=0; i<obj->ncomponents; i++) {
            if (!strncmp(obj->pdb_names[i], "'<i>", 4)) {
                moffset = ALIGN(moffset, sizeof(int));
                if (H5Tinsert(mtype, obj->comp_names[i], moffset,
                              H5T_NATIVE_INT)<0 ||
                    H5Tinsert(ftype, obj->comp_names[i], foffset,
                              dbfile->T_int)<0) {
                    db_perror("H5Tinsert", E_CALLFAIL, me);
                    UNWIND();
                }
                *(int*)(object+moffset) = strtol(obj->pdb_names[i]+4, NULL, 0);
                moffset += sizeof(int);
                foffset += H5Tget_size(dbfile->T_int);
            } else if (!strncmp(obj->pdb_names[i], "'<f>", 4)) {
                moffset = ALIGN(moffset, sizeof(float));
                if (H5Tinsert(mtype, obj->comp_names[i], moffset,
                              H5T_NATIVE_FLOAT)<0 ||
                    H5Tinsert(ftype, obj->comp_names[i], foffset,
                              dbfile->T_float)<0) {
                    db_perror("H5Tinsert", E_CALLFAIL, me);
                    UNWIND();
                }
                *(float*)(object+moffset) = strtod(obj->pdb_names[i]+4, NULL);
                moffset += sizeof(float);
                foffset += H5Tget_size(dbfile->T_float);
            } else if (!strncmp(obj->pdb_names[i], "'<d>", 4)) {
                moffset = ALIGN(moffset, sizeof(double));
                if (H5Tinsert(mtype, obj->comp_names[i], moffset,
                              H5T_NATIVE_DOUBLE)<0 ||
                    H5Tinsert(ftype, obj->comp_names[i], foffset,
                              dbfile->T_double)<0) {
                    db_perror("H5Tinsert", E_CALLFAIL, me);
                    UNWIND();
                }
                *(double*)(object+moffset) = strtod(obj->pdb_names[i]+4, NULL);
                moffset += sizeof(double);
                foffset += H5Tget_size(dbfile->T_double);
            } else if (!strncmp(obj->pdb_names[i], "'<s>", 4)) {
                size_t len = strlen(obj->pdb_names[i]+4)-1;
                hid_t str_type = H5Tcopy(H5T_C_S1);
                H5Tset_size(str_type, len);
                if (H5Tinsert(mtype, obj->comp_names[i], moffset,
                              str_type)<0 ||
                    H5Tinsert(ftype, obj->comp_names[i], foffset,
                              str_type)<0) {
                    db_perror("H5Tinsert", E_CALLFAIL, me);
                    UNWIND();
                }
                H5Tclose(str_type);
                strncpy((char*)(object+moffset), obj->pdb_names[i]+4, len);
                object[moffset+len] = '\0'; /*overwrite quote*/
                moffset += len;
                foffset += len;
            } else {
                size_t len = strlen(obj->pdb_names[i])+1;
                hid_t str_type = H5Tcopy(H5T_C_S1);
                H5Tset_size(str_type, len);
                if (H5Tinsert(mtype, obj->comp_names[i], moffset,
                              str_type)<0 ||
                    H5Tinsert(ftype, obj->comp_names[i], foffset,
                              str_type)<0) {
                    db_perror("H5Tinsert", E_CALLFAIL, me);
                    UNWIND();
                }
                H5Tclose(str_type);
                strcpy((char*)(object+moffset), obj->pdb_names[i]);
                moffset += len;
                foffset += len;
            }
        }

        if (db_hdf5_hdrwr(dbfile, obj->name, mtype, ftype, object,
                          DBGetObjtypeTag(obj->type))<0) {
            UNWIND();
        }
        H5Tclose(mtype);
        H5Tclose(ftype);
    } CLEANUP {
        H5Tclose(mtype);
        H5Tclose(ftype);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_WriteComponent
 *
 * Purpose:     Add a variable component to the given object structure and
 *              write out the associated data.
 *
 * Return:      Success:        non-negative
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke, 2001-02-06
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_WriteComponent(DBfile *_dbfile, DBobject *obj, char *compname,
                       char *prefix, char *dataname, const void *data,
                       int rank, long _size[])
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    int         size[32], i;
    char        varname[256];
    int         datatype = db_GetDatatypeID(dataname);
    
    for (i=0; i<rank; i++) size[i] = _size[i];
    db_hdf5_compwr(dbfile, datatype, rank, size, (void*)data, varname);
    DBAddVarComponent(obj, compname, varname);
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_InqVarExists
 *
 * Purpose:     Check whether the variable VARNAME exists.
 *
 * Return:      Success:        positive if exists, zero otherwise.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   I made it return true or false based on existence of named entity
 *   only and not that the entity also be a dataset.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_InqVarExists (DBfile *_dbfile, char *varname)
{
   DBfile_hdf5  *dbfile = (DBfile_hdf5*)_dbfile;
   herr_t       status;
   H5G_stat_t   sb;

   /* Check existence */
   H5E_BEGIN_TRY {
       status = H5Gget_objinfo(dbfile->cwg, varname, TRUE, &sb);
   } H5E_END_TRY;
   if (status<0)
       return FALSE;
   else
       return TRUE;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetVarLength
 *
 * Purpose:     Returns the number of elements in the specified variable.
 *
 * Return:      Success:        Number of elements
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetVarLength(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetVarLength";
    hid_t       dset=-1, space=-1;
    hsize_t     nelmts=-1;

    PROTECT {
        if ((dset=H5Dopen(dbfile->cwg, name))>=0) {
            if ((space=H5Dget_space(dset))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            nelmts = H5Sget_simple_extent_npoints(space);
            H5Dclose(dset);
            H5Sclose(space);
        }
        else
        {
            if (!db_hdf5_get_comp_var(dbfile->cwg, name, &nelmts,
                 NULL, NULL, NULL)) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
        }
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Dclose(dset);
            H5Sclose(space);
        } H5E_END_TRY;
    } END_PROTECT;

    return nelmts;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetVarByteLength
 *
 * Purpose:     Returns the number of bytes needed to store the entire
 *              variable in memory.
 *
 * Return:      Success:        Number of bytes
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 17, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetVarByteLength(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetVarByteLength";
    hid_t       dset=-1, ftype=-1, mtype=-1, space=-1;
    hsize_t     nbytes_big;
    int         nbytes_small=-1;

    PROTECT {
        /* Open the dataset */
        if ((dset=H5Dopen(dbfile->cwg, name))>=0) {
        
            /* Get data type and space */
            if ((ftype=H5Dget_type(dset))<0 ||
                (mtype=hdf2hdf_type(ftype))<0 ||
                (space=H5Dget_space(dset))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }

            /* Get total size in bytes and check for overflow */
            nbytes_big = H5Sget_simple_extent_npoints(space) * H5Tget_size(mtype);
            nbytes_small = (int)nbytes_big;
            if (nbytes_big!=(hsize_t)nbytes_small) {
                db_perror("overflow", E_INTERNAL, me);
                UNWIND();
            }

            /* Release resources */
            H5Tclose(ftype);
            H5Sclose(space);
            H5Dclose(dset);
        }
        else
        {
            hsize_t nelmts;
            size_t elsize;
            if (!db_hdf5_get_comp_var(dbfile->cwg, name, &nelmts,
                 &elsize, NULL, NULL)) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            nbytes_small = (int)(nelmts * elsize);
        }
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Tclose(ftype);
            H5Sclose(space);
            H5Dclose(dset);
        } H5E_END_TRY;
    } END_PROTECT;

    return nbytes_small;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetVarType
 *
 * Purpose:     Query the data type of the variable.
 *
 * Return:      Success:        One of the DB_* constants that describes data
 *                              type. The type is chosen to be at least as
 *                              large as the type in the file, but using
 *                              DB_LONG in cases where there is no native
 *                              type which is large enough.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Sunday, February 14, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   I made it return -1 if name or *name is null.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetVarType(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetVarType";
    hid_t       dset=-1, ftype=-1;
    int         silo_type=-1;

    if ((name == 0) || (*name == 0))
        return -1;

    PROTECT {
        if ((dset=H5Dopen(dbfile->cwg, name))>=0) {
            if ((ftype=H5Dget_type(dset))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            silo_type = hdf2silo_type(ftype);
            H5Dclose(dset);
            H5Tclose(ftype);
        }
        else
        {
            if (!db_hdf5_get_comp_var(dbfile->cwg, name, NULL,
                 NULL, &ftype, NULL)) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
            silo_type = hdf2silo_type(ftype);
        }
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Dclose(dset);
            H5Tclose(ftype);
        } H5E_END_TRY;
    } END_PROTECT;

    return silo_type;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetVarDims
 *
 * Purpose:     Obtains the size of the dimensions of some variable.
 *
 * Return:      Success:        Number of dimensions. At most MAXDIMS values
 *                              are copied into the DIMS argument.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Monday, February 15, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_GetVarDims(DBfile *_dbfile, char *name, int maxdims, int *dims/*out*/)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetVarDims";
    hid_t       dset=-1, space=-1;
    hsize_t     ds_size[H5S_MAX_RANK];
    int         i, ndims=-1;
    
    PROTECT {
        if ((dset=H5Dopen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((space=H5Dget_space(dset))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if ((ndims=H5Sget_simple_extent_dims(space, ds_size, NULL))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        for (i=0; i<maxdims; i++) dims[i] = ds_size[i];
        H5Sclose(space);
        H5Dclose(dset);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Sclose(space);
            H5Dclose(dset);
        } H5E_END_TRY;
    } END_PROTECT;

    return ndims;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetVar
 *
 * Purpose:     Same as db_hdf5_ReadVar except it allocates space for the
 *              result.
 *
 * Return:      Success:        Ptr to data read, allocated with malloc()
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 11, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb 15 14:53:29 PST 2005
 *   Forced it to ignore force_single setting 
 *
 *-------------------------------------------------------------------------
 */
CALLBACK void *
db_hdf5_GetVar(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetVar";
    hid_t       dset=-1, ftype=-1, mtype=-1, space=-1;
    void        *result=NULL;

    PROTECT {

        /* Get dataset, type, and space */
        if ((dset=H5Dopen(dbfile->cwg, name))>=0) {
            if ((ftype=H5Dget_type(dset))<0 ||
                (space=H5Dget_space(dset))<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }

            /* Choose a memory type based on the file type */
            if ((mtype=hdf2hdf_type(ftype))<0) {
                db_perror("data type", E_BADARGS, me);
                UNWIND();
            }
        
            /* Allocate space for the result */
            if (NULL==(result=malloc(H5Sget_simple_extent_npoints(space)*
                                     H5Tget_size(mtype)))) {
                db_perror(NULL, E_NOMEM, me);
                UNWIND();
            }

            P_rdprops = H5P_DEFAULT;
            if (!SILO_Globals.enableChecksums)
                P_rdprops = P_ckrdprops;

            /* Read entire variable */
            if (H5Dread(dset, mtype, H5S_ALL, H5S_ALL, P_rdprops, result)<0) {
                hdf5_to_silo_error(name, me);
                UNWIND();
            }

            /* Close everything */
            H5Dclose(dset);
            H5Tclose(ftype);
            H5Sclose(space);
        }
        else
        {
            if (!db_hdf5_get_comp_var(dbfile->cwg, name, NULL,
                 NULL, NULL, &result)) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
        }

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Dclose(dset);
            H5Tclose(ftype);
            H5Sclose(space);
        } H5E_END_TRY;
        if (result) free(result);
    } END_PROTECT;

    return result;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_ReadVar
 *
 * Purpose:     Reads a variable into the given space.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb 15 14:53:29 PST 2005
 *   Forced it to ignore force_single setting 
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_ReadVar(DBfile *_dbfile, char *vname, void *result)
{
   DBfile_hdf5  *dbfile = (DBfile_hdf5*)_dbfile;
   static char  *me = "db_hdf5_ReadVar";
   hid_t        dset=-1, mtype=-1, ftype=-1;

   PROTECT {

       /* Get dataset and data type */
       if ((dset=H5Dopen(dbfile->cwg, vname))>=0) {
           if ((ftype=H5Dget_type(dset))<0) {
               db_perror(vname, E_CALLFAIL, me);
               UNWIND();
           }

           /* Memory data type is based on file data type */
           if ((mtype=hdf2hdf_type(ftype))<0) {
               db_perror("data type", E_BADARGS, me);
               UNWIND();
           }

           P_rdprops = H5P_DEFAULT;
           if (!SILO_Globals.enableChecksums)
               P_rdprops = P_ckrdprops;

           /* Read entire variable */
           if (H5Dread(dset, mtype, H5S_ALL, H5S_ALL, P_rdprops, result)<0) {
               hdf5_to_silo_error(vname, me);
               UNWIND();
           }

           /* Close everything */
           H5Dclose(dset);
           H5Tclose(ftype);
        }
        else
        {
            if (!db_hdf5_get_comp_var(dbfile->cwg, vname, NULL,
                 NULL, NULL, &result)) {
                db_perror(vname, E_CALLFAIL, me);
                UNWIND();
            }
        }

   } CLEANUP {
       H5E_BEGIN_TRY {
           H5Dclose(dset);
           H5Tclose(ftype);
       } H5E_END_TRY;
   } END_PROTECT;
   
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_ReadVarSlice
 *
 * Purpose:     Reads a slice of a variable into the given memory. The slice
 *              is described with a multi-dimensional offset, length, and
 *              stride and is translated into an HDF5 hyperslab selection.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_ReadVarSlice(DBfile *_dbfile, char *vname, int *offset, int *length,
                     int *stride, int ndims, void *result)
{
   DBfile_hdf5  *dbfile = (DBfile_hdf5*)_dbfile;
   static char  *me = "db_hdf5_ReadVarSlice";
   hid_t        dset=-1, ftype=-1, mtype=-1, mspace=-1, fspace=-1;
   hsize_t      mem_size[H5S_MAX_RANK];

   PROTECT {
       /* Get dataset and data type */
       if ((dset=H5Dopen(dbfile->cwg, vname))<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }
       if ((ftype=H5Dget_type(dset))<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }

       /* Memory data type is based on file data type */
       if ((mtype=hdf2hdf_type(ftype))<0) {
           db_perror("data type", E_BADARGS, me);
           UNWIND();
       }

       /* Build file selection */
       if ((fspace=build_fspace(dset, ndims, offset, length, stride,
                                mem_size))<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }
       
       /* Build the memory space */
       if ((mspace=H5Screate_simple(ndims, mem_size, NULL))<0) {
           db_perror("memory data space", E_CALLFAIL, me);
           UNWIND();
       }

       P_rdprops = H5P_DEFAULT;
       if (!SILO_Globals.enableChecksums)
           P_rdprops = P_ckrdprops;

       /* Read the data */
       if (H5Dread(dset, mtype, mspace, fspace, P_rdprops, result)<0) {
           hdf5_to_silo_error(vname, me);
           UNWIND();
       }
   
       /* Close everything */
       H5Dclose(dset);
       H5Tclose(ftype);
       H5Sclose(fspace);
       H5Sclose(mspace);
       
   } CLEANUP {
       H5E_BEGIN_TRY {
           H5Dclose(dset);
           H5Tclose(ftype);
           H5Sclose(fspace);
           H5Sclose(mspace);
       } H5E_END_TRY;
   } END_PROTECT;
   
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_Write
 *
 * Purpose:     Writes a single variable into a file. The variable may
 *              already exist, in which case the NDIMS and DIMS must match
 *              what has already been defined.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_Write(DBfile *_dbfile, char *vname, void *var,
              int *dims, int ndims, int datatype)
{
   DBfile_hdf5  *dbfile = (DBfile_hdf5*)_dbfile;
   static char  *me = "db_hdf5_Write";
   hid_t        mtype=-1, ftype=-1, space=-1, dset=-1;
   hsize_t      ds_size[H5S_MAX_RANK];
   int          i;

   PROTECT {
       /* Create the memory and file data type */
       if ((mtype=silom2hdfm_type(datatype))<0 ||
           (ftype=silof2hdff_type(dbfile, datatype))<0) {
           db_perror("datatype", E_BADARGS, me);
           UNWIND();
       }

       /*
        * If the dataset already exists then make sure that the supplied
        * NDIMS and DIMS match what's already defined; otherwise create a new
        * dataset.
        */
       H5E_BEGIN_TRY {
           dset = H5Dopen(dbfile->cwg, vname);
       } H5E_END_TRY;
       if (dset>=0) {
           space = H5Dget_space(dset);
           if (ndims!=H5Sget_simple_extent_ndims(space)) {
               db_perror("ndims", E_BADARGS, me);
               UNWIND();
           }
           H5Sget_simple_extent_dims(space, ds_size, NULL);
           for (i=0; i<ndims; i++) {
               if (ds_size[i]!=(hsize_t)dims[i]) {
                   db_perror("dims", E_BADARGS, me);
                   UNWIND();
               }
           }
       } else {
           /* Create memory and file data space (both identical) */
           for (i=0; i<ndims; i++) ds_size[i] = dims[i];
           if ((space=H5Screate_simple(ndims, ds_size, NULL))<0) {
               db_perror("data space", E_CALLFAIL, me);
               UNWIND();
           }

           P_crprops = H5P_DEFAULT;
           if (SILO_Globals.enableChecksums)
           {
               H5Pset_chunk(P_ckcrprops, ndims, ds_size);
               P_crprops = P_ckcrprops;
           }

           /* Create dataset if it doesn't already exist */
           if ((dset=H5Dcreate(dbfile->cwg, vname, ftype, space,
                               P_crprops))<0) {
               db_perror(vname, E_CALLFAIL, me);
               UNWIND();
           }
       }
       
       /* Write data */
       if (H5Dwrite(dset, mtype, space, space, H5P_DEFAULT, var)<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }

       /* Close everything */
       H5Dclose(dset);
       H5Sclose(space);
   } CLEANUP {
       H5E_BEGIN_TRY {
           H5Dclose(dset);
           H5Sclose(space);
       } H5E_END_TRY;
   } END_PROTECT;
   
   return 0;
}


/*------------------------------------------------------------------------- 
 * Function:    db_hdf5_WriteSlice
 *
 * Purpose:     Similar to db_hdf5_ReadVarSlice() except it writes data
 *              instead.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February  9, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_WriteSlice(DBfile *_dbfile, char *vname, void *values, int dtype,
                   int offset[], int length[], int stride[], int dims[],
                   int ndims)
{
   DBfile_hdf5  *dbfile = (DBfile_hdf5*)_dbfile ;
   static char  *me = "db_hdf5_WriteSlice" ;
   hid_t        mtype=-1, ftype=-1, fspace=-1, mspace=-1, dset=-1;
   hsize_t      ds_size[H5S_MAX_RANK];
   int          i;

   PROTECT {
       if ((mtype=silom2hdfm_type(dtype))<0 ||
           (ftype=silof2hdff_type(dbfile, dtype))<0) {
           db_perror("datatype", E_BADARGS, me);
           UNWIND();
       }
       
       /*
        * If the dataset already exists then make sure that the supplied
        * NDIMS and DIMS match what's already defined. Otherwise create the
        * dataset.
        */
       H5E_BEGIN_TRY {
           dset = H5Dopen(dbfile->cwg, vname);
       } H5E_END_TRY;
       if (dset>=0) {
           fspace = H5Dget_space(dset);
           if (ndims!=H5Sget_simple_extent_ndims(fspace)) {
               db_perror("ndims", E_BADARGS, me);
               UNWIND();
           }
           H5Sget_simple_extent_dims(fspace, ds_size, NULL);
           for (i=0; i<ndims; i++) {
               if (ds_size[i]!=(hsize_t)dims[i]) {
                   db_perror("dims", E_BADARGS, me);
                   UNWIND();
               }
           }
           H5Sclose(fspace);
       } else {
           for (i=0; i<ndims; i++) ds_size[i] = dims[i];
           if ((fspace=H5Screate_simple(ndims, ds_size, NULL))<0) {
               db_perror("data space", E_CALLFAIL, me);
               UNWIND();
           }

           P_crprops = H5P_DEFAULT;
           if (SILO_Globals.enableChecksums)
           {
               H5Pset_chunk(P_ckcrprops, ndims, ds_size);
               P_crprops = P_ckcrprops;
           }

           if ((dset=H5Dcreate(dbfile->cwg, vname, ftype, fspace,
                               P_crprops))<0) {
               db_perror(vname, E_CALLFAIL, me);
               UNWIND();
           }
           H5Sclose(fspace);
       }

       /*
        * Verify that offset and length are compatible with the supplied
        * dimensions.
        */
       for (i=0; i<ndims; i++) {
           if (offset[i]<0 || offset[i]>=dims[i]) {
               db_perror("offset", E_BADARGS, me);
               UNWIND();
           }
           if (length[i]<=0 || length[i]>dims[i]) {
               db_perror("length", E_BADARGS, me);
               UNWIND();
           }
           if (offset[i]+length[i]>dims[i]) {
               db_perror("offset+length", E_BADARGS, me);
               UNWIND();
           }
       }
       
       /* Build the file space selection */
       if ((fspace=build_fspace(dset, ndims, offset, length, stride,
                                ds_size/*out*/))<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }

       /* Build the memory data space */
       if ((mspace=H5Screate_simple(ndims, ds_size, NULL))<0) {
           db_perror("memory data space", E_CALLFAIL, me);
           UNWIND();
       }

       /* Write data */
       if (H5Dwrite(dset, mtype, mspace, fspace, H5P_DEFAULT, values)<0) {
           db_perror(vname, E_CALLFAIL, me);
           UNWIND();
       }

       /* Close everything */
       H5Dclose(dset);
       H5Sclose(fspace);
       H5Sclose(mspace);
   } CLEANUP {
       H5E_BEGIN_TRY {
           H5Dclose(dset);
           H5Sclose(fspace);
           H5Sclose(mspace);
       } H5E_END_TRY;
   } END_PROTECT;
   
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetObject
 *
 * Purpose:     Reads information about a silo object.
 *
 * Return:      Success:        Ptr to new object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   I removed the seen_datatype functionality
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBobject *
db_hdf5_GetObject(DBfile *_dbfile, char *name)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_GetObject";
    hid_t       o=-1, attr=-1, atype=-1, s1024=-1;
    char        *file_value=NULL, *mem_value=NULL, *bkg=NULL, bigname[1024];
    DBObjectType objtype;
    int         _objtype, nmembs, i, j, memb_size[4];
    DBobject    *obj=NULL;
    size_t      asize, nelmts, msize;

    PROTECT {
        /* Open the object as a named data type */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }

        /* Open the `silo_type' attribute and read it */
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        objtype = (DBObjectType)_objtype;
        
        /*
         * Open the `silo' attribute (all silo objects have one), retrieve
         * its data type (a single-level H5T_COMPOUND type), and read the
         * attribute.
         */
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            (atype=H5Aget_type(attr))<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        asize = H5Tget_size(atype);
        msize = MAX(asize, 3*1024);
        if (NULL==(file_value=malloc(asize)) ||
            NULL==(mem_value=malloc(msize)) ||
            NULL==(bkg=malloc(msize))) {
            db_perror(name, E_NOMEM, me);
            UNWIND();
        }
        if (H5Aread(attr, atype, file_value)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        nmembs = H5Tget_nmembers(atype);

        /* Create the empty DBobject */
        if (NULL==(obj=DBMakeObject(name, objtype, 3*nmembs))) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Add members to the DBobject */
        s1024 = H5Tcopy(H5T_C_S1);
        H5Tset_size(s1024, 1024);
        for (i=0; i<nmembs; i++) {
            int ndims;
            hid_t member_type = db_hdf5_get_cmemb(atype, i, &ndims, memb_size);
            char *name = H5Tget_member_name(atype, i);
            hid_t mtype = H5Tcreate(H5T_COMPOUND, msize);
            for (nelmts=1, j=0; j<ndims; j++) nelmts *= memb_size[j];
            
            switch (H5Tget_class(member_type)) {
            case H5T_INTEGER:
                db_hdf5_put_cmemb(mtype, name, 0, ndims, memb_size,
                                  H5T_NATIVE_INT);
                memcpy(mem_value, file_value, H5Tget_size(atype));
                H5Tconvert(atype, mtype, 1, mem_value, bkg, H5P_DEFAULT);
                if (1==nelmts) {
                    DBAddIntComponent(obj, name, *((int*)mem_value));
                } else {
                    for (j=0; (size_t)j<nelmts; j++) {
                        sprintf(bigname, "%s%d", name, j+1);
                        DBAddIntComponent(obj, bigname, ((int*)mem_value)[j]);
                    }
                }
                break;

            case H5T_FLOAT:
                db_hdf5_put_cmemb(mtype, name, 0, ndims, memb_size,
                                  H5T_NATIVE_DOUBLE);
                memcpy(mem_value, file_value, H5Tget_size(atype));
                H5Tconvert(atype, mtype, 1, mem_value, bkg, H5P_DEFAULT);
                if (1==nelmts) {
                    DBAddFltComponent(obj, name, *((double*)mem_value));
                } else {
                    for (j=0; (size_t)j<nelmts; j++) {
                        sprintf(bigname, "%s%d", name, j+1);
                        DBAddFltComponent(obj, bigname,
                                          ((double*)mem_value)[j]);
                    }
                }
                break;

            case H5T_STRING:
                db_hdf5_put_cmemb(mtype, name, 0, ndims, memb_size, s1024);
                memcpy(mem_value, file_value, H5Tget_size(atype));
                H5Tconvert(atype, mtype, 1, mem_value, bkg, H5P_DEFAULT);
                if (1==nelmts) {
                    DBAddStrComponent(obj, name, mem_value);
                } else {
                    for (j=0; (size_t)j<nelmts; j++) {
                        sprintf(bigname, "%s%d", name, j+1);
                        DBAddStrComponent(obj, bigname,
                                          mem_value+j*1024);
                    }
                }
                break;

            default:
                /* Silo doesn't handle other types */
                break;
            }

            /* Release member resources */
            free(name);
            H5Tclose(mtype);
            H5Tclose(member_type);
        }

        /* Cleanup */
        H5Tclose(atype);
        H5Tclose(s1024);
        H5Aclose(attr);
        H5Tclose(o);
        free(file_value);
        free(mem_value);
        free(bkg);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Tclose(atype);
            H5Tclose(s1024);
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        if (file_value) free(file_value);
        if (mem_value) free(mem_value);
        if (bkg) free(bkg);
    } END_PROTECT;

    return obj;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutCurve
 *
 * Purpose:     Put a curve object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1, db_errno set
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 23, 1999
 *
 * Modifications:
 *
 *  Thomas R. Treadway, Fri Jul  7 12:44:38 PDT 2006
 *  Added support for DBOPT_REFERENCE in Curves
 *
 *  Mark C. Miller, Mon Jul 31 17:57:29 PDT 2006
 *  Eliminated use of db_hdf5_fullname for xvarname and yvarname
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutCurve(DBfile *_dbfile, char *name, void *xvals, void *yvals,
                 int dtype, int npts, DBoptlist *opts)
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_PutCurve";
    DBcurve_mt  m;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Check datatype */
        if (DB_FLOAT!=dtype && DB_DOUBLE!=dtype) {
            db_perror("invalid floating-point datatype", E_BADARGS, me);
            UNWIND();
        }
        
        /* Reset global curve options */
        memset(&_cu, 0, sizeof _cu);
        if (0!=db_ProcessOptlist(DB_CURVE, opts)) {
            db_perror("bad options", E_CALLFAIL, me);
            UNWIND();
        }

        if (_cu._reference && (xvals || yvals)) {
            db_perror("xvals and yvals can not be defined with DBOPT_REFERENCE",
                      E_BADARGS, me);
            UNWIND();
        }
        /* Write X and Y arrays if supplied */
        strcpy(m.xvarname, OPT(_cu._varname[0]));
        if (xvals) {
            db_hdf5_compwr(dbfile, dtype, 1, &npts, xvals, m.xvarname/*out*/);
        } else if (!_cu._varname[0] && !_cu._reference) {
            db_perror("one of xvals or xvarname must be specified",
                      E_BADARGS, me);
            UNWIND();
        }

        strcpy(m.yvarname, OPT(_cu._varname[1])); 
        if (yvals) {
            db_hdf5_compwr(dbfile, dtype, 1, &npts, yvals, m.yvarname/*out*/);
        } else if (!_cu._varname[1] && !_cu._reference) {
            db_perror("one of yvals or yvarname must be specified",
                      E_BADARGS, me);
            UNWIND();
        }

        /* Build the curve header in memory */
        m.npts = npts;
        m.guihide = _cu._guihide;
        strcpy(m.label, OPT(_cu._label));
        strcpy(m.xlabel, OPT(_cu._labels[0]));
        strcpy(m.ylabel, OPT(_cu._labels[1]));
        strcpy(m.xunits, OPT(_cu._units[0]));
        strcpy(m.yunits, OPT(_cu._units[1]));
        strcpy(m.reference, OPT(_cu._reference));

        /* Write curve header to file */
        STRUCT(DBcurve) {
            if (m.npts) MEMBER_S(int, npts);
            if (m.guihide) MEMBER_S(int, guihide);
            MEMBER_S(str(_cu._label), label);
            MEMBER_S(str(m.xvarname), xvarname);
            MEMBER_S(str(m.yvarname), yvarname);
            MEMBER_S(str(_cu._labels[0]), xlabel);
            MEMBER_S(str(_cu._labels[1]), ylabel);
            MEMBER_S(str(_cu._units[0]), xunits);
            MEMBER_S(str(_cu._units[1]), yunits);
            MEMBER_S(str(_cu._reference), reference);
        } OUTPUT(dbfile, DB_CURVE, name, &m);

    } CLEANUP {
        /*void*/;
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetCurve
 *
 * Purpose:     Read a curve object from a PDB data file.
 *
 * Return:      Success:        Pointer to new curve object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 25, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set datatype correctly. Added support for dataReadMask
 *
 *  Thomas R. Treadway, Fri Jul  7 12:44:38 PDT 2006
 *  Added support for DBOPT_REFERENCE in Curves
 *
 *   Mark C. Miller, Thu Sep  7 10:50:55 PDT 2006
 *   Added use of db_hdf5_resolvename for retrieval of x-data object
 *-------------------------------------------------------------------------
 */
CALLBACK DBcurve *
db_hdf5_GetCurve(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetCurve";
    DBcurve             *cu = NULL;
    DBcurve_mt          m;
    hid_t               o=-1, attr=-1;
    int                 _objtype;

    PROTECT {
        /* Open object and make sure it's a curve */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_CURVE!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read the curve data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBcurve_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a curve object and initialize meta data */
        if (NULL==(cu=DBAllocCurve())) return NULL;
        cu->npts = m.npts;
        cu->guihide = m.guihide;
        if ((cu->datatype = db_hdf5_GetVarType(_dbfile, 
                                db_hdf5_resolvename(_dbfile, name, m.xvarname))) < 0)
            cu->datatype = DB_FLOAT;
        if (cu->datatype == DB_DOUBLE && force_single_g)
            cu->datatype = DB_FLOAT;
        cu->title = OPTDUP(m.label);
        cu->xvarname = OPTDUP(m.xvarname);
        cu->yvarname = OPTDUP(m.yvarname);
        cu->xlabel = OPTDUP(m.xlabel);
        cu->ylabel = OPTDUP(m.ylabel);
        cu->xunits = OPTDUP(m.xunits);
        cu->yunits = OPTDUP(m.yunits);
        cu->reference = OPTDUP(m.reference);
        
        /* Read X and Y data */
        if (SILO_Globals.dataReadMask & DBCurveArrays)
        {
            if (cu->reference) {
                cu->x = NULL;
                cu->y = NULL;
            } else {
                cu->x = db_hdf5_comprd(dbfile, m.xvarname, 0);
                cu->y = db_hdf5_comprd(dbfile, m.yvarname, 0);
            }
        }
        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeCurve(cu);
    } END_PROTECT;

    return cu;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutCsgmesh
 *
 * Purpose:     Writes a Csgmesh to the silo file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Mark C. Miller 
 *              Tuesday, September 6, 2005 
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Jul 31 17:57:29 PDT 2006
 *   Eliminated use of db_hdf5_fullname for zonel_name
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutCsgmesh(DBfile *_dbfile, const char *name, int ndims,
                   int nbounds, const int *typeflags, const int *bndids,
                   const void *coeffs, int lcoeffs, int datatype,
                   const double *extents, const char *zonel_name,
                   DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutCsgmesh";
    int                 i,len;
    DBcsgmesh_mt        m;

    memset(&m, 0, sizeof m); 
    
    PROTECT {

        /* Set global options */
        db_ResetGlobalData_Csgmesh();
        if (db_ProcessOptlist(DB_CSGMESH, optlist)<0) {
            db_perror("bad options", E_CALLFAIL, me);
            UNWIND();
        }

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_csgm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_csgm._time), "time");
        }
        if (_csgm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_csgm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_csgm._cycle), "cycle");

        m.min_extents[0] = extents[0];
        m.min_extents[1] = extents[1];
        m.min_extents[2] = extents[2];
        m.max_extents[0] = extents[3];
        m.max_extents[1] = extents[4];
        m.max_extents[2] = extents[5];

        db_hdf5_compwr(dbfile,DB_INT,1,&nbounds,(void*)typeflags,m.typeflags/*out*/);
        if (bndids)
            db_hdf5_compwr(dbfile,DB_INT,1,&nbounds,(void*)bndids,m.bndids/*out*/);
        db_hdf5_compwr(dbfile,datatype,1,&lcoeffs,(void*)coeffs,m.coeffs/*out*/);

        /* Build csgmesh header in memory */
        m.ndims = ndims;
        m.cycle = _csgm._cycle;
        m.origin = _csgm._origin;
        m.group_no = _csgm._group_no;
        m.guihide = _csgm._guihide;
        strcpy(m.name, name);
        for (i=0; i<ndims; i++) {
            strcpy(m.labels[i], OPT(_csgm._labels[i]));
            strcpy(m.units[i], OPT(_csgm._units[i]));
        }
        m.time = _csgm._time_set ? _csgm._time : 0;
        m.dtime = _csgm._dtime_set ? _csgm._dtime : 0;
        m.nbounds = nbounds;
        m.lcoeffs = lcoeffs;
        strcpy(m.zonel_name, zonel_name);

        /* Build csgmesh header in file */
        STRUCT(DBcsgmesh) {
            if (m.group_no)       MEMBER_S(int, group_no);
            if (m.cycle)          MEMBER_S(int, cycle);
            if (_csgm._time_set)  MEMBER_S(float, time);
            if (_csgm._dtime_set) MEMBER_S(double, dtime);
            if (m.origin)         MEMBER_S(int, origin);
            if (bndids)           MEMBER_S(str(m.bndids), bndids);
            if (m.guihide)        MEMBER_S(int, guihide);
            MEMBER_S(int, lcoeffs);
            MEMBER_S(int, nbounds);
            MEMBER_S(int, ndims);
            MEMBER_3(double, min_extents);
            MEMBER_3(double, max_extents);
            MEMBER_R(str(m.units[_j]), units, ndims);
            MEMBER_R(str(m.labels[_j]), labels, ndims);
            MEMBER_S(str(m.typeflags), typeflags);
            MEMBER_S(str(m.coeffs), coeffs);
            MEMBER_S(str(m.zonel_name), zonel_name);
        } OUTPUT(dbfile, DB_CSGMESH, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetCsgmesh
 *
 * Purpose:     Reads a CSG mesh object from the file.
 *
 * Return:      Success:        Pointer to a new csgmesh.
 *
 *              Failure:        NULL
 *
 * Programmer:  Mark C. Miller 
 *              Wednesday, September 7, 2005 
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Sep  7 10:50:55 PDT 2006
 *   Added use of db_hdf5_resolvename for retrieval of subobjects
 *-------------------------------------------------------------------------
 */
CALLBACK DBcsgmesh *
db_hdf5_GetCsgmesh(DBfile *_dbfile, const char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetCsgmesh";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBcsgmesh_mt        m;
    DBcsgmesh           *csgm=NULL;

    PROTECT {
        /* Open object and make sure it's a csgmesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror((char*)name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_CSGMESH!=(DBObjectType)_objtype) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read header into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBcsgmesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a ucdmesh object and initialize meta data */
        if (NULL==(csgm=DBAllocCsgmesh())) return NULL;
        csgm->name = BASEDUP(name);
        csgm->cycle = m.cycle;
        if ((csgm->datatype = db_hdf5_GetVarType(_dbfile, m.coeffs)) < 0)
            csgm->datatype = DB_FLOAT;
        if (csgm->datatype == DB_DOUBLE && force_single_g)
            csgm->datatype = DB_FLOAT;
        csgm->time = m.time;
        csgm->dtime = m.dtime;
        csgm->ndims = m.ndims;
        csgm->nbounds = m.nbounds;
        csgm->lcoeffs = m.lcoeffs;
        csgm->origin = m.origin;
        csgm->group_no = m.group_no;
        csgm->guihide = m.guihide;
        for (i=0; i<m.ndims; i++) {
            csgm->units[i] = OPTDUP(m.units[i]);
            csgm->labels[i] = OPTDUP(m.labels[i]);
            csgm->min_extents[i] = m.min_extents[i];
            csgm->max_extents[i] = m.max_extents[i];
        }

        /* Read the raw data */
        if ((SILO_Globals.dataReadMask & DBCSGMBoundaryInfo) && (m.nbounds > 0))
        {
            csgm->typeflags = db_hdf5_comprd(dbfile, m.typeflags, 1);
            csgm->bndids = db_hdf5_comprd(dbfile, m.bndids, 1);
        }

        if ((SILO_Globals.dataReadMask & DBCSGMBoundaryNames) && (m.nbounds > 0))
        {
            char *tmpbndnames = db_hdf5_comprd(dbfile, m.bndnames, 1);
            if (tmpbndnames)
                csgm->bndnames = db_StringListToStringArray(tmpbndnames, m.nbounds);
            FREE(tmpbndnames);
        }

        if ((SILO_Globals.dataReadMask & DBCSGMBoundaryInfo) && (m.lcoeffs > 0))
            csgm->coeffs = db_hdf5_comprd(dbfile, m.coeffs, 0);

        if ((m.zonel_name[0] && (SILO_Globals.dataReadMask & DBCSGMZonelist)))
            csgm->zones = db_hdf5_GetCSGZonelist(_dbfile, 
                              db_hdf5_resolvename(_dbfile, name, m.zonel_name));

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
    } END_PROTECT;

    return csgm;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutCsgvar
 *
 * Purpose:     Writes CSG variables to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Mark C. Miller 
 *              Wednesday, September 7, 2005 
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutCsgvar(DBfile *_dbfile, const char *vname, const char *meshname,
                  int nvars, const char *varnames[], const void *vars[],
                  int nvals, int datatype, int centering, DBoptlist *optlist)

{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutCsgvar";
    DBcsgvar_mt         m;
    int                 i, saved_ndims, saved_nnodes, saved_nzones, len;

    memset(&m, 0, sizeof m);

    PROTECT {
        db_ResetGlobalData_Csgmesh();
        strcpy(_csgm._meshname, meshname);
        db_ProcessOptlist(DB_CSGMESH, optlist);

        /* Write variable arrays: vars[] */
        if (nvars>MAX_VARS) {
            db_perror("too many variables", E_BADARGS, me);
            UNWIND();
        }
        for (i=0; i<nvars; i++) {
            db_hdf5_compwr(dbfile, datatype, 1, &nvals, (void*)vars[i],
                           m.vals[i]/*out*/);
        }

        /* Build header in memory */
        m.nvals = nvars;
        m.nels = nvals;
        m.centering = centering;
        m.cycle = _csgm._cycle;
        m.guihide = _csgm._guihide;
        if (_csgm._time_set)
            m.time = _csgm._time;
        if (_csgm._dtime_set)
            m.dtime = _um._dtime;
        m.use_specmf = _um._use_specmf;
        m.datatype = datatype;
        strcpy(m.meshname, OPT(_csgm._meshname));
        strcpy(m.label, OPT(_csgm._label));
        strcpy(m.units, OPT(_csgm._unit));

        /* Write header to file */
        STRUCT(DBcsgvar) {
            MEMBER_R(str(m.vals[_j]), vals, nvars);
            MEMBER_S(str(m.meshname), meshname);
            MEMBER_S(int, cycle);
            MEMBER_S(str(m.label), label);
            MEMBER_S(str(m.units), units);
            if (m.nvals)        MEMBER_S(int, nvals);
            if (m.nels)         MEMBER_S(int, nels);
            if (m.centering)    MEMBER_S(int, centering);
            if (m.use_specmf)   MEMBER_S(int, use_specmf);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (_csgm._time_set)  MEMBER_S(float, time);
            if (_csgm._dtime_set) MEMBER_S(double, dtime);
        } OUTPUT(dbfile, DB_CSGVAR, vname, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetCsgvar
 *
 * Purpose:     Reads a CSG variable object from the file.
 *
 * Return:      Success:        Ptr to new CSG variable object
 *
 *              Failure:        NULL
 *
 * Programmer:  Mark C. Miller
 *              Wednesday, September 7, 2005 
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBcsgvar *
db_hdf5_GetCsgvar(DBfile *_dbfile, const char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetCsgvar";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBcsgvar_mt         m;
    DBcsgvar           *csgv=NULL;

    PROTECT {
        /* Open object and make sure it's a ucdvar */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror((char*)name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_CSGVAR!=(DBObjectType)_objtype) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read ucdvar data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBcsgvar_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a ucdvar object and initialize meta data */
        if (NULL==(csgv=DBAllocCsgvar())) return NULL;
        csgv->name = BASEDUP(name);
        csgv->meshname = OPTDUP(m.meshname);
        csgv->cycle = m.cycle;
        csgv->units = OPTDUP(m.units);
        csgv->label = OPTDUP(m.label);
        csgv->time = m.time;
        csgv->dtime = m.dtime;
        if ((csgv->datatype = db_hdf5_GetVarType(_dbfile, m.vals[0])) < 0)
            csgv->datatype = silo2silo_type(m.datatype);
        if (csgv->datatype == DB_DOUBLE && force_single_g)
            csgv->datatype = DB_FLOAT;
        csgv->nels = m.nels;
        csgv->nvals = m.nvals;
        csgv->centering = m.centering;
        csgv->use_specmf = m.use_specmf;
        csgv->ascii_labels = m.ascii_labels;
        csgv->guihide = m.guihide;

        /* Read the raw data */
        if (m.nvals>MAX_VARS) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        if (SILO_Globals.dataReadMask & DBCSGVData)
        {
            csgv->vals = calloc(m.nvals, sizeof(void*));
            for (i=0; i<m.nvals; i++) {
                csgv->vals[i] = db_hdf5_comprd(dbfile, m.vals[i], 0);
            }
        }
        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeCsgvar(csgv);
    } END_PROTECT;

    return csgv;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutCSGZonelist
 *
 * Purpose:     Write a DBcsgzonelist object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Mark C. Miller 
 *              Wednesday, September 7, 2005 
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutCSGZonelist(DBfile *_dbfile, const char *name, int nregs,
                 const int *typeflags,
                 const int *leftids, const int *rightids,
                 const void *xforms, int lxforms, int datatype,
                 int nzones, const int *zonelist, DBoptlist *optlist)

{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBcsgzonelist_mt     m;
    static char         *me = "db_hdf5_PutCSGZonelist";

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        memset(&_csgzl, 0, sizeof _csgzl);
        if (db_ProcessOptlist(DB_CSGZONELIST, optlist)<0) {
            db_perror("bad options", E_CALLFAIL, me);
            UNWIND();
        }
        
        /* Write variable arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &nregs, (void*)typeflags,
                       m.typeflags/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nregs, (void*)leftids,
                       m.leftids/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nregs, (void*)rightids,
                       m.rightids/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nzones, (void*)zonelist,
                       m.zonelist/*out*/);
        if (xforms && lxforms > 0) {
            db_hdf5_compwr(dbfile, datatype, 1, &lxforms, (void*)xforms,
                           m.xform/*out*/);
        }

        if (_csgzl._regnames) {
            int len; char *tmp;
            db_StringArrayToStringList((const char**) _csgzl._regnames, nregs, &tmp, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, tmp,
                           m.regnames/*out*/);
            FREE(tmp);
        }

        if (_csgzl._zonenames) {
            int len; char *tmp;
            db_StringArrayToStringList((const char**) _csgzl._zonenames, nzones, &tmp, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, tmp,
                           m.zonenames/*out*/);
            FREE(tmp);
        }

        /* Build header in memory */
        m.nregs = nregs;
        m.lxform = lxforms;
        m.nzones = nzones;

        /* Write header to file */
        STRUCT(DBcsgzonelist) {
            if (m.nregs)        MEMBER_S(int, nregs);
            if (m.lxform)       MEMBER_S(int, lxform);
            if (m.nzones)       MEMBER_S(int, nzones);
            MEMBER_S(str(m.typeflags), typeflags);
            MEMBER_S(str(m.leftids), leftids);
            MEMBER_S(str(m.rightids), rightids);
            MEMBER_S(str(m.zonelist), zonelist);
            MEMBER_S(str(m.xform), xform);
            MEMBER_S(str(m.regnames), regnames);
            MEMBER_S(str(m.zonenames), zonenames);
        } OUTPUT(dbfile, DB_CSGZONELIST, name, &m);
        
    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetCSGZonelist
 *
 * Purpose:     Reads a CSG zonelist object from the file
 *
 * Return:      Success:        Ptr to new CSG zonelist
 *
 *              Failure:        NULL
 *
 * Programmer:  Mark C. Miller 
 *              Wednesday, September 7, 2005 
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Mar 23 15:31:50 PST 2006
 *   Fixed case where we were forcing single on an integer array for the
 *   zonelist.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBcsgzonelist *
db_hdf5_GetCSGZonelist(DBfile *_dbfile, const char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetCSGZonelist";
    hid_t               o=-1, attr=-1;
    int                 _objtype;
    DBcsgzonelist_mt     m;
    DBcsgzonelist       *zl=NULL;

    PROTECT {
        /* Open object and make sure it's a zonelist */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror((char*)name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_CSGZONELIST!=(DBObjectType)_objtype) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read zonelist data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBcsgzonelist_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a zonelist object and initialize meta data */
        if (NULL==(zl=DBAllocCSGZonelist())) return NULL;
        zl->nregs = m.nregs;
        zl->nzones = m.nzones;
        zl->lxform = m.lxform;
        if ((zl->datatype = db_hdf5_GetVarType(_dbfile, m.xform)) < 0)
            zl->datatype = DB_FLOAT;
        if (zl->datatype == DB_DOUBLE && force_single_g)
            zl->datatype = DB_FLOAT;

        /* Read the raw data */
        if (SILO_Globals.dataReadMask & DBZonelistInfo)
        {
            zl->typeflags = db_hdf5_comprd(dbfile, m.typeflags, 1);
            zl->leftids = db_hdf5_comprd(dbfile, m.leftids, 1);
            zl->rightids = db_hdf5_comprd(dbfile, m.rightids, 1);
            zl->xform = db_hdf5_comprd(dbfile, m.xform, 0);
            zl->zonelist = db_hdf5_comprd(dbfile, m.zonelist, 1);
        }

        if (SILO_Globals.dataReadMask & DBCSGZonelistRegNames)
        {
            char *tmpnames = db_hdf5_comprd(dbfile, m.regnames, 1);
            if (tmpnames)
                zl->regnames = db_StringListToStringArray(tmpnames, m.nregs);
            FREE(tmpnames);
        }

        if (SILO_Globals.dataReadMask & DBCSGZonelistZoneNames)
        {
            char *tmpnames = db_hdf5_comprd(dbfile, m.zonenames, 1);
            if (tmpnames)
                zl->zonenames = db_StringListToStringArray(tmpnames, m.nzones);
            FREE(tmpnames);
        }

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeCSGZonelist(zl);
    } END_PROTECT;

    return zl;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutDefvars
 *
 * Purpose:     Put a defvars object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1, db_errno set
 *
 * Programmer:  Mark C. Miller 
 *              Tuesday, September 6, 2005 
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutDefvars(DBfile *_dbfile, const char *name, int ndefs,
                   const char *names[], const int *types,
                   const char *defns[], DBoptlist *opts[])
{
    DBfile_hdf5 *dbfile = (DBfile_hdf5*)_dbfile;
    static char *me = "db_hdf5_PutDefvars";
    DBdefvars_mt  m;
    int len;
    char *s;
    int *guihide = NULL;

   /*
    * Optlists are a little funky for this object because we were
    * concerned about possibly handling things like units, etc. So,
    * we really have an array of optlists that needs to get serialized.
    */
   if (opts)
   {
       int i;
       for (i = 0; i < ndefs; i++)
       {
           memset(&_dv, 0, sizeof(_dv)); /* reset global dv data */
           db_ProcessOptlist (DB_DEFVARS, opts[i]);
           if (_dv._guihide)
           {
               if (guihide == NULL)
                   guihide = (int *) calloc(ndefs, sizeof(int));
               guihide[i] = _dv._guihide;
           }
       }
   }

    memset(&m, 0, sizeof m);
    PROTECT {

        db_StringArrayToStringList(names, ndefs, &s, &len);
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.names/*out*/);
        FREE(s);

        db_hdf5_compwr(dbfile, DB_INT, 1, &ndefs, (void*)types, m.types/*out*/);

        db_StringArrayToStringList(defns, ndefs, &s, &len);
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.defns/*out*/);
        FREE(s);

        if (guihide)
        {
            db_hdf5_compwr(dbfile, DB_INT, 1, &ndefs, (void*)guihide, m.guihides/*out*/);
            free(guihide);
        }

        /* Build the curve header in memory */
        m.ndefs = ndefs;

        /* Write curve header to file */
        STRUCT(DBdefvars) {
            if (m.ndefs) MEMBER_S(int, ndefs);
            MEMBER_S(str(m.names), names);
            MEMBER_S(str(m.types), types);
            MEMBER_S(str(m.defns), defns);
            MEMBER_S(str(m.guihides), guihides);
        } OUTPUT(dbfile, DB_DEFVARS, name, &m);

    } CLEANUP {
        /*void*/;
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetDefvars
 *
 * Purpose:     Read a defvars object from a data file.
 *
 * Return:      Success:        Pointer to new defvars object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 25, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set datatype correctly. Added support for dataReadMask
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBdefvars*
db_hdf5_GetDefvars(DBfile *_dbfile, const char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetDefvars";
    DBdefvars           *defv = NULL;
    DBdefvars_mt         m;
    hid_t               o=-1, attr=-1;
    int                 _objtype;
    char               *s;

    PROTECT {
        /* Open object and make sure it's a curve */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror((char*)name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_DEFVARS!=(DBObjectType)_objtype) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read the data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBdefvars_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a defvars object and initialize meta data */
        if (NULL==(defv=DBAllocDefvars(0))) return NULL;
        defv->ndefs = m.ndefs;

        s = db_hdf5_comprd(dbfile, m.names, 1);
        if (s) defv->names = db_StringListToStringArray(s, defv->ndefs);
        FREE(s);

        defv->types = db_hdf5_comprd(dbfile, m.types, 1);

        s = db_hdf5_comprd(dbfile, m.defns, 1);
        if (s) defv->defns = db_StringListToStringArray(s, defv->ndefs);
        FREE(s);

        defv->guihides = db_hdf5_comprd(dbfile, m.guihides, 1);

        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeDefvars(defv);
    } END_PROTECT;

    return defv;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutQuadmesh
 *
 * Purpose:     Writes a Quadmesh to the silo file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Monday, March 29, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no' and `baseindex[]' properties to duplicate
 *              changes made to the PDB driver.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Made it set the correct mesh type
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutQuadmesh(DBfile *_dbfile, char *name, char *coordnames[],
                    float *coords[], int dims[], int ndims, int datatype,
                    int coordtype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutQuadmesh";
    int                 i,len;
    DBquadmesh_mt       m;

    FREE(_qm._meshname);
    memset(&_qm, 0, sizeof _qm);
    memset(&m, 0, sizeof m); 
    
    PROTECT {
        /* Check datatype */
        if (DB_FLOAT!=datatype && DB_DOUBLE!=datatype) {
            db_perror("invalid floating-point datatype", E_BADARGS, me);
            UNWIND();
        }

        /* Set global options */
        _qm._coordsys = DB_OTHER;
        _qm._facetype = DB_RECTILINEAR;
        _qm._ndims = ndims;
        _qm._nspace = ndims;
        _qm._planar = DB_AREA;
        _qm._use_specmf = DB_OFF;
        _qm._group_no = -1;
        if (db_ProcessOptlist(DB_QUADMESH, optlist)<0) {
            db_perror("bad options", E_CALLFAIL, me);
            UNWIND();
        }

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_qm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_qm._time), "time");
        }
        if (_qm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_qm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_qm._cycle), "cycle");

        /*
         * Number of zones and nodes. We have to do this because
         * _DBQMCalcExtents uses this global information.
         */
        for (_qm._nzones=_qm._nnodes=1, i=0; i<ndims; i++) {
            _qm._nzones *= (dims[i]-1);
            _qm._nnodes *= dims[i];
            _qm._dims[i] = dims[i];
            _qm._zones[i] = dims[i]-1;
            _qm._minindex[i] = _qm._lo_offset[i];
            _qm._maxindex_n[i] = dims[i] - _qm._hi_offset[i] - 1;
            _qm._maxindex_z[i] = _qm._maxindex_n[i] - 1;
        }
        
        /* Calculate extents, stored as DB_DOUBLE */
        if (DB_DOUBLE==datatype) {
            _DBQMCalcExtents(coords, datatype, _qm._minindex, _qm._maxindex_n,
                             dims, ndims, coordtype,
                             &(m.min_extents)/*out*/,
                             &(m.max_extents)/*out*/);
        } else {
            float       min_extents[3];
            float       max_extents[3];
            _DBQMCalcExtents(coords, datatype, _qm._minindex, _qm._maxindex_n,
                             dims, ndims, coordtype,
                             min_extents/*out*/, max_extents/*out*/);
            for (i=0; i<ndims; i++) {
                m.min_extents[i] = min_extents[i];
                m.max_extents[i] = max_extents[i];
            }
        }
        for (i=0; i<ndims; i++) {
            m.min_index[i] = _qm._minindex[i];
            m.max_index[i] = _qm._maxindex_n[i];
        }
        
        /* Write coordinate arrays */
        if (DB_COLLINEAR==coordtype) {
            for (i=0; i<ndims; i++) {
                db_hdf5_compwr(dbfile, datatype, 1, dims+i, coords[i],
                               m.coord[i]/*out*/);
            }
        } else {
            for (i=0; i<ndims; i++) {
                db_hdf5_compwr(dbfile, datatype, ndims, dims, coords[i],
                               m.coord[i]/*out*/);
            }
        }
        
        /* Build quadmesh header in memory */
        m.ndims = ndims;
        m.coordtype = coordtype;
        m.nspace = _qm._nspace;
        m.nnodes = _qm._nnodes;
        m.facetype = _qm._facetype;
        m.major_order = _qm._majororder;
        m.cycle = _qm._cycle;
        m.coord_sys = _qm._coordsys;
        m.planar = _qm._planar;
        m.origin = _qm._origin;
        m.group_no = _qm._group_no;
        m.guihide = _qm._guihide;
        for (i=0; i<ndims; i++) {
            m.dims[i] = dims[i];
            m.baseindex[i] = _qm._baseindex[i];
            strcpy(m.label[i], OPT(_qm._labels[i]));
            strcpy(m.units[i], OPT(_qm._units[i]));
        }
        m.time = _qm._time_set ? _qm._time : 0;
        m.dtime = _qm._dtime_set ? _qm._dtime : 0;

        /* Build quadmesh header in file */
        STRUCT(DBquadmesh) {
            MEMBER_R(str(m.coord[_j]), coord, ndims);
            MEMBER_3(double, min_extents);
            MEMBER_3(double, max_extents);
            MEMBER_S(int, ndims);
            MEMBER_S(int, coordtype);
            MEMBER_S(int, nspace);
            MEMBER_S(int, nnodes);
            MEMBER_S(int, facetype);
            if (m.major_order)  MEMBER_S(int, major_order);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.group_no)     MEMBER_S(int, group_no);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_S(int, coord_sys);
            MEMBER_S(int, planar);
            MEMBER_3(int, dims);
            MEMBER_3(int, min_index);
            MEMBER_3(int, max_index);
            MEMBER_3(int, baseindex);
            if (_qm._time_set) MEMBER_S(float, time);
            if (_qm._dtime_set) MEMBER_S(double, dtime);
            MEMBER_R(str(m.label[_j]), label, ndims);
            MEMBER_R(str(m.units[_j]), units, ndims);
        } OUTPUT(dbfile, coordtype == DB_COLLINEAR ? DB_QUAD_RECT : DB_QUAD_CURV, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetQuadmesh
 *
 * Purpose:     Reads a quadmesh object from the file.
 *
 * Return:      Success:        Ptr to new quadmesh
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 30, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no' and `baseindex[]' properties to duplicate
 *              changes made to the PDB driver.
 *
 *              Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *              Added support for dataReadMask
 *-------------------------------------------------------------------------
 */
CALLBACK DBquadmesh *
db_hdf5_GetQuadmesh(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetQuadmesh";
    DBquadmesh          *qm = NULL;
    DBquadmesh_mt       m;
    hid_t               o=-1, attr=-1;
    int                 _objtype, stride, i;
    
    PROTECT {
        /* Open object and make sure it's a quadmesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if ((DB_QUADMESH!=(DBObjectType)_objtype) &&
            (DB_QUAD_CURV!=(DBObjectType)_objtype) &&
            (DB_QUAD_RECT!=(DBObjectType)_objtype)) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read quadmesh data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBquadmesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a quadmesh object and initialize meta data */
        if (NULL==(qm=DBAllocQuadmesh())) return NULL;
        qm->name = BASEDUP(name);
        qm->cycle = m.cycle;
        qm->coord_sys = m.coord_sys;
        qm->major_order = m.major_order;
        qm->coordtype = m.coordtype;
        qm->facetype = m.facetype;
        qm->planar = m.planar;
        if ((qm->datatype = db_hdf5_GetVarType(_dbfile, m.coord[0])) < 0)
            qm->datatype = DB_FLOAT;
        if (qm->datatype == DB_DOUBLE && force_single_g)
            qm->datatype = DB_FLOAT;
        qm->time = m.time;
        qm->dtime = m.dtime;
        qm->ndims = m.ndims;
        qm->nspace = m.nspace;
        qm->nnodes = m.nnodes;
        qm->origin = m.origin;
        qm->group_no = m.group_no;
        qm->guihide = m.guihide;

        for (stride=1, i=0; i<qm->ndims; i++) {
            if (qm->datatype == DB_DOUBLE)
            {
                ((double*)qm->min_extents)[i] = m.min_extents[i];
                ((double*)qm->max_extents)[i] = m.max_extents[i];
            }
            else
            {
                qm->min_extents[i] = m.min_extents[i];
                qm->max_extents[i] = m.max_extents[i];
            }
            qm->labels[i] = OPTDUP(m.label[i]);
            qm->units[i] = OPTDUP(m.units[i]);
            qm->dims[i] = m.dims[i];
            qm->min_index[i] = m.min_index[i];
            qm->max_index[i] = m.max_index[i];
            qm->base_index[i] = m.baseindex[i];
            qm->stride[i] = stride;
            stride *= qm->dims[i];

            /* Read coordinate arrays */
            if (SILO_Globals.dataReadMask & DBQMCoords)
                qm->coords[i] = db_hdf5_comprd(dbfile, m.coord[i], 0);
        }
        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeQuadmesh(qm);
    } END_PROTECT;

    return qm;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutQuadvar
 *
 * Purpose:     Write a quadvar object to the file. The VARS and MIXVARS
 *              arguments should actually be of type `void*[]' because they
 *              point to either `float' or `double' depending on the value of
 *              the DATATYPE argument.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 31, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutQuadvar(DBfile *_dbfile, char *name, char *meshname, int nvars,
                   char *varnames[/*nvars*/], float *vars[/*nvars*/],
                   int dims[/*ndims*/], int ndims, float *mixvars[/*nvars*/],
                   int mixlen, int datatype, int centering, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutQuadvar";
    DBquadvar_mt        m;
    int                 i, nels, len;

    FREE(_qm._meshname);
    memset(&_qm, 0, sizeof _qm);
    memset(&m, 0, sizeof m);

    PROTECT {
        /* Set global options */
        _qm._coordsys = DB_OTHER;
        _qm._facetype = DB_RECTILINEAR;
        _qm._ndims = _qm._nspace = ndims;
        _qm._planar = DB_AREA;
        _qm._use_specmf = DB_OFF;
        _qm._group_no = -1;
        db_ProcessOptlist(DB_QUADMESH, optlist); /*yes, QUADMESH*/
        _qm._meshname = STRDUP(meshname);
        _qm._nzones = _qm._nnodes = 1; /*initial value only*/
        for (nels=1, i=0; i<ndims; i++) {
            nels *= dims[i];
            _qm._nzones *= (dims[i]-1);
            _qm._nnodes *= dims[i];
            _qm._dims[i] = dims[i];
            _qm._zones[i] = dims[i]-1;
            _qm._minindex[i] = _qm._lo_offset[i];
            _qm._maxindex_n[i] = dims[i] - _qm._hi_offset[i] - 1;
            _qm._maxindex_z[i] = _qm._maxindex_n[i] - 1;
        }

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_qm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_qm._time), "time");
        }
        if (_qm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_qm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_qm._cycle), "cycle");

        /* Write variable arrays: vars[] and mixvars[] */
        if (nvars>MAX_VARS) {
            db_perror("too many variables", E_BADARGS, me);
            UNWIND();
        }
        for (i=0; i<nvars; i++) {
            db_hdf5_compwr(dbfile, datatype, ndims, dims, vars[i],
                           m.value[i]/*out*/);
            if (mixvars && mixvars[i] && mixlen>0) {
                db_hdf5_compwr(dbfile, datatype, 1, &mixlen, mixvars[i],
                               m.mixed_value[i]/*out*/);
            }
        }
        
        /* Build quadvar header in memory */
        m.ndims = ndims;
        m.nvals = nvars;
        m.nels = nels;
        m.origin = _qm._origin;
        m.mixlen = mixlen;
        m.major_order = _qm._majororder;
        m.cycle = _qm._cycle;
        m.time = _qm._time;
        m.dtime = _qm._dtime;
        m.use_specmf = _qm._use_specmf;
        m.ascii_labels = _qm._ascii_labels;
        m.guihide = _qm._guihide;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;
        strcpy(m.label, OPT(_qm._label));
        strcpy(m.units, OPT(_qm._unit));
        strcpy(m.meshid, OPT(_qm._meshname));

        for (i=0; i<ndims; i++) {
            m.dims[i] = _qm._dims[i];
            m.zones[i] = _qm._zones[i];
            m.min_index[i] = _qm._minindex[i];
            m.max_index[i] = _qm._maxindex_n[i];
            m.align[i] = DB_NODECENT==centering ? 0.0 : 0.5;
        }

        /* Write quadvar header to file */
        STRUCT(DBquadvar) {
            MEMBER_S(str(m.meshid), meshid);
            MEMBER_R(str(m.value[_j]), value, nvars);
            MEMBER_R(str(m.mixed_value[_j]), mixed_value, nvars);
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nvals)        MEMBER_S(int, nvals);
            if (m.nels)         MEMBER_S(int, nels);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.mixlen)       MEMBER_S(int, mixlen);
            if (m.major_order)  MEMBER_S(int, major_order);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (_qm._time_set)  MEMBER_S(float, time);
            if (_qm._dtime_set) MEMBER_S(double, dtime);
            if (m.use_specmf)   MEMBER_S(int, use_specmf);
            if (m.ascii_labels) MEMBER_S(int, ascii_labels);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_3(int, dims);
            MEMBER_3(int, zones);
            MEMBER_3(int, min_index);
            MEMBER_3(int, max_index);
            MEMBER_3(float, align);
            MEMBER_S(str(m.label), label);
            MEMBER_S(str(m.units), units);
        } OUTPUT(dbfile, DB_QUADVAR, name, &m);
        
    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetQuadvar
 *
 * Purpose:     Reads a quadvar object from the file
 *
 * Return:      Success:        Ptr to new quadvar object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 31, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set datatype correctly. Added support for dataReadMask
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBquadvar *
db_hdf5_GetQuadvar(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetQuadvar";
    hid_t               o=-1, attr=-1;
    int                 _objtype, stride, i;
    DBquadvar_mt        m;
    DBquadvar           *qv=NULL;
    
    PROTECT {
        /* Open object and make sure it's a quadvar */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_QUADVAR!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read quadvar data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBquadvar_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a quadvar object and initialize meta data */
        if (NULL==(qv=DBAllocQuadvar())) return NULL;
        qv->name = BASEDUP(name);
        qv->meshname = OPTDUP(m.meshid);
        qv->units = OPTDUP(m.units);
        qv->label = OPTDUP(m.label);
        qv->cycle = m.cycle;
        if ((qv->datatype = db_hdf5_GetVarType(_dbfile, m.value[0])) < 0)
            qv->datatype = silo2silo_type(m.datatype);
        if (qv->datatype == DB_DOUBLE && force_single_g)
            qv->datatype = DB_FLOAT;
        qv->nels = m.nels;
        qv->nvals = m.nvals;
        qv->ndims = m.ndims;
        qv->major_order = m.major_order;
        qv->origin = m.origin;
        qv->time = m.time;
        qv->dtime = m.dtime;
        qv->mixlen = m.mixlen;
        qv->use_specmf = m.use_specmf;
        qv->ascii_labels = m.ascii_labels;
        qv->guihide = m.guihide;
        for (stride=1, i=0; i<m.ndims; stride*=m.dims[i++]) {
            qv->dims[i] = m.dims[i];
            qv->stride[i] = stride;
            qv->min_index[i] = m.min_index[i];
            qv->max_index[i] = m.max_index[i];
            qv->align[i] = m.align[i];
        }

        /* Read the raw data */
        if (m.nvals>MAX_VARS) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (SILO_Globals.dataReadMask & DBQVData)
        {
            qv->vals = calloc(m.nvals, sizeof(void*));
            if (m.mixlen) qv->mixvals = calloc(m.nvals, sizeof(void*));
            for (i=0; i<m.nvals; i++) {
                qv->vals[i] = db_hdf5_comprd(dbfile, m.value[i], 0);
                if (m.mixlen && m.mixed_value[i][0]) {
                    qv->mixvals[i] = db_hdf5_comprd(dbfile, m.mixed_value[i], 0);
                }
            }
        }
        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeQuadvar(qv);
    } END_PROTECT;

    return qv;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutUcdmesh
 *
 * Purpose:     Write a ucdmesh object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no' and `gnodeno' properties to duplicate
 *              changes made to the PDB driver.
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *   Mark C. Miller, Mon Jul 31 17:43:52 PDT 2006
 *   Removed use of fullname for zonelist, facelist, phzonelist options
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutUcdmesh(DBfile *_dbfile, char *name, int ndims, char *coordnames[],
                   float *coords[], int nnodes, int nzones, char *zlname,
                   char *flname, int datatype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutUcdmesh";
    DBucdmesh_mt        m;
    int                 i, len;

    memset(&_um, 0, sizeof _um);
    memset(&m, 0, sizeof m);

    PROTECT {
        /* Check datatype */
        if (DB_FLOAT!=datatype && DB_DOUBLE!=datatype) {
            db_perror("invalid floating-point datatype", E_BADARGS, me);
            UNWIND();
        }

        /* Set global options */
        strcpy(_um._meshname, name);
        _um._coordsys = DB_OTHER;
        _um._topo_dim = ndims;
        _um._facetype = DB_RECTILINEAR;
        _um._ndims = ndims;
        _um._nnodes = nnodes;
        _um._nzones = nzones;
        _um._planar = DB_OTHER;
        _um._use_specmf = DB_OFF;
        _um._group_no = -1;
        db_ProcessOptlist(DB_UCDMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_um._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_um._time), "time");
        }
        if (_um._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_um._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_um._cycle), "cycle");

        /* Obtain extents as doubles */
        if (DB_DOUBLE==datatype) {
            UM_CalcExtents(coords, datatype, ndims, nnodes,
                           m.min_extents/*out*/, m.max_extents/*out*/);
        } else {
            float min_extents[3], max_extents[3];
            UM_CalcExtents(coords, datatype, ndims, nnodes,
                           min_extents/*out*/, max_extents/*out*/);
            for (i=0; i<ndims; i++) {
                m.min_extents[i] = min_extents[i];
                m.max_extents[i] = max_extents[i];
            }
        }
        
        /* Write variable arrays: coords[], gnodeno[] */
        for (i=0; i<ndims; i++) {
            db_hdf5_compwr(dbfile, datatype, 1, &nnodes, coords[i],
                           m.coord[i]/*out*/);
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &nnodes, _um._gnodeno,
                       m.gnodeno/*out*/);
        
        /* Build ucdmesh header in memory */
        m.ndims = ndims;
        m.nnodes = nnodes;
        m.nzones = nzones;
        m.facetype = _um._facetype;
        m.coord_sys = _um._coordsys;
        m.topo_dim = _um._topo_dim;
        m.planar = _um._planar;
        m.origin = _um._origin;
        m.cycle = _um._cycle;
        m.time = _um._time;
        m.dtime = _um._dtime;
        m.group_no = _um._group_no;
        m.guihide = _um._guihide;
        strcpy(m.zonelist, zlname);
        strcpy(m.facelist, OPT(flname));
        strcpy(m.phzonelist, OPT(_um._phzl_name));
        for (i=0; i<ndims; i++) {
            strcpy(m.label[i], OPT(_um._labels[i]));
            strcpy(m.units[i], OPT(_um._units[i]));
        }

        /* Write ucdmesh header to file */
        STRUCT(DBucdmesh) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nnodes)       MEMBER_S(int, nnodes);
            if (m.nzones)       MEMBER_S(int, nzones);
            if (m.facetype)     MEMBER_S(int, facetype);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.coord_sys)    MEMBER_S(int, coord_sys);
            if (m.topo_dim)     MEMBER_S(int, topo_dim);
            if (m.planar)       MEMBER_S(int, planar);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.group_no)     MEMBER_S(int, group_no);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (_um._time_set)  MEMBER_S(float, time);
            if (_um._dtime_set) MEMBER_S(double, dtime);
            MEMBER_S(str(m.facelist), facelist);
            MEMBER_S(str(m.zonelist), zonelist);
            MEMBER_S(str(m.gnodeno), gnodeno);
            MEMBER_3(double, min_extents);
            MEMBER_3(double, max_extents);
            MEMBER_R(str(m.coord[_j]), coord, ndims);
            MEMBER_R(str(m.label[_j]), label, ndims);
            MEMBER_R(str(m.units[_j]), units, ndims);
            MEMBER_S(str(m.phzonelist), phzonelist);
        } OUTPUT(dbfile, DB_UCDMESH, name, &m);
        
    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutUcdsubmesh
 *
 * Purpose:     Write a subset of a ucdmesh object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *   Mark C. Miller, Mon Jul 31 17:57:29 PDT 2006
 *   Eliminated use of db_hdf5_fullname for zlname, flname and added
 *   possible optional ph_zlname which was mistakenly left out in the
 *   initial implementation.
 *   
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutUcdsubmesh(DBfile *_dbfile, char *name, char *parentmesh,
                      int nzones, char *zlname, char *flname,
                      DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_pdb_PutUcdmesh";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i, len;
    DBucdmesh_mt        m;

    PROTECT {
        /* Get metadata from the parent UCD mesh */
        if ((o=H5Topen(dbfile->cwg, parentmesh))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_UCDMESH!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBucdmesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        H5Tclose(o);

        /* Set global options */
        strcpy(_um._meshname, name);
        _um._coordsys = DB_OTHER;
        _um._topo_dim = m.ndims;
        _um._facetype = DB_RECTILINEAR;
        _um._ndims = m.ndims;
        _um._nnodes = m.nnodes;
        _um._nzones = m.nzones;
        _um._planar = DB_OTHER;
        _um._use_specmf = DB_OFF;
        _um._group_no = -1;
        db_ProcessOptlist(DB_UCDMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_um._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_um._time), "time");
        }
        if (_um._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_um._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_um._cycle), "cycle");

        /* Build header in memory -- most fields are already initialized */
        m.ndims = _um._ndims;
        m.nnodes = _um._nnodes;
        m.nzones = _um._nzones;
        m.facetype = _um._facetype;
        m.cycle = _um._cycle;
        m.coord_sys = _um._coordsys;
        m.topo_dim = _um._topo_dim;
        m.planar = _um._planar;
        m.origin = _um._origin;
        m.time = _um._time;
        m.dtime = _um._dtime;
        m.guihide = _um._guihide;
        strcpy(m.zonelist, zlname);
        strcpy(m.facelist, OPT(flname));
        strcpy(m.phzonelist, OPT(_um._phzl_name));
        for (i=0; i<m.ndims; i++) {
            strcpy(m.label[i], OPT(_um._labels[i]));
            strcpy(m.units[i], OPT(_um._units[i]));
        }
        
        /* Write header to file */
        STRUCT(DBucdmesh) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nnodes)       MEMBER_S(int, nnodes);
            if (m.nzones)       MEMBER_S(int, nzones);
            if (m.facetype)     MEMBER_S(int, facetype);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.coord_sys)    MEMBER_S(int, coord_sys);
            if (m.topo_dim)     MEMBER_S(int, topo_dim);
            if (m.planar)       MEMBER_S(int, planar);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (_um._time_set)  MEMBER_S(float, time);
            if (_um._dtime_set) MEMBER_S(double, dtime);
            MEMBER_S(str(m.facelist), facelist);
            MEMBER_S(str(m.zonelist), zonelist);
            MEMBER_3(double, min_extents);
            MEMBER_3(double, max_extents);
            MEMBER_R(str(m.coord[_j]), coord, m.ndims);
            MEMBER_R(str(m.label[_j]), label, m.ndims);
            MEMBER_R(str(m.units[_j]), units, m.ndims);
        } OUTPUT(dbfile, DB_UCDMESH, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetUcdmesh
 *
 * Purpose:     Reads a UCD mesh object from the file.
 *
 * Return:      Success:        Pointer to a new ucdmesh.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no', and `gnodeno' to duplicate changes made to
 *              the PDB driver.
 *
 *              Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *              Made it set datatype correctly.
 *              Added support for dataReadMask
 *
 *              Mark C. Miller, Thu Sep  7 10:50:55 PDT 2006
 *              Added use of db_hdf5_resolvename for retrieval of
 *              sub-objects.
 *-------------------------------------------------------------------------
 */
CALLBACK DBucdmesh *
db_hdf5_GetUcdmesh(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetUcdmesh";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBucdmesh_mt        m;
    DBucdmesh           *um=NULL;

    PROTECT {
        /* Open object and make sure it's a ucdmesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_UCDMESH!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read header into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBucdmesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a ucdmesh object and initialize meta data */
        if (NULL==(um=DBAllocUcdmesh())) return NULL;
        um->name = BASEDUP(name);
        um->cycle = m.cycle;
        um->coord_sys = m.coord_sys;
        um->topo_dim = m.topo_dim;
        if ((um->datatype = db_hdf5_GetVarType(_dbfile, m.coord[0])) < 0)
            um->datatype = DB_FLOAT;
        if (um->datatype == DB_DOUBLE && force_single_g)
            um->datatype = DB_FLOAT;
        um->time = m.time;
        um->dtime = m.dtime;
        um->ndims = m.ndims;
        um->nnodes = m.nnodes;
        um->origin = m.origin;
        um->group_no = m.group_no;
        um->guihide = m.guihide;
        for (i=0; i<m.ndims; i++) {
            um->units[i] = OPTDUP(m.units[i]);
            um->labels[i] = OPTDUP(m.label[i]);
            if (um->datatype == DB_DOUBLE)
            {
                ((double*)um->min_extents)[i] = m.min_extents[i];
                ((double*)um->max_extents)[i] = m.max_extents[i];
            }
            else
            {
                um->min_extents[i] = m.min_extents[i];
                um->max_extents[i] = m.max_extents[i];
            }
        }

        /* Read the raw data */
        if (SILO_Globals.dataReadMask & DBUMCoords)
        {
            for (i=0; i<m.ndims; i++) {
                um->coords[i] = db_hdf5_comprd(dbfile, m.coord[i], 0);
            }
        }
        if (SILO_Globals.dataReadMask & DBUMGlobNodeNo)
            um->gnodeno = db_hdf5_comprd(dbfile, m.gnodeno, 1);

        /* Read face, zone, and edge lists */
        if (m.facelist[0] && (SILO_Globals.dataReadMask & DBUMFacelist)) {
            um->faces = db_hdf5_GetFacelist(_dbfile,
                            db_hdf5_resolvename(_dbfile, name, m.facelist));
        }
        if (m.zonelist[0] && (SILO_Globals.dataReadMask & DBUMZonelist)) {
            calledFromGetUcdmesh = 1;
            um->zones = db_hdf5_GetZonelist(_dbfile, 
                            db_hdf5_resolvename(_dbfile, name, m.zonelist));
            calledFromGetUcdmesh = 0;

            /*----------------------------------------------------------*/
            /* If we have ghost zones, split any group of shapecnt so   */
            /* all the shapecnt refer to all real zones or all ghost    */
            /* zones.  This will make dealing with ghost zones easier   */
            /* for applications.                                        */
            /*----------------------------------------------------------*/
            if ((um->zones->min_index != 0) || 
                (um->zones->max_index != um->zones->nzones - 1))
            {
                db_SplitShapelist (um);
            }
        }
        if (m.phzonelist[0] && (SILO_Globals.dataReadMask & DBUMZonelist)) {
            um->phzones = db_hdf5_GetPHZonelist(_dbfile, 
                              db_hdf5_resolvename(_dbfile, name, m.phzonelist));
        }
        um->edges = NULL;                               /*FIXME*/

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
    } END_PROTECT;

    return um;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutUcdvar
 *
 * Purpose:     Writes UCD variables to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *   Brad Whitlock, Wed Jan 18 15:17:15 PST 2006 
 *   Added ascii_labels.
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutUcdvar(DBfile *_dbfile, char *name, char *meshname, int nvars,
                  char *varnames[/*nvars*/], float *vars[/*nvars*/],
                  int nels, float *mixvars[/*nvars*/], int mixlen,
                  int datatype, int centering, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutUcdvar";
    DBucdvar_mt         m;
    int                 i, saved_ndims, saved_nnodes, saved_nzones, len;

    memset(&m, 0, sizeof m);

    PROTECT {
        /* Set global options  - based on previous PutUcdmesh() call */
        saved_ndims = _um._ndims;
        saved_nnodes = _um._nnodes;
        saved_nzones = _um._nzones;
        memset(&_um, 0, sizeof _um);
        _um._coordsys = DB_OTHER;
        _um._topo_dim = saved_ndims;
        _um._facetype = DB_RECTILINEAR;
        _um._ndims = saved_ndims;
        _um._nnodes = saved_nnodes;
        _um._nzones = saved_nzones;
        _um._planar = DB_OTHER;
        _um._use_specmf = DB_OFF;
        _um._group_no = -1;
        strcpy(_um._meshname, meshname);
        db_ProcessOptlist(DB_UCDMESH, optlist); /*yes, UCDMESH*/

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_um._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_um._time), "time");
        }
        if (_um._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_um._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_um._cycle), "cycle");

        /* Write variable arrays: vars[], mixvars[] */
        if (nvars>MAX_VARS) {
            db_perror("too many variables", E_BADARGS, me);
            UNWIND();
        }
        for (i=0; i<nvars; i++) {
            db_hdf5_compwr(dbfile, datatype, 1, &nels, vars[i],
                           m.value[i]/*out*/);
            if (mixvars && mixvars[i] && mixlen>0) {
                db_hdf5_compwr(dbfile, datatype, 1, &mixlen, mixvars[i],
                               m.mixed_value[i]/*out*/);
            }
        }
        
        /* Build header in memory */
        m.ndims = _um._ndims;
        m.nvals = nvars;
        m.nels = nels;
        m.centering = centering;
        m.origin = _um._origin;
        m.mixlen = mixlen;
        m.cycle = _um._cycle;
        m.time = _um._time;
        m.dtime = _um._dtime;
        m.lo_offset = _um._lo_offset;
        m.hi_offset = _um._hi_offset;
        m.use_specmf = _um._use_specmf;
        m.ascii_labels = _um._ascii_labels;
        m.guihide = _um._guihide;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;
        strcpy(m.meshid, OPT(_um._meshname));
        strcpy(m.label, OPT(_um._label));
        strcpy(m.units, OPT(_um._unit));

        /* Write header to file */
        STRUCT(DBucdvar) {
            MEMBER_R(str(m.value[_j]), value, nvars);
            MEMBER_R(str(m.mixed_value[_j]), mixed_value, nvars);
            MEMBER_S(str(m.meshid), meshid);
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nvals)        MEMBER_S(int, nvals);
            if (m.nels)         MEMBER_S(int, nels);
            if (m.centering)    MEMBER_S(int, centering);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.mixlen)       MEMBER_S(int, mixlen);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.use_specmf)   MEMBER_S(int, use_specmf);
            if (m.ascii_labels) MEMBER_S(int, ascii_labels);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (_um._time_set)  MEMBER_S(float, time);
            if (_um._dtime_set) MEMBER_S(double, dtime);
            if (_um._lo_offset_set) MEMBER_S(int, lo_offset);
            if (_um._hi_offset_set) MEMBER_S(int, hi_offset);
            MEMBER_S(str(m.label), label);
            MEMBER_S(str(m.units), units);
        } OUTPUT(dbfile, DB_UCDVAR, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetUcdvar
 *
 * Purpose:     Reads a UCD variable object from the file.
 *
 * Return:      Success:        Ptr to new UCD variable object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set the correct datatype. Added support for dataReadMask
 *
 *   Brad Whitlock, Wed Jan 18 15:17:48 PST 2006
 *   Added ascii_labels.
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBucdvar *
db_hdf5_GetUcdvar(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetUcdvar";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBucdvar_mt         m;
    DBucdvar            *uv=NULL;

    PROTECT {
        /* Open object and make sure it's a ucdvar */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_UCDVAR!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read ucdvar data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBucdvar_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a ucdvar object and initialize meta data */
        if (NULL==(uv=DBAllocUcdvar())) return NULL;
        uv->name = BASEDUP(name);
        uv->meshname = OPTDUP(m.meshid);
        uv->cycle = m.cycle;
        uv->units = OPTDUP(m.units);
        uv->label = OPTDUP(m.label);
        uv->time = m.time;
        uv->dtime = m.dtime;
        if ((uv->datatype = db_hdf5_GetVarType(_dbfile, m.value[0])) < 0)
            uv->datatype = silo2silo_type(m.datatype);
        if (uv->datatype == DB_DOUBLE && force_single_g)
            uv->datatype = DB_FLOAT;
        uv->nels = m.nels;
        uv->nvals = m.nvals;
        uv->ndims = m.ndims;
        uv->origin = m.origin;
        uv->centering = m.centering;
        uv->mixlen = m.mixlen;
        uv->use_specmf = m.use_specmf;
        uv->ascii_labels = m.ascii_labels;
        uv->guihide = m.guihide;

        /* Read the raw data */
        if (m.nvals>MAX_VARS) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (SILO_Globals.dataReadMask & DBUVData)
        {
            uv->vals = calloc(m.nvals, sizeof(void*));
            if (m.mixlen) uv->mixvals = calloc(m.nvals, sizeof(void*));
            for (i=0; i<m.nvals; i++) {
                uv->vals[i] = db_hdf5_comprd(dbfile, m.value[i], 0);
                if (m.mixlen && m.mixed_value[i][0]) {
                    uv->mixvals[i] = db_hdf5_comprd(dbfile, m.mixed_value[i], 0);
                }
            }
        }
        H5Tclose(o);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeUcdvar(uv);
    } END_PROTECT;

    return uv;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutFacelist
 *
 * Purpose:     Writes facelist information to a facelist object in the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutFacelist(DBfile *_dbfile, char *name, int nfaces, int ndims,
                    int *nodelist, int lnodelist, int origin, int *zoneno,
                    int *shapesize, int *shapecnt, int nshapes, int *types,
                    int *typelist, int ntypes)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBfacelist_mt       m;

    memset(&m, 0, sizeof m);

    PROTECT {
        /* Write variable arrays */
        if (lnodelist) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &lnodelist, nodelist,
                           m.nodelist/*out*/);
        }
        if (3==ndims) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nshapes, shapecnt,
                           m.shapecnt/*out*/);
            db_hdf5_compwr(dbfile, DB_INT, 1, &nshapes, shapesize,
                           m.shapesize/*out*/);
        }
        if (ntypes && typelist) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &ntypes, typelist,
                           m.typelist/*out*/);
        }
        if (ntypes && types) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nfaces, types,
                           m.types/*out*/);
        }
        if (zoneno) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nfaces, zoneno,
                           m.zoneno/*out*/);
        }
        
        /* Build header in memory */
        m.ndims = ndims;
        m.nfaces = nfaces;
        m.nshapes = nshapes;
        m.ntypes = ntypes;
        m.lnodelist = lnodelist;
        m.origin = origin;

        /* Write header to file */
        STRUCT(DBfacelist) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nfaces)       MEMBER_S(int, nfaces);
            if (m.nshapes)      MEMBER_S(int, nshapes);
            if (m.ntypes)       MEMBER_S(int, ntypes);
            if (m.lnodelist)    MEMBER_S(int, lnodelist);
            if (m.origin)       MEMBER_S(int, origin);
            MEMBER_S(str(m.nodelist), nodelist);
            MEMBER_S(str(m.shapecnt), shapecnt);
            MEMBER_S(str(m.shapesize), shapesize);
            MEMBER_S(str(m.typelist), typelist);
            MEMBER_S(str(m.types), types);
            MEMBER_S(str(m.zoneno), zoneno);
        } OUTPUT(dbfile, DB_FACELIST, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetFacelist
 *
 * Purpose:     Reads a facelist object from the file.
 *
 * Return:      Success:        Ptr to a new facelist object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *  Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *  Added support for dataReadMask
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBfacelist *
db_hdf5_GetFacelist(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetFacelist";
    hid_t               o=-1, attr=-1;
    int                 _objtype;
    DBfacelist_mt       m;
    DBfacelist          *fl=NULL;

    PROTECT {
        /* Open object and make sure it's a facelist */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_FACELIST!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read facelist data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBfacelist_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a facelist object and initialize meta data */
        if (NULL==(fl=DBAllocFacelist())) return NULL;
        fl->ndims = m.ndims;
        fl->nfaces = m.nfaces;
        fl->origin = m.origin;
        fl->lnodelist = m.lnodelist;
        fl->nshapes = m.nshapes;
        fl->ntypes = m.ntypes;

        /* Read the raw data */
        if (SILO_Globals.dataReadMask & DBFacelistInfo)
        {
            fl->nodelist = db_hdf5_comprd(dbfile, m.nodelist, 1);
            fl->shapecnt = db_hdf5_comprd(dbfile, m.shapecnt, 1);
            fl->shapesize = db_hdf5_comprd(dbfile, m.shapesize, 1);
            fl->typelist = db_hdf5_comprd(dbfile, m.typelist, 1);
            fl->types = db_hdf5_comprd(dbfile, m.types, 1);
            fl->zoneno = db_hdf5_comprd(dbfile, m.zoneno, 1);
        }

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeFacelist(fl);
    } END_PROTECT;

    return fl;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutZonelist
 *
 * Purpose:     Writes a zonelist to a file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutZonelist(DBfile *_dbfile, char *name, int nzones, int ndims,
                    int nodelist[], int lnodelist, int origin, int shapesize[],
                    int shapecnt[], int nshapes)
{
    db_hdf5_PutZonelist2(_dbfile, name, nzones, ndims, nodelist, lnodelist,
                         origin, 0, 0, NULL, shapesize, shapecnt, nshapes,
                         NULL);
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutZonelist2
 *
 * Purpose:     Write a ucd zonelist object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Monday, April 12, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added an option list argument to duplicate changes to the
 *              PDB driver. Added gzoneno property.
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutZonelist2(DBfile *_dbfile, char *name, int nzones, int ndims,
                     int nodelist[], int lnodelist, int origin,
                     int lo_offset, int hi_offset, int shapetype[],
                     int shapesize[], int shapecnt[], int nshapes,
                     DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBzonelist_mt       m;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        memset(&_uzl, 0, sizeof _uzl);
        db_ProcessOptlist(DB_ZONELIST, optlist);
        
        /* Write variable arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &lnodelist, nodelist,
                       m.nodelist/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nshapes, shapecnt,
                       m.shapecnt/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nshapes, shapesize,
                       m.shapesize/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nshapes, shapetype,
                       m.shapetype/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nzones, _uzl._gzoneno,
                       m.gzoneno/*out*/);

        /* Build header in memory */
        m.ndims = ndims;
        m.nzones = nzones;
        m.nshapes = nshapes;
        m.lnodelist = lnodelist;
        m.origin = origin;
        m.lo_offset = lo_offset;
        m.hi_offset = hi_offset;

        /* Write header to file */
        STRUCT(DBzonelist) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nzones)       MEMBER_S(int, nzones);
            if (m.nshapes)      MEMBER_S(int, nshapes);
            if (m.lnodelist)    MEMBER_S(int, lnodelist);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.lo_offset)    MEMBER_S(int, lo_offset);
            if (m.hi_offset)    MEMBER_S(int, hi_offset);
            MEMBER_S(str(m.nodelist), nodelist);
            MEMBER_S(str(m.shapecnt), shapecnt);
            MEMBER_S(str(m.shapesize), shapesize);
            MEMBER_S(str(m.shapetype), shapetype);
            MEMBER_S(str(m.gzoneno), gzoneno);
        } OUTPUT(dbfile, DB_ZONELIST, name, &m);
        
    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutPHZonelist
 *
 * Purpose:     Write a DBphzonelist object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Monday, April 12, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added an option list argument to duplicate changes to the
 *              PDB driver. Added gzoneno property.
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutPHZonelist(DBfile *_dbfile, char *name, 
                      int nfaces, int *nodecnt, int lnodelist, int *nodelist,
                      char *extface,
                      int nzones, int *facecnt, int lfacelist, int *facelist,
                      int origin, int lo_offset, int hi_offset,
                      DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBphzonelist_mt       m;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        memset(&_phzl, 0, sizeof _phzl);
        db_ProcessOptlist(DB_PHZONELIST, optlist);
        
        /* Write variable arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &nfaces, nodecnt,
                       m.nodecnt/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &lnodelist, nodelist,
                       m.nodelist/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nfaces, extface,
                       m.extface/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nzones, facecnt,
                       m.facecnt/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &lfacelist, facelist,
                       m.facelist/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nzones, _uzl._gzoneno,
                       m.gzoneno/*out*/);

        /* Build header in memory */
        m.nfaces = nfaces;
        m.lnodelist = lnodelist;
        m.nzones = nzones;
        m.lfacelist = lfacelist;
        m.origin = origin;
        m.lo_offset = lo_offset;
        m.hi_offset = hi_offset;

        /* Write header to file */
        STRUCT(DBphzonelist) {
            if (m.nfaces)       MEMBER_S(int, nfaces);
            if (m.lnodelist)    MEMBER_S(int, lnodelist);
            if (m.nzones)       MEMBER_S(int, nzones);
            if (m.lfacelist)    MEMBER_S(int, lfacelist);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.lo_offset)    MEMBER_S(int, lo_offset);
            if (m.hi_offset)    MEMBER_S(int, hi_offset);
            MEMBER_S(str(m.nodecnt), nodecnt);
            MEMBER_S(str(m.nodelist), nodelist);
            MEMBER_S(str(m.extface), extface);
            MEMBER_S(str(m.facecnt), facecnt);
            MEMBER_S(str(m.facelist), facelist);
            MEMBER_S(str(m.gzoneno), gzoneno);
        } OUTPUT(dbfile, DB_PHZONELIST, name, &m);
        
    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetZonelist
 *
 * Purpose:     Reads a zonelist object from the file
 *
 * Return:      Success:        Ptr to new zonelist
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-14
 *              Added the `gzoneno' property to mirror changes made to the
 *              PDB driver.
 *
 *              Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *              Made it behave identically to PDB driver (for better or
 *              worse) when called from GetUcdmesh. Added support for
 *              dataReadMask
 *-------------------------------------------------------------------------
 */
CALLBACK DBzonelist *
db_hdf5_GetZonelist(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetZonelist";
    hid_t               o=-1, attr=-1;
    int                 _objtype;
    DBzonelist_mt       m;
    DBzonelist          *zl=NULL;

    PROTECT {
        /* Open object and make sure it's a zonelist */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_ZONELIST!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read zonelist data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBzonelist_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a zonelist object and initialize meta data */
        if (NULL==(zl=DBAllocZonelist())) return NULL;
        zl->ndims = m.ndims;
        zl->nzones = m.nzones;
        zl->nshapes = m.nshapes;
        zl->lnodelist = m.lnodelist;
        zl->origin = m.origin;

        /* hack so that HDF5 driver behaves same as PDB driver */
        if (calledFromGetUcdmesh)
        {
            zl->min_index = m.lo_offset;
            zl->max_index = m.nzones - m.hi_offset - 1;
        }
        else
        {
            zl->min_index = 0;
            zl->max_index = 0;
        }

        /* Read the raw data */
        if (SILO_Globals.dataReadMask & DBZonelistInfo)
        {
            zl->shapecnt = db_hdf5_comprd(dbfile, m.shapecnt, 1);
            zl->shapesize = db_hdf5_comprd(dbfile, m.shapesize, 1);
            zl->shapetype = db_hdf5_comprd(dbfile, m.shapetype, 1);
            zl->nodelist = db_hdf5_comprd(dbfile, m.nodelist, 1);
        }
        if (SILO_Globals.dataReadMask & DBZonelistGlobZoneNo)
            zl->gzoneno = db_hdf5_comprd(dbfile, m.gzoneno, 1);

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeZonelist(zl);
    } END_PROTECT;

    return zl;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetPHZonelist
 *
 * Purpose:     Reads a DBphzonelist object from the file
 *
 * Return:      Success:        Ptr to new DBphzonelist
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-14
 *              Added the `gzoneno' property to mirror changes made to the
 *              PDB driver.
 *-------------------------------------------------------------------------
 */
CALLBACK DBphzonelist *
db_hdf5_GetPHZonelist(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetPHZonelist";
    hid_t               o=-1, attr=-1;
    int                 _objtype;
    DBphzonelist_mt       m;
    DBphzonelist          *phzl=NULL;

    PROTECT {
        /* Open object and make sure it's a phzonelist */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_PHZONELIST!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read phzonelist data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBphzonelist_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create a phzonelist object and initialize meta data */
        if (NULL==(phzl=DBAllocPHZonelist())) return NULL;
        phzl->nfaces = m.nfaces;
        phzl->lnodelist = m.lnodelist;
        phzl->nzones = m.nzones;
        phzl->lfacelist = m.lfacelist;
        phzl->origin = m.origin;
        phzl->lo_offset = m.lo_offset;
        phzl->hi_offset = m.hi_offset;

        if (SILO_Globals.dataReadMask & DBZonelistInfo)
        {
            phzl->nodecnt = db_hdf5_comprd(dbfile, m.nodecnt, 1);
            phzl->nodelist = db_hdf5_comprd(dbfile, m.nodelist, 1);
            phzl->extface = db_hdf5_comprd(dbfile, m.extface, 1);
            phzl->facecnt = db_hdf5_comprd(dbfile, m.facecnt, 1);
            phzl->facelist = db_hdf5_comprd(dbfile, m.facelist, 1);
        }
        if (SILO_Globals.dataReadMask & DBZonelistGlobZoneNo)
            phzl->gzoneno = db_hdf5_comprd(dbfile, m.gzoneno, 1);

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreePHZonelist(phzl);
    } END_PROTECT;

    return phzl;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMaterial
 *
 * Purpose:     Write a material object to the file
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   Added support for dataReadMask. Added warning regarding missing
 *   material name functionality
 *
 *   Mark C. Miller, August 9, 2004
 *   Added code to output optional material names
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMaterial(DBfile *_dbfile, char *name, char *mname, int nmat,
                    int matnos[], int matlist[], int dims[], int ndims,
                    int mix_next[], int mix_mat[], int mix_zone[],
                    float mix_vf[], int mixlen, int datatype,
                    DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmaterial_mt       m;
    int                 i, nels;
    char               *s = NULL;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        db_ProcessOptlist(DB_MATERIAL, optlist);
        for (i=0, nels=1; i<ndims; i++) nels *= dims[i];

        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &nels, matlist,
                       m.matlist/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nmat, matnos,
                       m.matnos/*out*/);
        if (mixlen>0) {
            db_hdf5_compwr(dbfile, datatype, 1, &mixlen, mix_vf,
                           m.mix_vf/*out*/);
            db_hdf5_compwr(dbfile, DB_INT, 1, &mixlen, mix_next,
                           m.mix_next/*out*/);
            db_hdf5_compwr(dbfile, DB_INT, 1, &mixlen, mix_mat,
                           m.mix_mat/*out*/);
            db_hdf5_compwr(dbfile, DB_INT, 1, &mixlen, mix_zone,
                           m.mix_zone/*out*/);
        }

        if (_ma._matnames != NULL) {
            int len;
            db_StringArrayToStringList((const char**)_ma._matnames, nmat, &s, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.matnames/*out*/);
            FREE(s);
        }

        if (_ma._matcolors != NULL) {
            int len;
            db_StringArrayToStringList((const char **)_ma._matcolors, nmat, &s, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.matcolors/*out*/);
            FREE(s);
        }
        
        /* Build header in memory */
        m.ndims = ndims;
        m.nmat = nmat;
        m.mixlen = mixlen;
        m.origin = _ma._origin;
        m.major_order = _ma._majororder;
        m.allowmat0 = _ma._allowmat0;
        m.guihide = _ma._guihide;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;
        strcpy(m.meshid, OPT(mname));
        for (nels=1, i=0; i<ndims; i++) {
            m.dims[i] = dims[i];
        }
        
        /* Write header to file */
        STRUCT(DBmaterial) {
            if (m.dims)         MEMBER_S(int, ndims);
            if (m.nmat)         MEMBER_S(int, nmat);
            if (m.mixlen)       MEMBER_S(int, mixlen);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.major_order)  MEMBER_S(int, major_order);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (m.allowmat0)    MEMBER_S(int, allowmat0);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_3(int, dims);
            MEMBER_S(str(m.meshid), meshid);
            MEMBER_S(str(m.matlist), matlist);
            MEMBER_S(str(m.matnos), matnos);
            MEMBER_S(str(m.mix_vf), mix_vf);
            MEMBER_S(str(m.mix_next), mix_next);
            MEMBER_S(str(m.mix_mat), mix_mat);
            MEMBER_S(str(m.mix_zone), mix_zone);
            MEMBER_S(str(m.matnames), matnames);
            MEMBER_S(str(m.matcolors), matcolors);
        } OUTPUT(dbfile, DB_MATERIAL, name, &m);

        FREE(s);

    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMaterial
 *
 * Purpose:     Reads a material object from the file.
 *
 * Return:      Success:        Ptr to new material
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set correct datatype. Added support for dataReadMask
 *
 *   Mark C. Miller, August 9, 2004
 *   Added code to read in optional material names
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBmaterial *
db_hdf5_GetMaterial(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMaterial";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i, nels;
    DBmaterial_mt       m;
    DBmaterial          *ma=NULL;
    char                *s=NULL;
    
    PROTECT {
        /* Open object and make sure it's a material */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MATERIAL!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmaterial_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(ma=DBAllocMaterial())) return NULL;
        ma->name = BASEDUP(name);
        ma->meshname = OPTDUP(m.meshid);
        ma->ndims = m.ndims;
        ma->origin = m.origin;
        ma->major_order = m.major_order;
        ma->allowmat0 = m.allowmat0;
        ma->guihide = m.guihide;
        ma->nmat = m.nmat;
        ma->mixlen = m.mixlen;
        if ((ma->datatype = db_hdf5_GetVarType(_dbfile, m.mix_vf)) < 0)
            ma->datatype = DB_DOUBLE;  /* PDB driver assumes double */
        if (ma->datatype == DB_DOUBLE && force_single_g)
            ma->datatype = DB_FLOAT;
        for (nels=1, i=0; i<m.ndims; i++) {
            ma->dims[i] = m.dims[i];
            ma->stride[i] = nels;
            nels *= m.dims[i];
        }

        /* Read the raw data */
        if (SILO_Globals.dataReadMask & DBMatMatlist)
            ma->matlist = db_hdf5_comprd(dbfile, m.matlist, 1);
        if (SILO_Globals.dataReadMask & DBMatMatnos)
            ma->matnos = db_hdf5_comprd(dbfile, m.matnos, 1);
        if (SILO_Globals.dataReadMask & DBMatMixList)
        {
            ma->mix_vf = db_hdf5_comprd(dbfile, m.mix_vf, 0);
            ma->mix_next = db_hdf5_comprd(dbfile, m.mix_next, 1);
            ma->mix_mat = db_hdf5_comprd(dbfile, m.mix_mat, 1);
            ma->mix_zone = db_hdf5_comprd(dbfile, m.mix_zone, 1);
        }
        if (SILO_Globals.dataReadMask & DBMatMatnames)
        {
            s = db_hdf5_comprd(dbfile, m.matnames, 1);
            if (s) ma->matnames = db_StringListToStringArray(s, ma->nmat);
            FREE(s);
        }
        if (SILO_Globals.dataReadMask & DBMatMatcolors)
        {
            s = db_hdf5_comprd(dbfile, m.matcolors, 1);
            if (s) ma->matcolors = db_StringListToStringArray(s, ma->nmat);
            FREE(s);
        }

        H5Tclose(o);
        FREE(s);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMaterial(ma);
        FREE(s);
    } END_PROTECT;

    return ma;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMatspecies
 *
 * Purpose:     Write a matspecies object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Removed the `origin' property to duplicate changes made to
 *              the PDB driver.
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMatspecies(DBfile *_dbfile, char *name, char *matname, int nmat,
                      int nmatspec[], int speclist[], int dims[], int ndims,
                      int nspecies_mf, float species_mf[], int mix_speclist[],
                      int mixlen, int datatype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmatspecies_mt     m;
    int                 i, nels;
    
    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        db_ProcessOptlist(DB_MATSPECIES, optlist);

        /* Write raw data arrays */
        for (i=0, nels=1; i<ndims; i++) nels *= dims[i];
        db_hdf5_compwr(dbfile, DB_INT, 1, &nels, speclist, m.speclist/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nmat, nmatspec, m.nmatspec/*out*/);
        db_hdf5_compwr(dbfile, datatype, 1, &nspecies_mf, species_mf,
                       m.species_mf/*out*/);
        db_hdf5_compwr(dbfile, DB_INT, 1, &mixlen, mix_speclist,
                       m.mix_speclist/*out*/);
        
        /* Build header in memory */
        m.ndims = ndims;
        m.nmat = nmat;
        m.nspecies_mf = nspecies_mf;
        m.mixlen = mixlen;
        m.major_order = _ms._majororder;
        m.guihide = _ms._guihide;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;
        strcpy(m.matname, OPT(matname));
        for (i=0; i<ndims; i++) m.dims[i] = dims[i];
        
        /* Write header to file */
        STRUCT(DBmatspecies) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nmat)         MEMBER_S(int, nmat);
            if (m.nspecies_mf)  MEMBER_S(int, nspecies_mf);
            if (m.mixlen)       MEMBER_S(int, mixlen);
            if (m.major_order)  MEMBER_S(int, major_order);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_3(int, dims);
            MEMBER_S(str(m.matname), matname);
            MEMBER_S(str(m.speclist), speclist);
            MEMBER_S(str(m.nmatspec), nmatspec);
            MEMBER_S(str(m.species_mf), species_mf);
            MEMBER_S(str(m.mix_speclist), mix_speclist);
        } OUTPUT(dbfile, DB_MATSPECIES, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMatspecies
 *
 * Purpose:     Reads a matspecies object from the file.
 *
 * Return:      Success:        Ptr to new matspecies object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Removed the `origin' property, duplicating changes made to
 *              the PDB driver.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Made it set correct datatype.
 *-------------------------------------------------------------------------
 */
CALLBACK DBmatspecies *
db_hdf5_GetMatspecies(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMatspecies";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i, nels;
    DBmatspecies_mt     m;
    DBmatspecies        *ms=NULL;

    PROTECT {
        /* Open object and make sure it's a matspecies */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MATSPECIES!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmatspecies_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(ms=DBAllocMatspecies())) return NULL;
        ms->name = BASEDUP(name);
        ms->matname = OPTDUP(m.matname);
        ms->nmat = m.nmat;
        ms->ndims = m.ndims;
        ms->guihide = m.guihide;
        ms->major_order = m.major_order;
        ms->nspecies_mf = m.nspecies_mf;
        ms->mixlen = m.mixlen;
        if ((ms->datatype = db_hdf5_GetVarType(_dbfile, m.species_mf)) < 0)
            ms->datatype = silo2silo_type(m.datatype);
        if (ms->datatype == DB_DOUBLE && force_single_g)
            ms->datatype = DB_FLOAT;
        for (i=0, nels=1; i<m.ndims; i++) {
            ms->dims[i] = m.dims[i];
            ms->stride[i] = nels;
            nels *= m.dims[i];
        }

        /* Read the raw data */
        ms->nmatspec = db_hdf5_comprd(dbfile, m.nmatspec, 1);
        ms->species_mf = db_hdf5_comprd(dbfile, m.species_mf, 0);
        ms->speclist = db_hdf5_comprd(dbfile, m.speclist, 1);
        ms->mix_speclist = db_hdf5_comprd(dbfile, m.mix_speclist, 1);

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMatspecies(ms);
    } END_PROTECT;
    return ms;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMultimesh
 *
 * Purpose:     Write a multi-block mesh object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Eric Brugger, 2004-03-12
 *              Split the declaration and initialization of sizes
 *              into multiple statements so that it compiles on old
 *              sgi compilers.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Added call to reset global data
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMultimesh(DBfile *_dbfile, char *name, int nmesh,
                     char *meshnames[], int meshtypes[], DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmultimesh_mt      m;
    int                 i, len;
    char                *s=NULL;
    char                *t=NULL;
    
    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        db_ResetGlobalData_MultiMesh();
        db_ProcessOptlist(DB_MULTIMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_mm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_mm._time), "time");
        }
        if (_mm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_mm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_mm._cycle), "cycle");

        /*
         * Create a character string which is a semi-colon separated list of
         * mesh names.
         */
        for (i=len=0; i<nmesh; i++) len += strlen(meshnames[i])+1;
        s = malloc(len+1);
        for (i=len=0; i<nmesh; i++) {
            if (i) s[len++] = ';';
            strcpy(s+len, meshnames[i]);
            len += strlen(meshnames[i]);
        }
        len++; /*count null*/
        
        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &nmesh, meshtypes,
                       m.meshtypes/*out*/);
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s,
                       m.meshnames/*out*/);
        if (_mm._extents && _mm._extentssize) {
            int sizes[2];
            sizes[0] = nmesh;
            sizes[1] = _mm._extentssize;
            db_hdf5_compwr(dbfile, DB_DOUBLE, 2, sizes, _mm._extents,
                           m.extents/*out*/);
        }
        if (_mm._zonecounts) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nmesh, _mm._zonecounts,
                           m.zonecounts/*out*/);
        }
        if (_mm._has_external_zones) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nmesh, _mm._has_external_zones,
                           m.has_external_zones/*out*/);
        }
        if (_mm._lgroupings > 0 && _mm._groupings != NULL) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &_mm._lgroupings, _mm._groupings,
                           m.groupings/*out*/);
        }
        if (_mm._lgroupings > 0 && _mm._groupnames != NULL) {
           db_StringArrayToStringList((const char **)_mm._groupnames, 
                           _mm._lgroupings, &t, &len);
           db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, t,
                           m.groupnames/*out*/);
           FREE(t);
        }
        
        /* Initialize meta data */
        m.nblocks = nmesh;
        m.cycle = _mm._cycle;
        m.time = _mm._time;
        m.dtime = _mm._dtime;
        m.ngroups = _mm._ngroups;
        m.blockorigin = _mm._blockorigin;
        m.grouporigin = _mm._grouporigin;
        m.extentssize = _mm._extentssize;
        m.guihide = _mm._guihide;
        m.lgroupings = _mm._lgroupings;

        /* Write meta data to file */
        STRUCT(DBmultimesh) {
            if (m.nblocks)      MEMBER_S(int, nblocks);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.ngroups)      MEMBER_S(int, ngroups);
            if (m.blockorigin)  MEMBER_S(int, blockorigin);
            if (m.grouporigin)  MEMBER_S(int, grouporigin);
            if (_mm._time_set)  MEMBER_S(float, time);
            if (_mm._dtime_set) MEMBER_S(double, dtime);
            if (m.extentssize)  MEMBER_S(int, extentssize);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_S(str(m.meshtypes), meshtypes);
            MEMBER_S(str(m.meshnames), meshnames);
            MEMBER_S(str(m.extents), extents);
            MEMBER_S(str(m.zonecounts), zonecounts);
            MEMBER_S(str(m.has_external_zones), has_external_zones);
            if (m.lgroupings)   MEMBER_S(int, lgroupings);
            MEMBER_S(str(m.groupings), groupings);
            MEMBER_S(str(m.groupnames), groupnames);
        } OUTPUT(dbfile, DB_MULTIMESH, name, &m);

        /* Free resources */
        FREE(s);
        
    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMultimesh
 *
 * Purpose:     Reads a multimesh object from the file.
 *
 * Return:      Success:        Ptr to new multimesh object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Mark C. Miller, Wed Feb  2 07:52:22 PST 2005
 *              Added code to temporarily disable force single when
 *              reading extents
 *
 *   Mark C. Miller, Tue Feb 15 14:53:29 PST 2005
 *   Changed how force_single was handled to deal with possible throw
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultimesh *
db_hdf5_GetMultimesh(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMultimesh";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBmultimesh_mt      m;
    DBmultimesh         *mm=NULL;
    char                *s=NULL;
    char                *t=NULL;

    PROTECT {
        /* Open object and make sure it's a multimesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MULTIMESH!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmultimesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(mm=DBAllocMultimesh(0))) return NULL;
        mm->nblocks = m.nblocks;
        mm->ngroups = m.ngroups;
        mm->blockorigin = m.blockorigin;
        mm->grouporigin = m.grouporigin;
        mm->extentssize = m.extentssize;
        mm->guihide = m.guihide;
        mm->lgroupings = m.lgroupings;

        /* Read the raw data */
        if (mm->extentssize>0)
           mm->extents = db_hdf5_comprd(dbfile, m.extents, 1);
        mm->zonecounts =  db_hdf5_comprd(dbfile, m.zonecounts, 1);
        mm->has_external_zones =  db_hdf5_comprd(dbfile, m.has_external_zones, 1);
        mm->meshtypes = db_hdf5_comprd(dbfile, m.meshtypes, 1);
        mm->meshnames = calloc(m.nblocks, sizeof(char*));
        s = db_hdf5_comprd(dbfile, m.meshnames, 1);
        for (i=0; i<m.nblocks; i++) {
            char *tok = strtok(i?NULL:s, ";");
            mm->meshnames[i] = STRDUP(tok);
        }
        mm->groupings =  db_hdf5_comprd(dbfile, m.groupings, 1);
        t = db_hdf5_comprd(dbfile, m.groupnames, 1);
        if (t) mm->groupnames = db_StringListToStringArray(t, mm->lgroupings);
        FREE(t);
        
        H5Tclose(o);
        FREE(s);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMultimesh(mm);
        FREE(t);
        FREE(s);
    } END_PROTECT;
    return mm;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMultimeshadj
 *
 * Purpose:     Write a multi-block mesh adjacency object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Mark C. Miller 
 *              Thursday, September 8, 2005 
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMultimeshadj(DBfile *_dbfile, const char *name, int nmesh,
                  const int *meshtypes, const int *nneighbors,
                  const int *neighbors, const int *back,
                  const int *lnodelists, const int *nodelists[],
                  const int *lzonelists, const int *zonelists[],
                  DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmultimeshadj_mt   m;
    int                 i, len, lneighbors, zoff, noff;
    char                *s=NULL;
    hid_t               o=-1, attr=-1, nldset=-1, zldset=-1;
    hid_t               mtype=-1, fspace=-1, mspace=-1;
    static char         *me = "db_hdf5_PutMultimeshadj";
    int                 _objtype;
    
    memset(&m, 0, sizeof m);

    /* compute expected size of neighbors array */
    lneighbors = 0;
    for (i = 0; i < nmesh; i++)
       lneighbors += nneighbors[i];

    PROTECT {

       H5E_BEGIN_TRY {
           o = H5Topen(dbfile->cwg, name);
       } H5E_END_TRY;

       if (o >= 0)
       {

            /* Object exists, do some simple sanity checking */
            if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
                H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
                H5Aclose(attr)<0) {
                db_perror((char*)name, E_CALLFAIL, me);
                UNWIND();
            }
            if (DB_MULTIMESHADJ!=(DBObjectType)_objtype) {
                db_perror("not a DBmultimeshadj object", E_BADARGS, me);
                UNWIND();
            }

            /* Read meta data into memory */
            if ((attr=H5Aopen_name(o, "silo"))<0 ||
                H5Aread(attr, DBmultimeshadj_mt5, &m)<0 ||
                H5Aclose(attr)<0) {
                db_perror((char*)name, E_CALLFAIL, me);
                UNWIND();
            }

            /* should add correct sanity checks here */

       }
       else
       {

           /* Object doesn't exist, allocate space in the file for the datasets */
           db_ResetGlobalData_MultiMesh();
           db_ProcessOptlist(DB_MULTIMESH, optlist);

           /* Initialize meta data */
           m.nblocks = nmesh;
           m.blockorigin = _mm._blockorigin;
           m.lneighbors = lneighbors;

           /* compute length of neighbors, back, lnodelists, nodelists,
              lzonelists, zonelists arrays */
           lneighbors = 0;
           for (i = 0; i < nmesh; i++)
               lneighbors += nneighbors[i];

           db_hdf5_compwr(dbfile, DB_INT, 1, &nmesh, (void*)meshtypes,
               m.meshtypes/*out*/);
           db_hdf5_compwr(dbfile, DB_INT, 1, &nmesh, (void*)nneighbors,
               m.nneighbors/*out*/);
           db_hdf5_compwr(dbfile, DB_INT, 1, &lneighbors, (void*)neighbors,
               m.neighbors/*out*/);
           if (back)
           {
               db_hdf5_compwr(dbfile, DB_INT, 1, &lneighbors, (void*)back,
                   m.back/*out*/);
           }
           if (lnodelists)
           {
               db_hdf5_compwr(dbfile, DB_INT, 1, &lneighbors, (void*)lnodelists,
                   m.lnodelists/*out*/);
           }
           if (lzonelists)
           {
               db_hdf5_compwr(dbfile, DB_INT, 1, &lneighbors, (void*)lzonelists,
                   m.lzonelists/*out*/);
           }

           /* All object components up to here are invariant and *should*
              be identical in repeated calls. Now, handle the parts of the
              object that can vary from call to call. Reserve space for
              the entire nodelists and/or zonelists arrays */

           if (nodelists) {

               /* compute total length of nodelists array */
               len = 0;
               for (i = 0; i < lneighbors; i++)
                   len += lnodelists[i];
               m.totlnodelists = len;

               /* reserve space for the nodelists array in the file */
               /* negative rank means to reserve space */
               if (db_hdf5_compwr(dbfile, DB_INT, -1, &len, NULL,
                                  m.nodelists/*out*/)<0) {
                  return db_perror ("db_hdf5_compwr", E_CALLFAIL, me) ;
               }
           }

           if (zonelists) {

               /* compute total length of nodelists array */
               len = 0;
               for (i = 0; i < lneighbors; i++)
                   len += lzonelists[i];
               m.totlzonelists = len;

               /* reserve space for the zonelists array in the file */
               /* negative rank means to reserve space */
               if (db_hdf5_compwr(dbfile, DB_INT, -1, &len, NULL,
                                  m.zonelists/*out*/)<0) {
                  return db_perror ("db_hdf5_compwr", E_CALLFAIL, me) ;
               }
           }

           /* hack to maintain backward compatibility with pdb driver */
           len = 1;
           if (_mm._time_set == TRUE) {
               db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_mm._time), "time");
           }
           if (_mm._dtime_set == TRUE) {
               db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_mm._dtime), "dtime");
           }
           db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_mm._cycle), "cycle");

           /* Write meta data to file */
           STRUCT(DBmultimeshadj) {
               MEMBER_S(int, nblocks);
               MEMBER_S(int, blockorigin);
               MEMBER_S(int, lneighbors);
               if (m.totlnodelists) MEMBER_S(int, totlnodelists);
               if (m.totlzonelists) MEMBER_S(int, totlzonelists);
               MEMBER_S(str(m.meshtypes), meshtypes);
               MEMBER_S(str(m.nneighbors), nneighbors);
               MEMBER_S(str(m.neighbors), neighbors);
               if (m.back[0]) MEMBER_S(str(m.back), back);
               if (m.lnodelists[0]) MEMBER_S(str(m.lnodelists), lnodelists);
               if (m.nodelists[0]) MEMBER_S(str(m.nodelists), nodelists);
               if (m.lzonelists[0]) MEMBER_S(str(m.lzonelists), lzonelists);
               if (m.zonelists[0]) MEMBER_S(str(m.zonelists), zonelists);
           } OUTPUT(dbfile, DB_MULTIMESHADJ, name, &m);
       }

       if (m.nodelists[0] && 
           (nldset = H5Dopen(dbfile->cwg, m.nodelists)) < 0) {
           db_perror((char*)name, E_CALLFAIL, me);
           UNWIND();
       }

       if (m.zonelists[0] &&
           (zldset = H5Dopen(dbfile->cwg, m.zonelists)) < 0) {
           db_perror((char*)name, E_CALLFAIL, me);
           UNWIND();
       }

       if ((mtype=silom2hdfm_type(DB_INT))<0) {
           db_perror("datatype", E_BADARGS, me);
           UNWIND();
       }

       /* Ok, now write contents of nodelists and/or zonelists */
       noff = 0;
       zoff = 0;
       for (i = 0; i < lneighbors; i++)
       {
          hsize_t ds_size[H5S_MAX_RANK];

          if (nodelists)
          {
             if (nodelists[i])
             {
                int offset = noff;
                int length = lnodelists[i];
                int stride = 1;

                /* Build the file space selection */
                if ((fspace=build_fspace(nldset, 1, &offset, &length, &stride,
                                        ds_size/*out*/))<0) {
                   db_perror("file data space", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Build the memory data space */
               if ((mspace=H5Screate_simple(1, ds_size, NULL))<0) {
                   db_perror("memory data space", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Write data */
               if (H5Dwrite(nldset, mtype, mspace, fspace, H5P_DEFAULT, nodelists[i])<0) {
                   db_perror("partial write", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Close everything */
               H5Sclose(fspace);
               H5Sclose(mspace);
             }
             noff += lnodelists[i];
          }

          if (zonelists)
          {
             if (zonelists[i])
             {
                int offset = zoff;
                int length = lzonelists[i];
                int stride = 1;

               /* Build the file space selection */
               if ((fspace=build_fspace(zldset, 1, &offset, &length, &stride,
                                        ds_size/*out*/))<0) {
                   db_perror("file data space", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Build the memory data space */
               if ((mspace=H5Screate_simple(1, ds_size, NULL))<0) {
                   db_perror("memory data space", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Write data */
               if (H5Dwrite(zldset, mtype, mspace, fspace, H5P_DEFAULT, nodelists[i])<0) {
                   db_perror("partial write", E_CALLFAIL, me);
                   UNWIND();
               }

               /* Close everything */
               H5Sclose(fspace);
               H5Sclose(mspace);

             }
             zoff += lzonelists[i];
          }
       }

       if (nldset != -1)
           H5Dclose(nldset);
       if (zldset != -1)
           H5Dclose(zldset);

    } CLEANUP {
    } END_PROTECT;
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMultimeshadj
 *
 * Purpose:     Reads a multimesh adjacency object from the file.
 *
 * Return:      Success:        Ptr to new multimesh adjacency object
 *
 *              Failure:        NULL
 *
 * Programmer:  Mark C. Miller 
 *              Thursday, September 8, 2005 
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultimeshadj *
db_hdf5_GetMultimeshadj(DBfile *_dbfile, const char *name, int nmesh,
                        const int *block_map)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMultimesh";
    hid_t               o=-1, attr=-1, nldset = -1, zldset = -1;
    hid_t               mtype=-1, fspace=-1, mspace=-1;
    DBmultimeshadj_mt   m;
    DBmultimeshadj      *mmadj=NULL;
    char                *typestring = NULL;
    int                 i, j, tmpnmesh, _objtype;
    int                 *offsetmap, *offsetmapn=0, *offsetmapz=0, lneighbors, tmpoff;

    PROTECT {
        /* Open object and make sure it's a multimesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror((char*)name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MULTIMESHADJ!=(DBObjectType)_objtype) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmultimeshadj_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(mmadj=DBAllocMultimeshadj(0))) return NULL;
        mmadj->nblocks = m.nblocks;
        mmadj->blockorigin = m.blockorigin;
        mmadj->lneighbors = m.lneighbors;

        /* Read the raw data */
        mmadj->meshtypes = db_hdf5_comprd(dbfile, m.meshtypes, 1);
        mmadj->nneighbors = db_hdf5_comprd(dbfile, m.nneighbors, 1);
        mmadj->neighbors = db_hdf5_comprd(dbfile, m.neighbors, 1);
        mmadj->back = db_hdf5_comprd(dbfile, m.back, 1);
        mmadj->lnodelists = db_hdf5_comprd(dbfile, m.lnodelists, 1);
        mmadj->lzonelists = db_hdf5_comprd(dbfile, m.lzonelists, 1);

        offsetmap = ALLOC_N(int, mmadj->nblocks);
        lneighbors = 0;
        for (i = 0; i < mmadj->nblocks; i++)
        {
            offsetmap[i] = lneighbors;
            lneighbors += mmadj->nneighbors[i];
        }
 
        if (mmadj->lnodelists && (SILO_Globals.dataReadMask & DBMMADJNodelists))
        {
           mmadj->nodelists = ALLOC_N(int *, lneighbors); 
           offsetmapn = ALLOC_N(int, mmadj->nblocks);
           tmpoff = 0;
           for (i = 0; i < mmadj->nblocks; i++)
           {
               offsetmapn[i] = tmpoff;
               for (j = 0; j < mmadj->nneighbors[i]; j++)
                  tmpoff += mmadj->lnodelists[offsetmap[i]+j];
           }
           mmadj->totlnodelists = m.totlnodelists;
        }
 
        if (mmadj->lzonelists && (SILO_Globals.dataReadMask & DBMMADJZonelists))
        {
           mmadj->zonelists = ALLOC_N(int *, lneighbors); 
           offsetmapz = ALLOC_N(int, mmadj->nblocks);
           tmpoff = 0;
           for (i = 0; i < mmadj->nblocks; i++)
           {
               offsetmapz[i] = tmpoff;
               for (j = 0; j < mmadj->nneighbors[i]; j++)
                  tmpoff += mmadj->lzonelists[offsetmap[i]+j];
           }
           mmadj->totlzonelists = m.totlzonelists;
        }
        
        tmpnmesh = nmesh;
        if (nmesh <= 0 || !block_map)
            tmpnmesh = mmadj->nblocks;
 
        if (m.nodelists[0] && 
            (nldset = H5Dopen(dbfile->cwg, m.nodelists)) < 0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        if (m.zonelists[0] &&
            (zldset = H5Dopen(dbfile->cwg, m.zonelists)) < 0) {
            db_perror((char*)name, E_CALLFAIL, me);
            UNWIND();
        }

        if ((mtype=silom2hdfm_type(DB_INT))<0) {
            FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
            DBFreeMultimeshadj(mmadj);
            db_perror("datatype", E_BADARGS, me);
            UNWIND();
        }

        /* This loop could be optimized w.r.t. number of I/O requests
           it makes. The nodelists and/or zonelists could be read in
           a single call. But then we'd have to split it into separate
           arrays duplicating memory */
        for (i = 0; (i < tmpnmesh) &&
                    (SILO_Globals.dataReadMask & (DBMMADJNodelists|DBMMADJZonelists)); i++)
        {
           hsize_t ds_size[H5S_MAX_RANK];
           int blockno = block_map ? block_map[i] : i;
 
           if (mmadj->lnodelists && (SILO_Globals.dataReadMask & DBMMADJNodelists))
           {
              tmpoff = offsetmapn[blockno];
              for (j = 0; j < mmadj->nneighbors[blockno]; j++)
              {
                 int stride = 1;
                 int len = mmadj->lnodelists[offsetmap[blockno]+j];
                 int *nlist = ALLOC_N(int, len);

                 /* Build the file space selection */
                 if ((fspace=build_fspace(nldset, 1, &tmpoff, &len, &stride,
                                          ds_size/*out*/))<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     db_perror("file data space", E_CALLFAIL, me);
                     UNWIND();
                 }

                 /* Build the memory data space */
                 if ((mspace=H5Screate_simple(1, ds_size, NULL))<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     db_perror("memory data space", E_CALLFAIL, me);
                     UNWIND();
                 }

                 P_rdprops = H5P_DEFAULT;
                 if (!SILO_Globals.enableChecksums)
                     P_rdprops = P_ckrdprops;

                 /* Read data */
                 if (H5Dread(nldset, mtype, mspace, fspace, P_rdprops, nlist)<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     hdf5_to_silo_error(name, me);
                     UNWIND();
                 }

                 /* Close everything */
                 H5Sclose(fspace);
                 H5Sclose(mspace);
 
                 mmadj->nodelists[offsetmap[blockno]+j] = nlist;
                 tmpoff += len;
              }
           }
 
           if (mmadj->lzonelists && (SILO_Globals.dataReadMask & DBMMADJZonelists))
           {
              tmpoff = offsetmapz[blockno];
              for (j = 0; j < mmadj->nneighbors[blockno]; j++)
              {
                 int stride = 1;
                 int len = mmadj->lzonelists[offsetmap[blockno]+j];
                 int *zlist = ALLOC_N(int, len);

                 /* Build the file space selection */
                 if ((fspace=build_fspace(zldset, 1, &tmpoff, &len, &stride,
                                          ds_size/*out*/))<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     db_perror("file data space", E_CALLFAIL, me);
                     UNWIND();
                 }

                 /* Build the memory data space */
                 if ((mspace=H5Screate_simple(1, ds_size, NULL))<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     db_perror("memory data space", E_CALLFAIL, me);
                     UNWIND();
                 }

                 P_rdprops = H5P_DEFAULT;
                 if (!SILO_Globals.enableChecksums)
                     P_rdprops = P_ckrdprops;

                 /* Read data */
                 if (H5Dread(zldset, mtype, mspace, fspace, P_rdprops, zlist)<0) {
                     FREE(offsetmap); FREE(offsetmapn); FREE(offsetmapz);
                     DBFreeMultimeshadj(mmadj);
                     hdf5_to_silo_error(name, me);
                     UNWIND();
                 }

                 /* Close everything */
                 H5Sclose(fspace);
                 H5Sclose(mspace);
 
                 mmadj->zonelists[offsetmap[blockno]+j] = zlist;
                 tmpoff += len;
              }
           }
        }
 
        FREE(offsetmap);
        FREE(offsetmapn);
        FREE(offsetmapz);
        if (nldset != -1)
            H5Dclose(nldset);
        if (zldset != -1)
            H5Dclose(zldset);
        H5Tclose(o);

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMultimeshadj(mmadj);
    } END_PROTECT;

    return mmadj;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMultivar
 *
 * Purpose:     Writes a multivar object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Eric Brugger, 2004-03-12
 *              Split the declaration and initialization of sizes
 *              into multiple statements so that it compiles on old
 *              sgi compilers.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Added call to reset global data
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMultivar(DBfile *_dbfile, char *name, int nvars, char *varnames[],
                    int vartypes[], DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmultivar_mt       m;
    int                 i, len;
    char                *s=NULL;

    memset(&m, 0, sizeof m);
    PROTECT {

        /* Set global options */
        db_ResetGlobalData_MultiMesh();
        db_ProcessOptlist(DB_MULTIMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_mm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_mm._time), "time");
        }
        if (_mm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_mm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_mm._cycle), "cycle");

        /*
         * Create a character string which is a semi-colon separated list of
         * variable names.
         */
        for (i=len=0; i<nvars; i++) len += strlen(varnames[i])+1;
        s = malloc(len+1);
        for (i=len=0; i<nvars; i++) {
            if (i) s[len++] = ';';
            strcpy(s+len, varnames[i]);
            len += strlen(varnames[i]);
        }
        len++; /*count null*/
        
        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, DB_INT, 1, &nvars, vartypes,
                       m.vartypes/*out*/);
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s,
                       m.varnames/*out*/);
        if (_mm._extents && _mm._extentssize) {
            int sizes[2];
            sizes[0] = nvars;
            sizes[1] = _mm._extentssize;
            db_hdf5_compwr(dbfile, DB_DOUBLE, 2, sizes, _mm._extents,
                           m.extents/*out*/);
        }
        
        /* Initialize meta data */
        m.nvars = nvars;
        m.cycle = _mm._cycle;
        m.time = _mm._time;
        m.dtime = _mm._dtime;
        m.ngroups = _mm._ngroups;
        m.blockorigin = _mm._blockorigin;
        m.grouporigin = _mm._grouporigin;
        m.extentssize = _mm._extentssize;
        m.guihide = _mm._guihide;

        /* Write meta data to file */
        STRUCT(DBmultivar) {
            if (m.nvars)        MEMBER_S(int, nvars);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.ngroups)      MEMBER_S(int, ngroups);
            if (m.blockorigin)  MEMBER_S(int, blockorigin);
            if (m.grouporigin)  MEMBER_S(int, grouporigin);
            if (_mm._time_set)  MEMBER_S(float, time);
            if (_mm._dtime_set) MEMBER_S(double, dtime);
            if (m.extentssize)  MEMBER_S(int, extentssize);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_S(str(m.vartypes), vartypes);
            MEMBER_S(str(m.varnames), varnames);
            MEMBER_S(str(m.extents), extents);
        } OUTPUT(dbfile, DB_MULTIVAR, name, &m);

        /* Free resources */
        FREE(s);
        
    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMultivar
 *
 * Purpose:     Reads a multivar object from the file.
 *
 * Return:      Success:        Ptr to new multivar object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Removed erroneous code setting vartypes
 *
 *              Mark C. Miller, Wed Feb  2 07:52:22 PST 2005
 *              Added code to temporarily disable force single when
 *              reading extents
 *
 *   Mark C. Miller, Tue Feb 15 14:53:29 PST 2005
 *   Changed how force_single was handled to deal with possible throw
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultivar *
db_hdf5_GetMultivar(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMultivar";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBmultivar_mt       m;
    DBmultivar          *mv=NULL;
    char                *s=NULL;

    PROTECT {
        /* Open object and make sure it's a multivar */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MULTIVAR!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmultivar_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(mv=DBAllocMultivar(0))) return NULL;
        mv->nvars = m.nvars;
        mv->ngroups = m.ngroups;
        mv->blockorigin = m.blockorigin;
        mv->grouporigin = m.grouporigin;
        mv->extentssize = m.extentssize;
        mv->guihide = m.guihide;

        /* Read the raw data variable types and convert to mem types*/
        if (mv->extentssize>0)
           mv->extents = db_hdf5_comprd(dbfile, m.extents, 1);
        mv->vartypes = db_hdf5_comprd(dbfile, m.vartypes, 1);

        /* Read the raw data variable names */
        mv->varnames = calloc(m.nvars, sizeof(char*));
        s = db_hdf5_comprd(dbfile, m.varnames, 1);
        for (i=0; i<m.nvars; i++) {
            char *tok = strtok(i?NULL:s, ";");
            mv->varnames[i] = STRDUP(tok);
        }

        
        H5Tclose(o);
        FREE(s);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMultivar(mv);
        FREE(s);
    } END_PROTECT;
    return mv;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMultimat
 *
 * Purpose:     Write a multimat object into the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Added call to reset global data
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *   Mark C. Miller, Mon Aug  7 17:03:51 PDT 2006
 *   Added material names and matcolors options
 *
 *   Thoamas R. Treadway, Tue Aug 15 14:05:59 PDT 2006
 *   Added DBOPT_ALLOWMAT0
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMultimat(DBfile *_dbfile, char *name, int nmats, char *matnames[],
                    DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmultimat_mt       m;
    int                 i, len;
    char                *s=NULL;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        db_ResetGlobalData_MultiMesh();
        db_ProcessOptlist(DB_MULTIMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_mm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_mm._time), "time");
        }
        if (_mm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_mm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_mm._cycle), "cycle");

        /*
         * Create a character string which is a semi-colon separated list of
         * material names.
         */
        for (i=len=0; i<nmats; i++) len += strlen(matnames[i])+1;
        s = malloc(len+1);
        for (i=len=0; i<nmats; i++) {
            if (i) s[len++] = ';';
            strcpy(s+len, matnames[i]);
            len += strlen(matnames[i]);
        }
        len++; /*count null*/

        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.matnames/*out*/);
        if (_mm._matnos && _mm._nmatnos > 0) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &_mm._nmatnos, _mm._matnos,
                           m.matnos/*out*/);
        }
        if (_mm._mixlens) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nmats, _mm._mixlens,
                           m.mixlens/*out*/);
        }
        if (_mm._matcounts && _mm._matlists) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &nmats, _mm._matcounts,
                           m.matcounts/*out*/);
            for (i=len=0; i<nmats; i++)
               len += _mm._matcounts[i];
            db_hdf5_compwr(dbfile, DB_INT, 1, &len, _mm._matlists,
                           m.matlists/*out*/);
        }
        if (_mm._matcolors && _mm._nmatnos > 0) {
            int len; char *tmp;
            db_StringArrayToStringList((const char**) _mm._matcolors,
                _mm._nmatnos, &tmp, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, tmp,
                           m.mat_colors/*out*/);
            FREE(tmp);
        }
        if (_mm._matnames && _mm._nmatnos > 0) {
            int len; char *tmp;
            db_StringArrayToStringList((const char**) _mm._matnames,
                _mm._nmatnos, &tmp, &len);
            db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, tmp,
                           m.material_names/*out*/);
            FREE(tmp);
        }

        /* Initialize meta data */
        m.nmats = nmats;
        m.cycle = _mm._cycle;
        m.time = _mm._time;
        m.dtime = _mm._dtime;
        m.ngroups = _mm._ngroups;
        m.blockorigin = _mm._blockorigin;
        m.grouporigin = _mm._grouporigin;
        m.nmatnos = _mm._nmatnos;
        m.allowmat0 = _mm._allowmat0;
        m.guihide = _mm._guihide;

        /* Write meta data to file */
        STRUCT(DBmultimat) {
            if (m.nmats)        MEMBER_S(int, nmats);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.ngroups)      MEMBER_S(int, ngroups);
            if (m.blockorigin)  MEMBER_S(int, blockorigin);
            if (m.grouporigin)  MEMBER_S(int, grouporigin);
            if (_mm._time_set)  MEMBER_S(float, time);
            if (_mm._dtime_set) MEMBER_S(double, dtime);
            MEMBER_S(str(m.matnames), matnames);
            MEMBER_S(str(m.matnos), matnos);
            MEMBER_S(str(m.mixlens), mixlens);
            MEMBER_S(str(m.matcounts), matcounts);
            MEMBER_S(str(m.matlists), matlists);
            if (m.nmatnos)      MEMBER_S(int, nmatnos);
            if (m.allowmat0)    MEMBER_S(int, allowmat0);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_S(str(m.material_names), material_names);
            MEMBER_S(str(m.mat_colors), mat_colors);
        } OUTPUT(dbfile, DB_MULTIMAT, name, &m);

        /* Free resources */
        FREE(s);
        
    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMultimat
 *
 * Purpose:     Reads a multimat object from the file.
 *
 * Return:      Success:        Ptr to new multimat object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *    Mark C. Miller, Mon Aug  7 17:03:51 PDT 2006
 *    Added material names and material colors options as well as nmatnos
 *    and matnos
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultimat *
db_hdf5_GetMultimat(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMultimat";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBmultimat_mt       m;
    DBmultimat          *mm=NULL;
    char                *s=NULL;

    PROTECT {
        /* Open object and make sure it's a multimat */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MULTIMAT!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmultimat_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(mm=DBAllocMultimat(0))) return NULL;
        mm->nmats = m.nmats;
        mm->ngroups = m.ngroups;
        mm->blockorigin = m.blockorigin;
        mm->grouporigin = m.grouporigin;
        mm->allowmat0 = m.allowmat0;
        mm->guihide = m.guihide;
        mm->nmatnos = m.nmatnos;

        /* Read the raw data */
        mm->mixlens = db_hdf5_comprd(dbfile, m.mixlens, 1);
        mm->matcounts = db_hdf5_comprd(dbfile, m.matcounts, 1);
        mm->matlists = db_hdf5_comprd(dbfile, m.matlists, 1);
        mm->matnos = db_hdf5_comprd(dbfile, m.matnos, 1);
        mm->matnames = calloc(m.nmats, sizeof(char*));
        s = db_hdf5_comprd(dbfile, m.matnames, 1);
        for (i=0; i<m.nmats; i++) {
            char *tok = strtok(i?NULL:s, ";");
            mm->matnames[i] = STRDUP(tok);
        }
        if (m.nmatnos > 0) {
            char *tmpmaterial_names = db_hdf5_comprd(dbfile, m.material_names, 1);
            char *tmpmat_colors = db_hdf5_comprd(dbfile, m.mat_colors, 1);
            if (tmpmaterial_names)
                mm->material_names = db_StringListToStringArray(tmpmaterial_names,
                                                                m.nmatnos);
            if (tmpmat_colors)
                mm->matcolors = db_StringListToStringArray(tmpmat_colors,
                                                            m.nmatnos);
            FREE(tmpmaterial_names);
            FREE(tmpmat_colors);
        }
        
        H5Tclose(o);
        FREE(s);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMultimat(mm);
        FREE(s);
    } END_PROTECT;
    return mm;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutMultimatspecies
 *
 * Purpose:     Write a multi-mat species object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *
 *              Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *              Added call to reset global data
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutMultimatspecies(DBfile *_dbfile, char *name, int nspec,
                           char *specnames[], DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBmultimatspecies_mt m;
    int                 i, len;
    char                *s=NULL;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        db_ResetGlobalData_MultiMesh();
        db_ProcessOptlist(DB_MULTIMESH, optlist);

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_mm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_mm._time), "time");
        }
        if (_mm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_mm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_mm._cycle), "cycle");

        /*
         * Create a character string which is a semi-colon separated list of
         * material names.
         */
        for (i=len=0; i<nspec; i++) len += strlen(specnames[i])+1;
        s = malloc(len+1);
        for (i=len=0; i<nspec; i++) {
            if (i) s[len++] = ';';
            strcpy(s+len, specnames[i]);
            len += strlen(specnames[i]);
        }
        len++; /*count null*/

        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.specnames/*out*/);
        if (_mm._nmat>0 && _mm._nmatspec) {
            db_hdf5_compwr(dbfile, DB_INT, 1, &_mm._nmat, _mm._nmatspec,
                           m.nmatspec/*out*/);
        }

        /* Initialize meta data */
        m.nspec = nspec;
        m.nmat = _mm._nmat;
        m.cycle = _mm._cycle;
        m.time = _mm._time;
        m.dtime = _mm._dtime;
        m.ngroups = _mm._ngroups;
        m.blockorigin = _mm._blockorigin;
        m.grouporigin = _mm._grouporigin;
        m.guihide = _mm._guihide;
        strcpy(m.matname, OPT(_mm._matname));

        /* Write meta data to file */
        STRUCT(DBmultimatspecies) {
            if (m.nspec)        MEMBER_S(int, nspec);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.ngroups)      MEMBER_S(int, ngroups);
            if (m.blockorigin)  MEMBER_S(int, blockorigin);
            if (m.grouporigin)  MEMBER_S(int, grouporigin);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (_mm._time_set)  MEMBER_S(float, time);
            if (_mm._dtime_set) MEMBER_S(double, dtime);
            if (_mm._nmat>0 && _mm._nmatspec) MEMBER_S(int, nmat);
            MEMBER_S(str(m.specnames), specnames);
            MEMBER_S(str(m.nmatspec), nmatspec);
            MEMBER_S(str(m.matname), matname);
        } OUTPUT(dbfile, DB_MULTIMATSPECIES, name, &m);

        /* Free resources */
        FREE(s);
        
    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetMultimatspecies
 *
 * Purpose:     Reads a multimat species object from the file.
 *
 * Return:      Success:        Ptr to new multimatspecies.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `ngroups', `blockorigin', and `grouporigin' to
 *              duplicate changes to the PDB driver.
 *-------------------------------------------------------------------------
 */
CALLBACK DBmultimatspecies *
db_hdf5_GetMultimatspecies(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetMultimatspecies";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBmultimatspecies_mt m;
    DBmultimatspecies   *mm=NULL;
    char                *s=NULL;

    PROTECT {
        /* Open object and make sure it's a multimatspecies */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_MULTIMATSPECIES!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBmultimatspecies_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(mm=DBAllocMultimatspecies(0))) return NULL;
        mm->nspec = m.nspec;
        mm->ngroups = m.ngroups;
        mm->blockorigin = m.blockorigin;
        mm->grouporigin = m.grouporigin;
        mm->guihide = m.guihide;

        /* Read the raw data */
        mm->specnames = calloc(m.nspec, sizeof(char*));
        s = db_hdf5_comprd(dbfile, m.specnames, 1);
        for (i=0; i<m.nspec; i++) {
            char *tok = strtok(i?NULL:s, ";");
            mm->specnames[i] = STRDUP(tok);
        }
        
        H5Tclose(o);
        FREE(s);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMultimatspecies(mm);
        FREE(s);
    } END_PROTECT;
    return mm;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutPointmesh
 *
 * Purpose:     Writes a pointmesh object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  6, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no' property to duplicate changes made to the
 *              PDB driver.
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutPointmesh(DBfile *_dbfile, char *name, int ndims, float *coords[],
                     int nels, int datatype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_PutPointmesh";
    DBpointmesh_mt      m;
    int                 i, len;
    
    memset(&m, 0, sizeof m);
    PROTECT {
        /* Check datatype */
        if (DB_FLOAT!=datatype && DB_DOUBLE!=datatype) {
            db_perror("invalid floating-point datatype", E_BADARGS, me);
            UNWIND();
        }

        /* Set global options */
        memset(&_pm, 0, sizeof _pm);
        _pm._ndims = _pm._nspace = ndims;
        _pm._group_no = -1;
        db_ProcessOptlist(DB_POINTMESH, optlist);
        _pm._nels = nels;
        _pm._minindex = _pm._lo_offset;
        _pm._maxindex = nels - _pm._hi_offset - 1;

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_pm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_pm._time), "time");
        }
        if (_pm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_pm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_pm._cycle), "cycle");

        /* Write raw data arrays */
        for (i=0; i<ndims; i++) {
            db_hdf5_compwr(dbfile, datatype, 1, &nels, coords[i],
                           m.coord[i]/*out*/);
        }

        /* Find the mesh extents from the coordinate arrays */
        if (DB_DOUBLE==datatype) {
            for (i=0; i<ndims; i++) {
                _DBdarrminmax(((double**)coords)[i], nels,
                              m.min_extents+i, m.max_extents+i);
            }
        } else {
            for (i=0; i<ndims; i++) {
                float min_extents, max_extents;
                _DBarrminmax(coords[i], nels, &min_extents, &max_extents);
                m.min_extents[i] = min_extents;
                m.max_extents[i] = max_extents;
            }
        }

        /* Global node numbers */
        if (_pm._gnodeno)
            db_hdf5_compwr(dbfile, DB_INT, 1, &nels, _pm._gnodeno,
                           m.gnodeno/*out*/);

        /* Build header in memory */
        m.ndims = ndims;
        m.nspace = _pm._nspace;
        m.nels = _pm._nels;
        m.cycle = _pm._cycle;
        m.origin = _pm._origin;
        m.min_index = _pm._minindex;
        m.max_index = _pm._maxindex;
        m.time = _pm._time;
        m.dtime = _pm._dtime;
        m.group_no = _pm._group_no;
        m.guihide = _pm._guihide;
        for (i=0; i<ndims; i++) {
            strcpy(m.label[i], OPT(_pm._labels[i]));
            strcpy(m.units[i], OPT(_pm._units[i]));
        }
        
        /* Write header to file */
        STRUCT(DBpointmesh) {
            if (m.ndims)        MEMBER_S(int, ndims);
            if (m.nspace)       MEMBER_S(int, nspace);
            if (m.nels)         MEMBER_S(int, nels);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (_pm._time_set)  MEMBER_S(float, time);
            if (_pm._dtime_set) MEMBER_S(double, dtime);
            if (m.min_index)    MEMBER_S(int, min_index);
            if (m.max_index)    MEMBER_S(int, max_index);
            if (m.group_no)     MEMBER_S(int, group_no);
            if (m.guihide)      MEMBER_S(int, guihide);
            MEMBER_3(double, min_extents);
            MEMBER_3(double, max_extents);
            MEMBER_R(str(m.coord[_j]), coord, ndims);
            MEMBER_R(str(m.label[_j]), label, ndims);
            MEMBER_R(str(m.units[_j]), units, ndims);
            MEMBER_S(str(m.gnodeno), gnodeno);
        } OUTPUT(dbfile, DB_POINTMESH, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetPointmesh
 *
 * Purpose:     Reads a pointmesh object from the file.
 *
 * Return:      Success:        Ptr to new pointmesh.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April  7, 1999
 *
 * Modifications:
 *              Robb Matzke, 1999-07-13
 *              Added `group_no' to duplicate changes to the PDB driver.
 *
 *              Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *              Made it set datatype correctly. Added support for dataReadMask
 *-------------------------------------------------------------------------
 */
CALLBACK DBpointmesh *
db_hdf5_GetPointmesh(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetPointmesh";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBpointmesh_mt      m;
    DBpointmesh         *pm=NULL;

    PROTECT {
        /* Open object and make sure it's a pointmesh */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_POINTMESH!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBpointmesh_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(pm=DBAllocPointmesh())) return NULL;
        pm->name = BASEDUP(name);
        pm->cycle = m.cycle;
        pm->time = m.time;
        pm->dtime = m.dtime;
        if ((pm->datatype = db_hdf5_GetVarType(_dbfile, m.coord[0])) < 0)
            pm->datatype = DB_FLOAT;
        if (pm->datatype == DB_DOUBLE && force_single_g)
            pm->datatype = DB_FLOAT;
        pm->ndims = m.ndims;
        pm->nels = m.nels;
        pm->group_no = m.group_no;
        pm->guihide = m.guihide;
        for (i=0; i<m.ndims; i++) {
            pm->units[i] = OPTDUP(m.units[i]);
            pm->labels[i] = OPTDUP(m.label[i]);
            if (pm->datatype == DB_DOUBLE)
            {
                ((double*)pm->min_extents)[i] = m.min_extents[i];
                ((double*)pm->max_extents)[i] = m.max_extents[i];
            }
            else
            {
                pm->min_extents[i] = m.min_extents[i];
                pm->max_extents[i] = m.max_extents[i];
            }
        }

        /* Read raw data */
        if (SILO_Globals.dataReadMask & DBPMCoords)
        {
            for (i=0; i<m.ndims; i++) {
                pm->coords[i] = db_hdf5_comprd(dbfile, m.coord[i], 0);
            }
        }
        if (SILO_Globals.dataReadMask & DBPMGlobNodeNo)
            pm->gnodeno = db_hdf5_comprd(dbfile, m.gnodeno, 1);

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreePointmesh(pm);
    } END_PROTECT;
    return pm;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutPointvar
 *
 * Purpose:     Write a pointvar object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April  7, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Added Hack to make HDF5 driver deal with cycle/time same as PDB driver
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_PutPointvar(DBfile *_dbfile, char *name, char *meshname, int nvars,
                    float **vars, int nels, int datatype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBpointvar_mt       m;
    int                 i, saved_ndims, len;

    memset(&m, 0, sizeof m);
    PROTECT {
        /* Set global options */
        saved_ndims = _pm._ndims;
        memset(&_pm, 0, sizeof _pm);
        _pm._ndims = _pm._nspace = saved_ndims;
        _pm._group_no = -1;
        db_ProcessOptlist(DB_POINTMESH, optlist);
        _pm._nels = nels;
        _pm._minindex = _pm._lo_offset;
        _pm._maxindex = nels - _pm._hi_offset - 1;

        /* hack to maintain backward compatibility with pdb driver */
        len = 1;
        if (_pm._time_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_FLOAT, 1, &len, &(_pm._time), "time");
        }
        if (_pm._dtime_set == TRUE) {
            db_hdf5_compwr(dbfile, DB_DOUBLE, 1, &len, &(_pm._dtime), "dtime");
        }
        db_hdf5_compwr(dbfile, DB_INT, 1, &len, &(_pm._cycle), "cycle");

        /* Write raw data arrays */
        for (i=0; i<nvars; i++) {
            db_hdf5_compwr(dbfile, datatype, 1, &nels, vars[i],
                           m.data[i]/*out*/);
        }

        /* Build header in memory */
        m.nvals = nvars;
        m.nels = nels;
        m.nspace = _pm._nspace;
        m.origin = _pm._origin;
        m.min_index = _pm._minindex;
        m.max_index = _pm._maxindex;
        m.cycle = _pm._cycle;
        m.guihide = _pm._guihide;
        m.time = _pm._time;
        m.dtime = _pm._dtime;
        m.ascii_labels = _pm._ascii_labels;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;
        strcpy(m.meshid, OPT(meshname));
        strcpy(m.label, OPT(_pm._label));
        strcpy(m.units, OPT(_pm._unit));

        /* Write header to file */
        STRUCT(DBpointvar) {
            if (m.nvals)        MEMBER_S(int, nvals);
            if (m.nels)         MEMBER_S(int, nels);
            if (m.nspace)       MEMBER_S(int, nspace);
            if (m.origin)       MEMBER_S(int, origin);
            if (m.datatype)     MEMBER_S(int, datatype);
            if (m.min_index)    MEMBER_S(int, min_index);
            if (m.max_index)    MEMBER_S(int, max_index);
            if (m.cycle)        MEMBER_S(int, cycle);
            if (m.guihide)      MEMBER_S(int, guihide);
            if (m.ascii_labels) MEMBER_S(int, ascii_labels);
            if (_pm._time_set)  MEMBER_S(float, time);
            if (_pm._dtime_set) MEMBER_S(double, dtime);
            MEMBER_S(str(m.meshid), meshid);
            MEMBER_S(str(m.label), label);
            MEMBER_S(str(m.units), units);
            MEMBER_R(str(m.data[_j]), data, m.nvals);
        } OUTPUT(dbfile, DB_POINTVAR, name, &m);

    } CLEANUP {
        /*void*/
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetPointvar
 *
 * Purpose:     Reads a pointvar object from the file.
 *
 * Return:      Success:        Ptr to new pointvar object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Thu Jul 29 11:26:24 PDT 2004
 *   Made it set datatype correctly. Added support for dataReadMask
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBmeshvar *
db_hdf5_GetPointvar(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetPointvar";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBpointvar_mt       m;
    DBmeshvar           *pv=NULL;

    PROTECT {
        /* Open object and make sure it's a pointvar */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_POINTVAR!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBpointvar_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(pv=DBAllocMeshvar())) return NULL;
        pv->name = BASEDUP(name);
        pv->units = OPTDUP(m.units);
        pv->meshname = OPTDUP(m.meshid);
        pv->label = OPTDUP(m.label);
        pv->cycle = m.cycle;
        if ((pv->datatype = db_hdf5_GetVarType(_dbfile, m.data[0])) < 0)
            pv->datatype = silo2silo_type(m.datatype);
        if (pv->datatype == DB_DOUBLE && force_single_g)
            pv->datatype = DB_FLOAT;
        pv->nels = m.nels;
        pv->nvals = m.nvals;
        pv->nspace = m.nspace;
        pv->ndims = 1;
        pv->origin = m.origin;
        pv->time = m.time;
        pv->dtime = m.dtime;
        pv->min_index[0] = m.min_index;
        pv->max_index[0] = m.max_index;
        pv->guihide = m.guihide;
        pv->ascii_labels = m.ascii_labels;

        /* Read raw data */
        if (SILO_Globals.dataReadMask & DBPVData)
        {
            pv->vals = calloc(m.nvals, sizeof(void*));
            for (i=0; i<m.nvals; i++) {
                pv->vals[i] = db_hdf5_comprd(dbfile, m.data[i], 0);
            }
        }

        H5Tclose(o);
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeMeshvar(pv);
    } END_PROTECT;
    return pv;
}
        

/*-------------------------------------------------------------------------
 * Function:    db_hdf5_PutCompoundarray
 *
 * Purpose:     Writes a compound array object to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
CALLBACK int
db_hdf5_PutCompoundarray(DBfile *_dbfile, char *name, char *elmtnames[],
                         int elmtlen[], int nelmts, void *values,
                         int nvalues, int datatype, DBoptlist *optlist)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    DBcompoundarray_mt  m;
    int                 i, len;
    char                *s=NULL;

    memset(&m, 0, sizeof m);
    PROTECT {
        /*
         * Create a character string which is a semi-colon separated list of
         * component names.
         */
        for (i=len=0; i<nelmts; i++) len += strlen(elmtnames[i])+1;
        s = malloc(len+1);
        for (i=len=0; i<nelmts; i++) {
            if (i) s[len++] = ';';
            strcpy(s+len, elmtnames[i]);
            len += strlen(elmtnames[i]);
        }
        len++; /*count null*/

        /* Write raw data arrays */
        db_hdf5_compwr(dbfile, datatype, 1, &nvalues, values, m.values);
        db_hdf5_compwr(dbfile, DB_CHAR, 1, &len, s, m.elemnames);
        db_hdf5_compwr(dbfile, DB_INT, 1, &nelmts, elmtlen, m.elemlengths);

        /* Initialize meta data */
        m.nelems = nelmts;
        m.nvalues = nvalues;
        m.datatype = (DB_FLOAT==datatype || DB_DOUBLE==datatype)?0:datatype;

        /* Write meta data to file */
        STRUCT(DBcompoundarray) {
            if (m.nelems)       MEMBER_S(int, nelems);
            if (m.nvalues)      MEMBER_S(int, nvalues);
            if (m.datatype)     MEMBER_S(int, datatype);
            MEMBER_S(str(m.values), values);
            MEMBER_S(str(m.elemnames), elemnames);
            MEMBER_S(str(m.elemlengths), elemlengths);
        } OUTPUT(dbfile, DB_ARRAY, name, &m);

        /* Free resources */
        FREE(s);
        
    } CLEANUP {
        FREE(s);
    } END_PROTECT;
    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_GetCompoundarray
 *
 * Purpose:     Reads a compound array object from the file.
 *
 * Return:      Success:        Ptr to new compound array.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Aug  2 15:06:57 PDT 2004
 *   Made it set datatype correctly
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBcompoundarray *
db_hdf5_GetCompoundarray(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_GetCompoundarray";
    hid_t               o=-1, attr=-1;
    int                 _objtype, i;
    DBcompoundarray_mt m;
    DBcompoundarray     *ca=NULL;
    char                *s=NULL;

    PROTECT {
        /* Open object and make sure it's a compund array */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            db_perror(name, E_NOTFOUND, me);
            UNWIND();
        }
        if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
            H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }
        if (DB_ARRAY!=(DBObjectType)_objtype) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Read meta data into memory */
        memset(&m, 0, sizeof m);
        if ((attr=H5Aopen_name(o, "silo"))<0 ||
            H5Aread(attr, DBcompoundarray_mt5, &m)<0 ||
            H5Aclose(attr)<0) {
            db_perror(name, E_CALLFAIL, me);
            UNWIND();
        }

        /* Create object and initialize meta data */
        if (NULL==(ca=DBAllocCompoundarray())) return NULL;
        ca->name = BASEDUP(name);
        ca->nelems = m.nelems;
        ca->nvalues = m.nvalues;
        if ((ca->datatype = db_hdf5_GetVarType(_dbfile, m.values)) < 0)
            ca->datatype = silo2silo_type(m.datatype);
        if (ca->datatype == DB_DOUBLE && force_single_g)
            ca->datatype = DB_FLOAT;
        
        /* Read the raw data */
        ca->elemlengths = db_hdf5_comprd(dbfile, m.elemlengths, 1);
        ca->values = db_hdf5_comprd(dbfile, m.values, 1);
        ca->elemnames = calloc(m.nelems, sizeof(char*));
        s = db_hdf5_comprd(dbfile, m.elemnames, 1);
        for (i=0; i<m.nelems; i++) {
            char *tok = strtok(i?NULL:s, ";");
            ca->elemnames[i] = STRDUP(tok);
        }
        
        H5Tclose(o);
        FREE(s);
        
    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
        DBFreeCompoundarray(ca);
        FREE(s);
    } END_PROTECT;
    return ca;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_InqVarType
 *
 * Purpose:     Returns the object type for an object.
 *
 * Return:      Success:        Object type
 *
 *              Failure:        DB_INVALID_OBJECT
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Feb  1 13:48:33 PST 2005
 *   Fixed return of variable type for case of QUAD_RECT | QUAD_CURV
 *
 *   Mark C. Miller, Mon Feb 14 20:16:50 PST 2005
 *   Undid fix above
 *
 *   Mark C. Miller, Wed Apr 20 16:05:23 PDT 2005
 *   Made it return DB_VARIABLE for any inquiry where it couldn't
 *   get actual variable type from the object
 *
 *   Mark C. Miller, Mon Jul 17 18:07:57 PDT 2006
 *   Improved fix, above, for inquries on "/.silo/#000XXXX" datasets
 *   for silex
 *
 *-------------------------------------------------------------------------
 */
CALLBACK DBObjectType
db_hdf5_InqVarType(DBfile *_dbfile, char *name)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_InqVarType";
    hid_t               o=-1, attr=-1;
    int                 _objtype = DB_INVALID_OBJECT;

    PROTECT {

        /* Open object */
        if ((o=H5Topen(dbfile->cwg, name))<0) {
            if ((o=H5Gopen(dbfile->cwg, name))<0) {
                _objtype = DB_VARIABLE;
            }
            else
            {
                _objtype = DB_DIR;
                H5Gclose(o);
            }
        }
        else
        {
            /* Read the `silo_type' attribute */
            if ((attr=H5Aopen_name(o, "silo_type"))<0 ||
                H5Aread(attr, H5T_NATIVE_INT, &_objtype)<0 ||
                H5Aclose(attr)<0) {
                _objtype = DB_VARIABLE;
            }
            H5Tclose(o);
        }

    } CLEANUP {
        H5E_BEGIN_TRY {
            H5Aclose(attr);
            H5Tclose(o);
        } H5E_END_TRY;
    } END_PROTECT;
    return (DBObjectType)_objtype;
}


/*-------------------------------------------------------------------------
 * Function:    db_hdf5_InqMeshName
 *
 * Purpose:     Returns the name of the mesh with which the specified
 *              variable is associated.
 *
 * Bugs:        The user must pass a large enough buffer for the `meshname'
 *              argument. This function will never copy more than 1024 bytes
 *              into the `meshname' array, counting the null termination.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              Friday, April  9, 1999
 *
 * Modifications:
 *
 *   Mark C. Miller, Tue Oct 17 08:16:13 PDT 2006
 *   Made it try either "meshid" or "meshname" data members
 *
 *-------------------------------------------------------------------------
 */
CALLBACK int
db_hdf5_InqMeshName(DBfile *_dbfile, char *name, char *meshname/*out*/)
{
    DBfile_hdf5         *dbfile = (DBfile_hdf5*)_dbfile;
    static char         *me = "db_hdf5_InqMeshName";
    int                  pass;

    for (pass = 0; pass < 2; pass++)
    {
        char   s[1024];
        hid_t  o=-1, attr=-1, type=-1, str_type=-1;

        PROTECT {
            /* Describe memory */
            if ((str_type=H5Tcopy(H5T_C_S1))<0 ||
                H5Tset_size(str_type, sizeof s)<0 ||
                (type=H5Tcreate(H5T_COMPOUND, sizeof s))<0 ||
                db_hdf5_put_cmemb(type, pass==0?"meshid":"meshname",
                                  0, 0, NULL, str_type)<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }

            /* Open variable */
            if ((o=H5Topen(dbfile->cwg, name))<0 ||
                (attr=H5Aopen_name(o, "silo"))<0) {
                db_perror(name, E_NOTFOUND, me);
                UNWIND();
            }

            /*
             * Read "silo" attribute. If the read fails it's probably because
             * there is no "meshid" field in the attribute, in which case we
             * haven't opened a mesh variable and we should fail.
             */
            s[0] = '\0';
            if (H5Aread(attr, type, s)<0) {
                db_perror(name, E_CALLFAIL, me);
                UNWIND();
            }
        
            /* Copy result to output buffer and release resources */
            strcpy(meshname, s);
            H5Aclose(attr);
            H5Tclose(type);
            H5Tclose(str_type);
            H5Tclose(o);

        } CLEANUP {
            H5E_BEGIN_TRY {
                H5Aclose(attr);
                H5Tclose(type);
                H5Tclose(str_type);
                H5Tclose(o);
            } H5E_END_TRY;
        } END_PROTECT;

        if (s[0] != '\0')
            break;
    }
    return 0;
}

#else
/* Stub for when we don't have hdf5 */
INTERNAL DBfile *
db_hdf5_Open(char *name, int mode, int subtype)
{
    db_perror(name, E_NOTIMP, "db_hdf5_Open");
    return NULL;
}

/* Stub for when we don't have hdf5 */
INTERNAL DBfile *
db_hdf5_Create(char *name, int mode, int target, int subtype, char *finfo)
{
    db_perror(name, E_NOTIMP, "db_hdf5_Create");
    return NULL;
}

/* Stub for when we don't have hdf5 */
INTERNAL int
db_hdf5_ForceSingle(int satus)
{
    return 0; /*no-op, don't fail*/
}


#endif /* defined(HAVE_HDF5_H) && defined(HAVE_LIBHDF5) */

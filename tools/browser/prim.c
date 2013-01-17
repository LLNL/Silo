/*
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
*/
/*-------------------------------------------------------------------------
 *
 * Created:             prim.c
 *                      Dec  5 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Primitive type class. A primitive type is one
 *                      of the strings `string', `int', `float' or `double'
 *                      or one of the SILO primitive type constants stored
 *                      as ASCII, or the name of a silo object or component
 *                      thereof holding a SILO primitive type constant.
 *
 * Modifications:       Jeremy Meredith, Sept 21, 1998
 *                      Added multimatspecies type.
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#include <config.h>
#include <ctype.h>
#include <math.h>
#define MYCLASS(X)      ((obj_prim_t*)(X))

#define BROWSER_STR     0               /*char string (ptr to array of char) */
#define BROWSER_INT8    1               /*8-bit signed integers              */
#define BROWSER_SHORT   2
#define BROWSER_INT     3
#define BROWSER_LONG    4
#define BROWSER_FLOAT   5
#define BROWSER_DOUBLE  6
#define BROWSER_LONG_LONG 7

#define PA_NAME         (-999999)

typedef struct obj_prim_t {
   obj_pub_t    pub;
   char         *name ;                 /*name of type or variable      */
   char         *tname ;                /*name of type (var resolved)   */
   int          browser_type ;          /*browser type constant         */
   int          nbytes ;                /*size of the type              */
   prim_assoc_t *assoc ;                /*I/O association table         */
} obj_prim_t;

class_t         C_PRIM;
static obj_t    prim_new (va_list);
static obj_t    prim_copy (obj_t, int);
static obj_t    prim_dest (obj_t);
static char *   prim_name (obj_t);
static obj_t    prim_feval (obj_t);
static obj_t    prim_apply (obj_t, obj_t);
static void     prim_print (obj_t, out_t*);
static void     prim_walk1 (obj_t, void*, int, walk_t*);
static int      prim_walk2 (obj_t, void*, obj_t, void*, walk_t*);
static int      prim_walk3 (void*, obj_t, obj_t);
static int      prim_sizeof (obj_t);
static obj_t    prim_bind (obj_t, void*);
static int      prim_diff (obj_t, obj_t);

prim_assoc_t    PA_BR_OBJTYPE[] = {
   {PA_NAME,                                    "FileObjectType"},
   {BROWSER_DB_CURVE,           "curve"},
   {BROWSER_DB_MULTIMESH,       "multimesh"},
   {BROWSER_DB_MULTIMESHADJ,    "multimeshadj"},
   {BROWSER_DB_MULTIVAR,        "multivar"},
   {BROWSER_DB_MULTIMAT,        "multimat"},
   {BROWSER_DB_MULTIMATSPECIES, "multimatspecies"},
   {BROWSER_DB_QMESH,           "quadmesh"},
   {BROWSER_DB_QVAR,            "quadvar"},
   {BROWSER_DB_UCDMESH,         "ucdmesh"},
   {BROWSER_DB_UCDVAR,          "ucdvar"},
   {BROWSER_DB_PTMESH,          "ptmesh"},
   {BROWSER_DB_PTVAR,           "ptvar"},
   {BROWSER_DB_MAT,             "material"},
   {BROWSER_DB_MATSPECIES,      "matspecies"},
   {BROWSER_DB_VAR,             "var"},
   {BROWSER_DB_OBJ,             "obj"},
   {BROWSER_DB_DIR,             "directory"},
   {BROWSER_DB_ARRAY,           "array"},
   {BROWSER_DB_DEFVARS,         "defvars"},
   {BROWSER_DB_CSGMESH,         "csgmesh"},
   {BROWSER_DB_CSGVAR,          "csgvar"},
   {BROWSER_DB_MRGTREE,         "mrgtree"},
   {BROWSER_DB_GROUPELMAP,      "groupelmap"},
   {BROWSER_DB_MRGVAR,          "mrgvar"},
   {0,                          NULL}};

prim_assoc_t    PA_OBJTYPE[] = {
   {PA_NAME,                                    "DBObjectType"},
   {DB_QUADMESH,        "DB_QUADMESH"},
   {DB_QUADVAR,         "DB_QUADVAR"},
   {DB_UCDMESH,         "DB_UCDMESH"},
   {DB_UCDVAR,          "DB_UCDVAR"},
   {DB_MULTIMESH,       "DB_MULTIMESH"},
   {DB_MULTIMESHADJ,    "DB_MULTIMESHADJ"},
   {DB_MULTIVAR,        "DB_MULTIVAR"},
   {DB_MULTIMAT,        "DB_MULTIMAT"},
   {DB_MULTIMATSPECIES, "DB_MULTIMATSPECIES"},
   {DB_MULTIBLOCKMESH,  "DB_MULTIBLOCKMESH"},
   {DB_MULTIBLOCKVAR,   "DB_MULTIBLOCKVAR"},
   {DB_MATERIAL,        "DB_MATERIAL"},
   {DB_MATSPECIES,      "DB_MATSPECIES"},
   {DB_FACELIST,        "DB_FACELIST"},
   {DB_ZONELIST,        "DB_ZONELIST"},
   {DB_PHZONELIST,      "DB_PHZONELIST"},
   {DB_EDGELIST,        "DB_EDGELIST"},
   {DB_CURVE,           "DB_CURVE"},
   {DB_POINTMESH,       "DB_POINTMESH"},
   {DB_POINTVAR,        "DB_POINTVAR"},
   {DB_ARRAY,           "DB_ARRAY"},
   {DB_DEFVARS,         "DB_DEFVARS"},
   {DB_CSGMESH,         "DB_CSGMESH"},
   {DB_CSGVAR,          "DB_CSGVAR"},
   {DB_MRGTREE,         "DB_MRGTREE"},
   {DB_GROUPELMAP,      "DB_GROUPELMAP"},
   {DB_MRGVAR,          "DB_MRGVAR"},
   {DB_USERDEF,         "DB_USERDEF"},
   {0,                  NULL}};

prim_assoc_t    PA_DATATYPE[] = {
   {PA_NAME,                                    "DBdatatype"},
   {DB_INT,             "int"},
   {DB_SHORT,           "short"},
   {DB_LONG,            "long"},
   {DB_FLOAT,           "float"},
   {DB_DOUBLE,          "double"},
   {DB_CHAR,            "char"},
   {DB_LONG_LONG,       "long_long"},
   {DB_NOTYPE,          "notype"},
   {0,                  NULL}};

prim_assoc_t    PA_ORDER[] = {
   {PA_NAME,                                    "array order"},
   {DB_ROWMAJOR,        "row major"},
   {DB_COLMAJOR,        "column major"},
   {0,                  NULL}};

prim_assoc_t    PA_ONOFF[] = {
   {PA_NAME,                                    "on/off"},
   {DB_ON,              "on"},
   {DB_OFF,             "off"},
   {0,                  NULL}};

prim_assoc_t    PA_BOOLEAN[] = {
   {PA_NAME,                                    "Boolean"},
   {1,                  "true"},
   {0,                  "false"},
   {0,                  NULL}};

prim_assoc_t    PA_COORDSYS[] = {
   {PA_NAME,                                    "coordinate system"},
   {DB_CARTESIAN,       "Cartesian"},
   {DB_CYLINDRICAL,     "cylindrical"},
   {DB_SPHERICAL,       "spherical"},
   {DB_NUMERICAL,       "numerical"},
   {DB_OTHER,           "other"},
   {0,                  NULL}};

prim_assoc_t    PA_COORDTYPE[] = {
   {PA_NAME,                                    "coordinate type"},
   {DB_COLLINEAR,       "collinear"},
   {DB_NONCOLLINEAR,    "noncollinear"},
   {DB_QUAD_RECT,       "quad_rect"},
   {DB_QUAD_CURV,       "quad_curv"},
   {0,                  NULL}};

prim_assoc_t    PA_FACETYPE[] = {
   {PA_NAME,                                    "face type"},
   {DB_RECTILINEAR,     "rectilinear"},
   {DB_CURVILINEAR,     "curvilinear"},
   {0,                  NULL}};

prim_assoc_t    PA_PLANAR[] = {
   {PA_NAME,                                    "planar"},
   {DB_AREA,            "area"},
   {DB_VOLUME,          "volume"},
   {0,                  NULL}};

prim_assoc_t    PA_CENTERING[] = {
   {PA_NAME,                                    "centering"},
   {DB_NOTCENT,         "not centered"},
   {DB_NODECENT,        "node centered"},
   {DB_ZONECENT,        "zone centered"},
   {DB_FACECENT,        "face centered"},
   {DB_EDGECENT,        "edge centered"},
   {DB_BNDCENT,         "bondary centered"},
   {DB_BLOCKCENT,       "block centered"},
   {0,                  NULL}};
    
prim_assoc_t    PA_DEFVARTYPE[] = {
   {PA_NAME,                                    "derived variable type"},
   {DB_VARTYPE_SCALAR,    "scalar"},
   {DB_VARTYPE_VECTOR,    "vector"},
   {DB_VARTYPE_TENSOR,    "tensor"},
   {DB_VARTYPE_SYMTENSOR, "symmetric tensor"},
   {DB_VARTYPE_ARRAY,     "array"},
   {DB_VARTYPE_MATERIAL,  "material"},
   {DB_VARTYPE_SPECIES,   "species"},
   {DB_VARTYPE_LABEL,     "LABEL"},
   {0,                    NULL}};
    
prim_assoc_t    PA_REGIONOP[] = {
   {PA_NAME,                                    "region op"},
   {DBCSG_INNER,        "INNER"},
   {DBCSG_OUTER,        "OUTER"},
   {DBCSG_ON,           "ON"},
   {DBCSG_UNION,        "UNION"},
   {DBCSG_INTERSECT,    "INTERSECT"},
   {DBCSG_DIFF,         "DIFF"},
   {DBCSG_XFORM,        "XFORM"},
   {DBCSG_COMPLIMENT,   "COMPLIMENT"},
   {DBCSG_SWEEP,        "SWEEP"},
   {0,                  NULL}};

prim_assoc_t    PA_BOUNDARYTYPE[] = {
   {PA_NAME,                                    "boundary type"},
   {DBCSG_QUADRIC_G,        "QUADRIC_G"},
   {DBCSG_SPHERE_PR,        "SPHERE_PR"},
   {DBCSG_ELLIPSOID_PRRR,   "ELLIPSOID_PRRR"},
   {DBCSG_PLANE_G,          "PLANE_G"},
   {DBCSG_PLANE_X,          "PLANE_X"},
   {DBCSG_PLANE_Y,          "PLANE_Y"},
   {DBCSG_PLANE_Z,          "PLANE_Z"},
   {DBCSG_PLANE_PN,         "PLANE_PN"},
   {DBCSG_PLANE_PPP,        "PLANE_PPP"},
   {DBCSG_CYLINDER_PNLR,    "CYLINDER_PNLR"},
   {DBCSG_CYLINDER_PPR,     "CYLINDER_PPR"},
   {DBCSG_BOX_XYZXYZ,       "BOX_XYZXYZ"},
   {DBCSG_CONE_PNLA,        "CONE_PNLA"},
   {DBCSG_CONE_PPA,         "CONE_PPA"},
   {DBCSG_POLYHEDRON_KF,    "POLYHEDRON_KF"},
   {DBCSG_HEX_6F,           "HEX_6F"},
   {DBCSG_TET_4F,           "TET_4F"},
   {DBCSG_PYRAMID_5F,       "PYRAMID_5F"},
   {DBCSG_PRISM_5F,         "PRISM_5F"},
   {DBCSG_QUADRATIC_G,      "QUADRATIC_G"},
   {DBCSG_CIRCLE_PR,        "CIRCLE_PR"},
   {DBCSG_ELLIPSE_PRR,      "ELLIPSE_PRR"},
   {DBCSG_LINE_G,           "LINE_G"},
   {DBCSG_LINE_X,           "LINE_X"},
   {DBCSG_LINE_Y,           "LINE_Y"},
   {DBCSG_LINE_PN,          "LINE_PN"},
   {DBCSG_LINE_PP,          "LINE_PP"},
   {DBCSG_BOX_XYXY,         "BOX_XYXY"},
   {DBCSG_ANGLE_PNLA,       "ANGLE_PNLA"},
   {DBCSG_ANGLE_PPA,        "ANGLE_PPA"},
   {DBCSG_POLYGON_KP,       "POLYGON_KP"},
   {DBCSG_TRI_3P,           "TRI_3P"},
   {DBCSG_QUAD_4P,          "QUAD_4P"},
   {0,                      NULL}};

prim_assoc_t PA_TOPODIM[] = {
   {PA_NAME,                                    "topological dimension"},
   {-1,        "not specified"},
   { 0,        "0"},
   { 1,        "1"},
   { 2,        "2"},
   { 3,        "3"},
   {0,         NULL}};

prim_assoc_t PA_REPRBLOCK[] = {
   {PA_NAME,                                    "representative block index"},
   {-1,        "not specified"},
   {0,         NULL}};


/*-------------------------------------------------------------------------
 * Function:    prim_class
 *
 * Purpose:     Initializes the PRIM class.
 *
 * Return:      Success:        Ptr to the class.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
prim_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("PRIM");
   cls->new = prim_new;
   cls->copy = prim_copy;
   cls->dest = prim_dest;
   cls->objname = prim_name;
   cls->feval = prim_feval;
   cls->apply = prim_apply;
   cls->print = prim_print;
   cls->walk1 = prim_walk1;
   cls->walk2 = prim_walk2;
   cls->walk3 = prim_walk3;
   cls->size_of = prim_sizeof;
   cls->bind = prim_bind;
   cls->diff = prim_diff;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    prim_new
 *
 * Purpose:     Creates a new primitive type.
 *
 * Return:      Success:        Ptr to new PRIM object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
prim_new (va_list ap) {

   obj_prim_t   *self = calloc (1, sizeof(obj_prim_t));
   char         *s;

   assert (self);
   s = va_arg (ap, char*);
   self->name = safe_strdup (s);
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    prim_copy
 *
 * Purpose:     Copies a primitive type.
 *
 * Return:      Success:        Ptr to copy of SELF
 *
 *              Failure:        abort()
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 22 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
prim_copy (obj_t _self, int flag) {

   obj_prim_t   *self = MYCLASS(_self);
   obj_prim_t   *retval = NULL;

   if (SHALLOW==flag) {
      retval = self;

   } else {
      retval = calloc (1, sizeof(obj_prim_t));
      retval->name = safe_strdup (self->name);
      retval->tname = self->tname ? safe_strdup (self->tname) : NULL;
      retval->browser_type = self->browser_type;
      retval->nbytes = self->nbytes;
      retval->assoc = self->assoc;
      
   }
   return (obj_t)retval;
}


/*-------------------------------------------------------------------------
 * Function:    prim_dest
 *
 * Purpose:     Destroys a primitive type object.
 *
 * Return:      Success:        NIL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
prim_dest (obj_t _self) {

   obj_prim_t   *self = MYCLASS(_self);

   if (0==self->pub.ref) {
      free (self->name);
      if (self->tname) free (self->tname);
      memset (self, 0, sizeof(obj_prim_t));
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    prim_name
 *
 * Purpose:     Returns a pointer to the name of the primitive type.
 *
 * Return:      Success:        Ptr to name.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
prim_name (obj_t _self) {

   obj_prim_t   *self = MYCLASS(_self);

   return self->tname ? self->tname : self->name;
}


/*-------------------------------------------------------------------------
 * Function:    prim_feval
 *
 * Purpose:     Functional value of a type is the type itself.
 *
 * Return:      Success:        Copy of SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
prim_feval (obj_t _self) {

   return obj_copy (_self, SHALLOW);
}


/*-------------------------------------------------------------------------
 * Function:    prim_apply
 *
 * Purpose:     Applying a primitive type to an argument list consisting of
 *              a single SILO database object (SDO) causes the object to
 *              be cast to that type.
 *
 * Return:      Success:        Ptr to a new SDO object with the appropriate
 *                              type.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
prim_apply (obj_t _self, obj_t args) {

   obj_t        sdo=NIL, retval=NIL;

   if (1!=F_length(args)) {
      out_errorn ("typecast: wrong number of arguments");
      return NIL;
   }
   
   sdo = obj_eval (cons_head (args));
   retval = sdo_cast (sdo, _self);
   obj_dest (sdo);
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    prim_print
 *
 * Purpose:     Prints the name of a primitive type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 5 Feb 1997
 *      Prints the name of the I/O assoc array if the primitive type
 *      has one and the array has a name.
 *
 *-------------------------------------------------------------------------
 */
static void
prim_print (obj_t _self, out_t *f) {

   obj_prim_t   *self = MYCLASS(_self);

   if (self->tname) {
      out_puts (f, self->tname);
   } else {
      out_puts (f, self->name);
   }
   if (self->assoc && PA_NAME==self->assoc[0].n && self->assoc[0].s) {
      out_printf (f, " (%s)", self->assoc[0].s);
   }
}

/*---------------------------------------------------------------------------
 * Purpose:     Given a pointer to some memory of known length, render that
 *              memory as an unsigned octal integer stored in big-endian
 *              order. The complication is that an octal "digit" is three
 *              bits, which is not a divisor of eight.
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 19, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
prim_octal(char *buf/*out*/, const void *_mem, size_t nbytes)
{
    unsigned char       *mem = (unsigned char*)_mem;
    size_t              qlen=0, at=0, stride=(8*nbytes)%3;
    unsigned            q=0;

    
    while (nbytes>0 || qlen) {

        /* fill the queue */
        if (qlen<3) {
            assert(nbytes);
            q = (q<<8) | *mem++;
            qlen += 8;
            --nbytes;
        }

        /* use up the high-order bits */
        qlen -= stride;
        buf[at++] = "01234567"[q>>qlen];
        q &= (1<<qlen)-1;
        stride = 3;
    }
    buf[at] = '\0';
}


/*-------------------------------------------------------------------------
 * Function:    prim_walk1
 *
 * Purpose:     Print memory cast as a primitive type.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 2 Sep 1997
 *      Added the BROWSER_INT8 data type.
 *
 *      Robb Matzke, 2000-06-01
 *      Added `$' to the built-in format symbols.
 *
 *      Robb Matzke, 2000-06-02
 *      If the $fmt_string format is nothing then call out_putw(). This is
 *      used by the documentation system.
 *
 *      Robb Matzke, 2000-10-19
 *      Use the $obase variable. Reindented.
 *
 *      Mark C. Miller, Wed Sep 23 11:51:52 PDT 2009
 *      Added long long case.
 *
 *      Mark C. Miller, Mon Jan 11 16:08:57 PST 2010
 *      Fixed handling base16/8 for long and long long types.
 *-------------------------------------------------------------------------
 */
static void
prim_walk1 (obj_t _self, void *mem, int operation, walk_t *wdata)
{
    obj_prim_t          *self = MYCLASS(_self);
    char                buf[1024], *s;
    out_t               *f=NULL;
    int                 i, j, obase;
    unsigned long       u, nbits, mask;
    unsigned long long  ull;

    switch (operation) {
    case WALK_PRINT:
        f = (wdata && wdata->f) ? wdata->f : OUT_STDOUT;

        /* Find the format string. The special strings `b16', `b8', and
         * `b2' mean hexadecimal, octal, or binary format. */
        obase = sym_bi_true("obase");
        if (16==obase) {
            strcpy(buf, "b16");
        } else if (8==obase) {
            strcpy(buf, "b8");
        } else if (2==obase) {
            strcpy(buf, "b2");
        } else {
            obase = 0;
            sprintf(buf, "fmt_%s", obj_name(_self));
            if ((s=sym_bi_gets(buf))) {
                strcpy(buf, s);
                free(s);
            } else {
                buf[0] = '\0';
            }
        }

        /* We better have a format string by now. */
        if (!buf[0] && BROWSER_STR!=self->browser_type) {
            out_errorn("no format for primitive type %s", obj_name(_self));
        } else if (!mem) {
            out_printf(f, "(%s*)NULL", obj_name(_self));
        } else {
            switch (self->browser_type) {
            case BROWSER_STR:
                if (NULL==*((char**)mem)) {
                    if ((s=sym_bi_gets("fmt_null"))) {
                        strcpy(buf, s);
                        free(s);
                    } else {
                        strcpy(buf, "%s");
                    }
                    out_printf(f, buf, "(null)");
                } else if (!buf[0]) {
                    out_putw(f, *((char**)mem));
                } else {
                    str_doprnt(f, buf, *((char**)mem));
                }
                break;
                
            case BROWSER_INT8:
                u = *((unsigned char*)mem);
                nbits = 8;
                if (16==obase) {
                    out_printf(f, "%02x", u);
                } else if (8==obase) {
                    out_printf(f, "%03x", u);
                } else if (2==obase) {
                    for (i=0, mask=1<<(nbits-1); i<nbits; i++, mask>>=1) {
                        sprintf(buf+i, "%c", u&mask?'1':'0');
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((signed char*)mem));
                }
                break;
                
            case BROWSER_SHORT:
                u = *((unsigned short*)mem);
                nbits = 8*sizeof(short);
                if (16==obase) {
                    out_printf(f, "%0*x", 2*sizeof(short), u);
                } else if (8==obase) {
                    out_printf(f, "%0*o", (nbits+2)/3, u);
                } else if (2==obase) {
                    for (i=0; i<sizeof(short); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((short*)mem));
                }
                break;

            case BROWSER_INT:
                u = *((unsigned*)mem);
                nbits = 8*sizeof(int);
                for (i=0; self->assoc && self->assoc[i].s; i++) {
                    if (*((int*)mem) == self->assoc[i].n) break;
                }
                if (self->assoc && self->assoc[i].s) {
                    out_printf(f, "%s", self->assoc[i].s);
                } else if (16==obase) {
                    out_printf(f, "%0*x", 2*sizeof(int), u);
                } else if (8==obase) {
                    out_printf(f, "%0*o", (nbits+2)/3, u);
                } else if (2==obase) {
                    for (i=0; i<sizeof(int); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((int*)mem));
                }
                break;
                
            case BROWSER_LONG:
                u = *((unsigned long*)mem);
                nbits = 8*sizeof(long);
                if (16==obase) {
                    out_printf(f, "%0*lx", 2*sizeof(long), u);
                } else if (8==obase) {
                    out_printf(f, "%0*lo", (nbits+2)/3, u);
                } else if (2==obase) {
                    for (i=0; i<sizeof(long); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((long*)mem));
                }
                break;

            case BROWSER_LONG_LONG:
                ull = *((unsigned long long*)mem);
                nbits = 8*sizeof(long long);
                if (16==obase) {
                    out_printf(f, "%0*llx", 2*sizeof(long long), ull);
                } else if (8==obase) {
                    out_printf(f, "%0*llo", (nbits+2)/3, ull);
                } else if (2==obase) {
                    for (i=0; i<sizeof(long long); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((long long*)mem));
                }
                break;

            case BROWSER_FLOAT:
                if (16==obase) {
                    for (i=0; i<sizeof(float); i++) {
                        sprintf(buf+2*i, "%02x", *((unsigned char*)mem+i));
                    }
                    out_puts(f, buf);
                } else if (8==obase) {
                    prim_octal(buf, mem, sizeof(float));
                    out_puts(f, buf);
                } else if (2==obase) {
                    for (i=0; i<sizeof(float); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((float*)mem));
                }
                break;
                
            case BROWSER_DOUBLE:
                if (16==obase) {
                    for (i=0; i<sizeof(double); i++) {
                        sprintf(buf+2*i, "%02x", *((unsigned char*)mem+i));
                    }
                    out_puts(f, buf);
                } else if (8==obase) {
                    prim_octal(buf, mem, sizeof(double));
                    out_puts(f, buf);
                } else if (2==obase) {
                    for (i=0; i<sizeof(double); i++) {
                        u = *((unsigned char*)mem+i);
                        for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                            sprintf(buf+i*8+j, "%c", u&mask?'1':'0');
                        }
                    }
                    out_puts(f, buf);
                } else {
                    out_printf(f, buf, *((double*)mem));
                }
                break;

            default:
                abort();
            }
        }
        break;

    case WALK_RETRIEVE:
        /* Retrieve the integer value into a buffer. */        
        if (wdata->nvals<0) return ;    /*error already detected*/
        assert(self->tname);
        if (strcmp(self->tname, "int")) {
            out_errorn("prim_walk1: cannot retrieve a non-integer type");
            wdata->nvals = -1;
            return;
        }
        if (!mem) {
            out_errorn("prim_walk1: no associated memory for value retrieval");
            wdata->nvals = -1;
            return;
        }
        if (wdata->nvals>=wdata->maxvals) {
            out_errorn("prim_walk1: overflow at %d value%s",
                       wdata->maxvals,
                       1==wdata->maxvals?"":"s");
            wdata->nvals = -1;
            return;
        }
        wdata->vals[wdata->nvals] = *((int *)mem);
        wdata->nvals += 1;
        break;

    default:
        abort();
    }
}

/*-------------------------------------------------------------------------
 * Function:    prim_getval 
 *
 * Purpose:     Return value of primitive as long long.
 *
 * Mark C. Miller, Mon Jan 11 16:10:26 PST 2010
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static long long
prim_getval_ll(int type, void *mem)
{
    switch (type) {
    case BROWSER_STR:       return 0;
    case BROWSER_INT8:      return (long long) *((signed char*)mem);
    case BROWSER_SHORT:     return (long long) *((short*)mem);
    case BROWSER_INT:       return (long long) *((int*)mem);
    case BROWSER_LONG:      return (long long) *((long*)mem);
    case BROWSER_LONG_LONG: return (long long) *((long long*)mem);
    case BROWSER_FLOAT:     return (long long) *((float*)mem);
    case BROWSER_DOUBLE:    return (long long) *((double*)mem);
    }
}

/*-------------------------------------------------------------------------
 * Function:    prim_getval 
 *
 * Purpose:     Return value of primitive as double.
 *
 * Mark C. Miller, Mon Jan 11 16:10:26 PST 2010
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static double
prim_getval(int type, void *mem)
{
    switch (type) {
    case BROWSER_STR:       return 0;
    case BROWSER_INT8:      return (double) *((signed char*)mem);
    case BROWSER_SHORT:     return (double) *((short*)mem);
    case BROWSER_INT:       return (double) *((int*)mem);
    case BROWSER_LONG:      return (double) *((long*)mem);
    case BROWSER_LONG_LONG: return (double) *((long long*)mem);
    case BROWSER_FLOAT:     return (double) *((float*)mem);
    case BROWSER_DOUBLE:    return (double) *((double*)mem);
    }
}


/*-------------------------------------------------------------------------
 * Function:    prim_walk2
 *
 * Purpose:     Determines if the memory pointed to by a_mem and b_mem
 *              is the same thing.
 *
 * Return:      Success:
 *                 0: A and B are identical.
 *                 1: A and B are partially different, we printed the
 *                    summary already.
 *                 2: A and B are totally different, the caller should
 *                    print the summary.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 21 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 2 Sep 1997
 *      Added differencing for BROWSER_INT8 datatypes.
 *
 *      Mark C. Miller, Wed Sep 23 11:52:20 PDT 2009
 *      Added support for long long type
 *
 *  Mark C. Miller, Wed Nov 11 22:18:17 PST 2009
 *  Added suppot for alternate relative diff option using epsilon param.
 *
 *  Mark C. Miller, Sun Dec  6 16:01:01 PST 2009
 *  Added support for diffing values of different type. Added special
 *  diffing logic for diffing long long values as double mantissa is not
 *  long enough to store all possible long long values.
 *
 *  Mark C. Miller, Mon Dec  7 09:52:54 PST 2009
 *  Expand above mods to handle case where sizeof(long)>=sizeof(double).
 *
 *  Mark C. Miller, Mon Jan 11 16:11:11 PST 2010
 *  Split logic handling same types for a and b operands and logic handling
 *  case where a/b operands are of different type. For same type case,
 *  all diffing is done as before, in double precision, except long long
 *  which is done using long long. For differing types, all integral
 *  valued data is handled using long long and all float data is handled
 *  using double. If for the differing types one is float and the other
 *  is integral, then it will diff using double also. Added support
 *  for Diffopt.ll_xxx options also.
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
prim_walk2 (obj_t _a, void *a_mem, obj_t _b, void *b_mem, walk_t *wdata)
{
    obj_prim_t  *a = MYCLASS(_a);
    obj_prim_t  *b = MYCLASS(_b);
    char                *a_s=NULL, *b_s=NULL;
    int          status = 0;
    out_t       *f = wdata->f;

    if (a->browser_type == b->browser_type)
    {
        if (a->browser_type == BROWSER_STR)
        {
            a_s = *((char**)a_mem);
            b_s = *((char**)b_mem);

            if ((a_s && !b_s) || (!a_s && b_s)) {
                /* One is null, the other isn't */
                status = 2;
            } else if (a_s && b_s && strcmp(a_s, b_s)) {
                /* Non-null and different */
                status = 2;
            }
        }
        else if (a->browser_type == BROWSER_LONG_LONG)
        {
            long long a_ll = prim_getval_ll(a->browser_type, a_mem);
            long long b_ll = prim_getval_ll(b->browser_type, b_mem);
            double d_abs = DiffOpt.ll_abs;
            double d_rel = DiffOpt.ll_rel;
            double d_eps = DiffOpt.ll_eps;
            status = differentll(a_ll, b_ll, d_abs, d_rel, d_eps) ? 2 : 0;
        }
        else
        {
            double a_d = prim_getval(a->browser_type, a_mem);
            double b_d = prim_getval(b->browser_type, b_mem);
            double d_abs, d_rel, d_eps;

            switch (a->browser_type) {
            case BROWSER_INT8:
                d_abs = DiffOpt.c_abs;
                d_rel = DiffOpt.c_rel;
                d_eps = DiffOpt.c_eps;
                break;
            case BROWSER_SHORT:
                d_abs = DiffOpt.s_abs;
                d_rel = DiffOpt.s_rel;
                d_eps = DiffOpt.s_eps;
                break;
            case BROWSER_INT:
                d_abs = DiffOpt.i_abs;
                d_rel = DiffOpt.i_rel;
                d_eps = DiffOpt.i_eps;
                break;
            case BROWSER_LONG:
                d_abs = DiffOpt.l_abs;
                d_rel = DiffOpt.l_rel;
                d_eps = DiffOpt.l_eps;
                break;
            case BROWSER_FLOAT:
                d_abs = DiffOpt.f_abs;
                d_rel = DiffOpt.f_rel;
                d_eps = DiffOpt.f_eps;
                break;
            case BROWSER_DOUBLE:
                d_abs = DiffOpt.d_abs;
                d_rel = DiffOpt.d_rel;
                d_eps = DiffOpt.d_eps;
                break;
            default:
                abort();
            }

            status = different(a_d, b_d, d_abs, d_rel, d_eps) ? 2 : 0;
        }
    }
    else
    {
        if (a->browser_type == BROWSER_STR ||
            b->browser_type == BROWSER_STR)
        {
            /* different types but one is string. Can't handle that */
            status = 2;
        }
        else if ((a->browser_type == BROWSER_INT8 ||
                  a->browser_type == BROWSER_SHORT ||
                  a->browser_type == BROWSER_INT ||
                  a->browser_type == BROWSER_LONG ||
                  a->browser_type == BROWSER_LONG_LONG) &&
                 (b->browser_type == BROWSER_INT8 ||
                  b->browser_type == BROWSER_SHORT ||
                  b->browser_type == BROWSER_INT ||
                  b->browser_type == BROWSER_LONG ||
                  b->browser_type == BROWSER_LONG_LONG))
        {
            /* diff using largest integral type logic we can */
            long long a_ll = prim_getval_ll(a->browser_type, a_mem);
            long long b_ll = prim_getval_ll(b->browser_type, b_mem);
            double d_abs = DiffOpt.ll_abs;
            double d_rel = DiffOpt.ll_rel;
            double d_eps = DiffOpt.ll_eps;
            status = differentll(a_ll, b_ll, d_abs, d_rel, d_eps) ? 2 : 0;
        }
        else
        {
            /* diff using double precision logic */
            double a_d = prim_getval(a->browser_type, a_mem);
            double b_d = prim_getval(b->browser_type, b_mem);
            double d_abs = DiffOpt.d_abs;
            double d_rel = DiffOpt.d_rel;
            double d_eps = DiffOpt.d_eps;
            status = different(a_d, b_d, d_abs, d_rel, d_eps) ? 2 : 0;
        }
    }

    if (status) {
        switch (DiffOpt.report) {
        case DIFF_REP_ALL:
            if (DiffOpt.two_column) {
                obj_walk1(_a, a_mem, WALK_PRINT, wdata);
                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                obj_walk1(_b, b_mem, WALK_PRINT, wdata);
                out_nl(f);
                status = 1;
            }
            break;
        case DIFF_REP_BRIEF:
        case DIFF_REP_SUMMARY:
            break;
        }
    }
   
    return status;
}


/*-------------------------------------------------------------------------
 * Function:    prim_walk3
 *
 * Purpose:     Assigns a value to a silo primitive object.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Apr  2 1997
 *
 * Modifications:
 *
 *    Robb Matzke, 2 Sep 1997
 *    Added the BROWSER_INT8 datatype.
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static int
prim_walk3 (void *mem, obj_t _type, obj_t val) {

   char         *s, *tmp_s;
   obj_prim_t   *type = MYCLASS(_type);

   assert (type->tname);

   if (!strcmp (type->tname, "int8")) {
      if (!num_isint (val)) {
         out_error ("prim_walk3: cannot be assigned to an int8: ", val);
         return -1;
      }
      *((signed char*)mem) = num_int (val);

   } else if (!strcmp (type->tname, "short")) {
      if (!num_isint (val)) {
         out_error ("prim_walk3: cannot be assigned to a short: ", val);
         return -1;
      }
      *((short*)mem) = num_int (val);
         
   } else if (!strcmp (type->tname, "int")) {
      if (!num_isint (val)) {
         out_error ("prim_walk3: cannot be assigned to an int: ", val);
         return -1;
      }
      *((int*)mem) = num_int (val);

   } else if (!strcmp (type->tname, "long")) {
      if (!num_isint (val)) {
         out_error ("prim_walk3: cannot be assigned to a long: ", val);
         return -1;
      }
      *((long*)mem) = num_int (val);

   } else if (!strcmp (type->tname, "float")) {
      if (!num_isfp (val)) {
         out_error ("prim_walk3: cannot be assigned to a float: ", val);
         return -1;
      }
      *((float*)mem) = num_fp (val);

   } else if (!strcmp (type->tname, "double")) {
      if (!num_isfp (val)) {
         out_error ("prim_walk3: cannot be assigned to a double: ", val);
         return -1;
      }
      *((double*)mem) = num_fp (val);

   } else if (!strcmp (type->tname, "string")) {
      s = obj_name (val);
      if (!s) {
         out_error ("prim_walk3: cannot be assigned to a string: ", val);
         return -1;
      }
      tmp_s = *((char**)mem);
      if (tmp_s) free (tmp_s);
      *((char**)mem) = safe_strdup (s);

   } else {
      out_error ("sdo_assign: unknown primitive type: ", _type);
   }
   return 0;
}
         

/*-------------------------------------------------------------------------
 * Function:    prim_sizeof
 *
 * Purpose:     Returns the number of bytes represented by the primitive
 *              type.
 *
 * Return:      Success:        Number of bytes
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
prim_sizeof (obj_t _self) {

   if (NULL==MYCLASS(_self)->tname) return -1;
   return MYCLASS(_self)->nbytes;
}


/*-------------------------------------------------------------------------
 * Function:    prim_bind
 *
 * Purpose:     Given a primitive type name or variable containing a SILO
 *              primitive type integer, fill in additional information in
 *              the object to describe the type.
 *
 * Return:      Success:        SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  6 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 2 Sep 1997
 *      Due to the ambiguity of the words `char' and `character' they will
 *      no longer be accepted to mean `string'.  `String' is a pointer to
 *      an array of characters and the array is interpreted as a character
 *      string.  `Int8' is a character which is interpreted as an 8-bit
 *      signed integer.
 *
 *      Robb Matzke, 2 Sep 1997
 *      The silo datatype `DB_CHAR' is always translated to BROWSER_INT8
 *      because the only silo datatypes that use this feature want to
 *      interpret the values as integers.  Example:
 *
 *              mixvals: *[self.nvals] *[self.mixlen] self.datatype
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *    Mark C. Miller, Wed Sep 23 11:52:45 PDT 2009
 *    Added support for long long type.
 *
 *    Mark C. Miller, Fri Nov 13 15:38:07 PST 2009
 *    Changed name of "long long" type to "longlong" as PDB is sensitive
 *    to spaces in type names.
 *
 *    Mark C. Miller, Tue Nov 17 22:30:30 PST 2009
 *    Changed name of long long datatype to match PDB proper.
 *
 *    Mark C. Miller, Mon Dec  7 09:50:19 PST 2009
 *    Conditionally compile long long support only when its
 *    different from long.
 *
 *    Mark C. Miller, Mon Jan 11 16:02:16 PST 2010
 *    Made long long support UNconditionally compiled.
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static obj_t
prim_bind (obj_t _self, void *mem) {

   obj_prim_t   *self = MYCLASS(_self);
   obj_t        in=NIL, sdo=NIL;
   lex_t        *lex_input=NULL;
   walk_t       wdata;
   int          silo_type = -1;

   assert (self && C_PRIM==self->pub.cls);
   if (self->tname) {
      free (self->tname);
      self->tname = NULL;
   }

   if (!strcmp (self->name, "char") || !strcmp (self->name, "character")) {
      out_errorn ("prim_bind: do not use `string' or `int8' instead of %s",
                  self->name);
      return NIL;
      
   } else if (!strcmp (self->name, "string")) {
      self->tname = safe_strdup ("string");
      self->browser_type = BROWSER_STR;
      self->nbytes = sizeof(char*);

   } else if (!strcmp (self->name, "int8")) {
      self->tname = safe_strdup ("int8");
      self->browser_type = BROWSER_INT8;
      self->nbytes = sizeof(char);

   } else if (!strcmp (self->name, "short")) {
      self->tname = safe_strdup ("short");
      self->browser_type = BROWSER_SHORT;
      self->nbytes = sizeof(short);

   } else if (!strcmp (self->name, "int") ||
              !strcmp (self->name, "integer")) {
      self->tname = safe_strdup ("int");
      self->browser_type = BROWSER_INT;
      self->nbytes = sizeof(int);

   } else if (!strcmp (self->name, "long_long")) {
      self->tname = safe_strdup ("long_long");
      self->browser_type = BROWSER_LONG_LONG;
      self->nbytes = sizeof(long long);

   } else if (!strcmp (self->name, "long")) {
      self->tname = safe_strdup ("long");
      self->browser_type = BROWSER_LONG;
      self->nbytes = sizeof(long);

   } else if (!strcmp (self->name, "float")) {
      self->tname = safe_strdup ("float");
      self->browser_type = BROWSER_FLOAT;
      self->nbytes = sizeof(float);

   } else if (!strcmp (self->name, "double")) {
      self->tname = safe_strdup ("double");
      self->browser_type = BROWSER_DOUBLE;
      self->nbytes = sizeof(double);

   } else {
      if (isdigit(self->name[0])) {
         silo_type = strtol (self->name, NULL, 0);
      } else {
         lex_input = lex_string (self->name);
         in = parse_stmt (lex_input, false);
         lex_close (lex_input);
         sdo = obj_eval (in);
         in = obj_dest (in);
         if (!sdo || C_SDO!=sdo->pub.cls) {
            out_error ("prim_bind: data type is not appropriate: ", sdo);
            obj_dest (sdo);
            return NIL;
         }
         wdata.vals = &silo_type;
         wdata.nvals = 0;
         wdata.maxvals = 1;
         obj_walk1 (sdo, NULL, WALK_RETRIEVE, &wdata);
         obj_dest (sdo);
         if (1!=wdata.nvals) {
            out_errorn ("prim_bind: could not read data type from %s",
                        self->name);
            return NIL;
         }
      }
      
      switch (silo_type) {
      case DB_CHAR:
         self->tname = safe_strdup ("int8");
         self->browser_type = BROWSER_INT8;
         self->nbytes = sizeof(char);
         break;

      case DB_SHORT:
         self->tname = safe_strdup ("short");
         self->browser_type = BROWSER_SHORT;
         self->nbytes = sizeof(short);
         break;

      case DB_INT:
         self->tname = safe_strdup ("int");
         self->browser_type = BROWSER_INT;
         self->nbytes = sizeof(int);
         break;

      case DB_LONG:
         self->tname = safe_strdup ("long");
         self->browser_type = BROWSER_LONG;
         self->nbytes = sizeof(long);
         break;

      case DB_LONG_LONG:
         self->tname = safe_strdup ("long_long");
         self->browser_type = BROWSER_LONG_LONG;
         self->nbytes = sizeof(long long);
         break;

      case DB_FLOAT:
         self->tname = safe_strdup ("float");
         self->browser_type = BROWSER_FLOAT;
         self->nbytes = sizeof(float);
         break;

      case DB_DOUBLE:
         self->tname = safe_strdup ("double");
         self->browser_type = BROWSER_DOUBLE;
         self->nbytes = sizeof(double);
         break;

      default:
         out_errorn ("prim_bind: cannot resolve %s to a silo type",
                     self->name);
         return NIL;
      }
   }
   return _self;
}


/*-------------------------------------------------------------------------
 * Function:    prim_diff
 *
 * Purpose:     Computes the difference between two primitive types.
 *
 * Return:      Success:        0:      same
 *                              2:      different
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
prim_diff (obj_t _a, obj_t _b) {

   obj_prim_t   *a = MYCLASS(_a);
   obj_prim_t   *b = MYCLASS(_b);

   assert (a->tname);
   assert (b->tname);

   return (!strcmp(a->tname, b->tname) && a->assoc==b->assoc) ? 0 : 2;
}


/*-------------------------------------------------------------------------
 * Function:    prim_set_io_assoc
 *
 * Purpose:     Sets the I/O association table for a primitive type.
 *
 * Return:      Success:        SELF
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 13 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
obj_t
prim_set_io_assoc (obj_t _self, prim_assoc_t *assoc) {

   obj_prim_t   *self = MYCLASS(_self);

   if (!self || C_PRIM!=self->pub.cls) return NIL;
   self->assoc = assoc;
   return _self;
}


/*-------------------------------------------------------------------------
 * Function:    prim_silotype
 *
 * Purpose:     Returns the silo data type constant which corresponds to
 *              the primitive type.
 *
 * Return:      Success:        One of the DB_* type constants from silo.h
 *
 *              Failure:        DB_NOTYPE
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Apr  2 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 2 Sep 1997
 *      The browser type `string' is no longer translated to DB_CHAR.
 *      Instead, `int8' is translated to DB_CHAR.
 *
 *      Mark C. Miller, Wed Sep 23 11:53:21 PDT 2009
 *      Added support for long long.
 *
 *      Mark C. Miller, Fri Nov 13 15:38:07 PST 2009
 *      Changed name of "long long" type to "longlong" as PDB is sensitive
 * 
 *      Mark C. Miller, Tue Nov 17 22:30:30 PST 2009
 *      Changed name of long long datatype to match PDB proper.
 *-------------------------------------------------------------------------
 */
DBdatatype
prim_silotype (obj_t _self) {

   obj_prim_t   *self = MYCLASS(_self);

   if (!self) return DB_NOTYPE;
   if (self->tname) {
      switch (self->browser_type) {
      case BROWSER_STR:
         return DB_NOTYPE;
      case BROWSER_INT8:
         return DB_CHAR;
      case BROWSER_SHORT:
         return DB_SHORT;
      case BROWSER_INT:
         return DB_INT;
      case BROWSER_LONG:
         return DB_LONG;
      case BROWSER_LONG_LONG:
         return DB_LONG_LONG;
      case BROWSER_FLOAT:
         return DB_FLOAT;
      case BROWSER_DOUBLE:
         return DB_DOUBLE;
      default:
         return DB_NOTYPE;
      }
   } else if (!strcmp (self->name, "string")) {
      return DB_NOTYPE;

   } else if (!strcmp (self->name, "int8")) {
      return DB_CHAR;

   } else if (!strcmp (self->name, "short")) {
      return DB_SHORT;

   } else if (!strcmp (self->name, "int")) {
      return DB_INT;

   } else if (!strcmp (self->name, "long_long")) {
      return DB_LONG_LONG;

   } else if (!strcmp (self->name, "long")) {
      return DB_LONG;

   } else if (!strcmp (self->name, "float")) {
      return DB_FLOAT;

   } else if (!strcmp (self->name, "double")) {
      return DB_DOUBLE;

   }
   return DB_NOTYPE;
}

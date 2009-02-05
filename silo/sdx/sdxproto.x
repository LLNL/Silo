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

/*
 * sdxproto.x: Remote sdx protocol.
 */

const MAXSTRLEN = 256;

/*
 * Request and reply structures
 */

enum request_type {
   READVAR    = 0,
   CONTINUE   = 1,
   PAUSE      = 2,
   OPEN       = 3,
   CLOSE      = 4,
   ACCEPT     = 5,
   NEWCONTROL = 6,
   NEWDATA    = 7
};

enum reply_type {
   OK       = 0,
   ERROR    = 1
};

struct SDXreadvar {
   int       type;
   char      varname [MAXSTRLEN];
};

union request_info switch (request_type request) {
   case READVAR:
      SDXreadvar read;
   default:
      void;
};

struct SDXrequest {
   int       type;
   request_info  info;
};

union reply_info switch (reply_type reply) {
   case OK:
      void;
   case ERROR:
      int       errorno;
};

struct SDXreply {
   reply_info  info;
};

/*
 * The connect structure.
 */

enum connect_type {
   CONTROL_CONNECT = 0,
   OPEN_CONNECT    = 1,
   SERVER_CONNECT  = 2
};

struct SDXcontrol {
   char      username [MAXSTRLEN];
};

struct SDXopen {
   char      username [MAXSTRLEN];
   char      idstring [MAXSTRLEN];
};

struct SDXserver {
   char      username [MAXSTRLEN];
   char      idstring [MAXSTRLEN];
   int       nvar;
   char      varnames < >;
   char      meshnames < >;
   int       vartypes < >;
   int       nmats;
   int       nblocks;
};

union connect_info switch (connect_type connect) {
   case CONTROL_CONNECT:
      SDXcontrol control;
   case OPEN_CONNECT:
      SDXopen    open;
   case SERVER_CONNECT:
      SDXserver  server;
   default:
      void;
};

struct SDXconnect {
   int       type;
   connect_info info;
};

/*
 * The control info structure.
 */

struct SDXcontrolinfo {
   int       nservers;
   char      idstrings < >;
   int       nvars < >;
   char      varnames < >;
   int       vartypes < >;
   int       nmats < >;
   int       nblocks < >;
};

/*
 * The table of contents structure.
 */

struct SDXtoc {
   int       nvars;
   char      varnames < >;
   int       vartypes < >;
};

/*
 * The variable object.
 */

enum var_type {
   SDX_INTEGER = 0,
   SDX_FLOAT   = 1,
   SDX_DOUBLE  = 2,
   SDX_CHAR    = 3
};

union var_info switch (var_type var) {
   case SDX_INTEGER:
      int       ivalue;
   case SDX_FLOAT:
      float     fvalue;
   case SDX_DOUBLE:
      double    dvalue;
   case SDX_CHAR:
      char      cvalue [MAXSTRLEN];
   default:
      void;
};

struct SDXvar {
   var_type  type;
   var_info  info;
};

/*
 * The array object.
 */

union SDXarray switch (var_type type) {
   case SDX_INTEGER:
      int       iarray < >;
   case SDX_FLOAT:
      float     farray < >;
   case SDX_DOUBLE:
      double    darray < >;
   case SDX_CHAR:
      char      carray < >;
   default:
      void;
};

/*
 * The multi mesh object.
 */

struct SDXmultimesh {
   int       id;
   int       nblocks;
   int       meshids < >;
   char      meshnames < >;
   int       meshtypes < >;
   };

/*
 * The multi var object.
 *
 * Added by Brooke Unger, Jul 22, 1997
 */

struct SDXmultivar {
   int       id;
   int       nvars;
   char      varnames < >;
   int       vartypes < >;
   };

/*
 * The multi mat object.
 *
 * Added by Brooke Unger, Jul 22, 1997
 */

struct SDXmultimat {
   int       id;
   int       nmats;
   char      matnames < >;
   };

/*
 * The multi species object.
 *
 * Added by Jeremy Meredith, Sept 24, 1998
 */

struct SDXmultimatspecies {
   int       id;
   int       nspec;
   char      specnames < >;
   };

/*
 * The zone list structure from the ucd mesh object.
 */

struct SDXzonelist {
   int       ndims;
   int       nzones;
   int       nshapes;
   int       shapecnt < >;
   int       shapesize < >;
   int       nodelist < >;
   int       lnodelist;
   int       origin;
   };

/*
 * The face list structure from the ucd mesh object.
 */

struct SDXfacelist {
   int       ndims;
   int       nfaces;
   int       origin;
   int       nodelist < >;
   int       lnodelist;

   int       nshapes;
   int       shapecnt < >;
   int       shapesize < >;

   int       ntypes;
   int       typelist < >;
   int       types < >;

   int       zoneno < >;
   };

/*
 * The edge list structure from the ucd mesh object.
 */

struct SDXedgelist {
   int       ndims;
   int       nedges;
   int       edge_beg < >;
   int       edge_end < >;
   int       origin;
   };

/*
 * The quad mesh object.
 */

struct SDXquadmesh {
   int       id;
   int       block_no;
   char      name [MAXSTRLEN];
   int       cycle;
   float     time;
   int       coord_sys;
   int       major_order;
   int       stride [3];
   int       coordtype;
   int       facetype;
   int       planar;

   SDXarray  xcoords;
   SDXarray  ycoords;
   SDXarray  zcoords;
   int       datatype;
   float     min_extents [3];
   float     max_extents [3];
   char      xlabel [MAXSTRLEN];
   char      ylabel [MAXSTRLEN];
   char      zlabel [MAXSTRLEN];
   char      xunits [MAXSTRLEN];
   char      yunits [MAXSTRLEN];
   char      zunits [MAXSTRLEN];
   int       ndims;
   int       nspace;
   int       nnodes;

   int       dims [3];
   int       origin;
   int       min_index [3];
   int       max_index [3];
   };

/*
 * The ucd mesh object.
 */

struct SDXucdmesh {
   int       id;
   int       block_no;
   char      name [MAXSTRLEN];
   int       cycle;
   float     time;
   int       coord_sys;
   char      xunits [MAXSTRLEN];
   char      yunits [MAXSTRLEN];
   char      zunits [MAXSTRLEN];
   char      xlabel [MAXSTRLEN];
   char      ylabel [MAXSTRLEN];
   char      zlabel [MAXSTRLEN];

   float     xcoords < >;
   float     ycoords < >;
   float     zcoords < >;
   int       datatype;
   float     min_extents [3];
   float     max_extents [3];
   int       ndims;
   int       nnodes;
   int       origin;
   };

/*
 * The quad variable object.
 */

struct SDXquadvar {
   int       id;
   char      name [MAXSTRLEN];
   char      units [MAXSTRLEN];
   char      label [MAXSTRLEN];
   int       cycle;
   float     time;
   int       meshid;

   SDXarray  vals;
   int       datatype;
   int       nels;
   int       ndims;
   int       dims [3];

   int       major_order;
   int       stride [3];
   int       min_index [3];
   int       max_index [3];
   int       origin;
   float     align [3];
   };

/*
 * The ucd variable object.
 */

struct SDXucdvar {
   int       id;
   char      name [MAXSTRLEN];
   int       cycle;
   char      units [MAXSTRLEN];
   char      label [MAXSTRLEN];
   float     time;
   int       meshid;
   float     vals < >;
   int       datatype;
   int       nels;
   int       ndims;
   int       origin;
   int       centering;
   };

/*
 * The material object.
 */

struct SDXmaterial {
   int       id;
   char      name [MAXSTRLEN];
   int       ndims;
   int       origin;
   int       dims [3];
   int       major_order;
   float     stride [3];
   int       nmat;
   int       matnos < >;
   int       matlist < >;
   int       mixlen;
   int       datatype;
   SDXarray  mix_vf;
   int       mix_next < >;
   int       mix_mat < >;
   int       mix_zone < >;
   };

/*
 * The mesh with data information object.
 */

struct SDXMWDInfo {
   int       nqmesh;
   int       nqvar < >;
   int       numesh;
   int       nuvar < >;
   float     min_extents [3];
   float     max_extents [3];
   int       nspace;
   int       cycle;
   float     time;
   char      name [MAXSTRLEN];
};

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

#include <stdio.h>
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
    int            i, j, k, l;
    int            nservers;
    char          *idstrings;
    int           *nvars;
    char          *varnames;
    int           *vartypes;
    int           *nmats;
    int           *nblocks;
    char           varname[20];
    int            socket_control;

    socket_control = SDXOpenControl(&nservers, &idstrings, &nvars,
                                 &varnames, &vartypes, &nmats, &nblocks);

    fprintf(stdout, "nservers = %d\n", nservers);
    k = 0;
    for (i = 0; i < nservers; i++) {
        fprintf(stdout, "server %d:\n", i);
        strncpy(varname, &idstrings[i], SDX_LEN);
        fprintf(stdout, "   id       = '%s'\n", varname);
        fprintf(stdout, "   nvars    = %d\n", nvars[i]);
        fprintf(stdout, "   nmats    = %d\n", nmats[i]);
        fprintf(stdout, "   nblocks  = %d\n", nblocks[i]);
        fprintf(stdout, "   varnames         vartypes\n");
        fprintf(stdout, "-------------------------------\n");
        for (j = 0; j < nvars[i]; j++) {
            strncpy(varname, &varnames[(k + j) * SDX_LEN], SDX_LEN);
            for (l = 0; l < SDX_LEN && varname[l] != ' '; l++)
                /* do nothing */ ;
            varname[l] = '\0';
            fprintf(stdout, "   %-SDX_LENs %d\n", varname, vartypes[k + j]);
        }
        k += nvars[i];
    }

    FREE(idstrings);
    FREE(nvars);
    FREE(varnames);
    FREE(vartypes);

    SDXCloseControl();
    return 0;
}

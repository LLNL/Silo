#include <silo.h>
#include <stdio.h>
#include <stdlib.h>

void
add_edge(int nid1, int nid2, int *nedges, int *maxedges, int **edges)
{
    if (*nedges == *maxedges)
    {
        *maxedges = (*maxedges) * 2 + 1;
        *edges = (int *) realloc(*edges, *maxedges * 2 * sizeof(int));
    }
    (*edges)[2*(*nedges)+0] = nid1;
    (*edges)[2*(*nedges)+1] = nid2;
    *nedges = (*nedges) + 1;
}

int
have_edge(int nid1, int nid2, int nedges, const int *edges)
{
    int i;
    for (i = 0; i < nedges; i++)
    {
        if ((edges[2*i+0] == nid1 &&
             edges[2*i+1] == nid2) ||
            (edges[2*i+0] == nid2 &&
             edges[2*i+1] == nid1))
            return 1;
    }
    return 0;
}

#define HANDLE_EDGE(A,B)						\
    if (!have_edge(nodelist[nlidx+A],nodelist[nlidx+B],*nedges,*edges))	\
        add_edge(nodelist[nlidx+A],nodelist[nlidx+B],nedges,&maxedges,edges)

void
build_edgelist(int nzones, int ndims, const int *nodelist,
    int lnodelist, int origin, int lo_off, int hi_off,
    const int *st, const int *sz, const int *sc, int nshapes,
    int *nedges, int **edges)
{
    int nlidx = 0;
    int maxedges = 0;
    int shape;

    *nedges = 0;
    *edges = 0;
    for (shape = 0; shape < nshapes; shape++)
    {
        int zonetype = st[shape];
        int zonesize = sz[shape];
        int zone;
        for (zone = 0; zone < sc[shape]; zone++)
        {
            switch (zonetype)
            {
                case DB_ZONETYPE_HEX:
                {
                    HANDLE_EDGE(0,1);
                    HANDLE_EDGE(0,3);
                    HANDLE_EDGE(0,4);
                    HANDLE_EDGE(1,2);
                    HANDLE_EDGE(1,5);
                    HANDLE_EDGE(2,3);
                    HANDLE_EDGE(2,6);
                    HANDLE_EDGE(3,7);
                    HANDLE_EDGE(4,5);
                    HANDLE_EDGE(4,7);
                    HANDLE_EDGE(5,6);
                    HANDLE_EDGE(6,7);
                    break;
                }
            }
            nlidx += zonesize;
        }
    }
}

int compare_nodes(const void *nap, const void *nbp)
{
    int na = *((int*)nap);
    int nb = *((int*)nbp);
    if (na < nb) return -1;
    else if (na > nb) return 1;
    return 0;
}

void
add_face(int nid1, int nid2, int nid3, int nid4, int *nfaces, int *maxfaces, int **faces)
{
    int i;
    int fnid[4];
    fnid[0] = nid1; 
    fnid[1] = nid2; 
    fnid[2] = nid3; 
    fnid[3] = nid4; 
    qsort(fnid, sizeof(int), 4, compare_nodes);
    if (*nfaces == *maxfaces)
    {
        *maxfaces = (*maxfaces) * 2 + 1;
        *faces = (int *) realloc(*faces, *maxfaces * 4 * sizeof(int));
    }
    for (i = 0; i < 4; i++)
        (*faces)[4*(*nfaces)+i] = fnid[i];
    *nfaces = (*nfaces) + 1;
}

int
have_face(int nid1, int nid2, int nid3, int nid4, int nfaces, const int *faces)
{
    int i,j;
    int fnid[4];
    fnid[0] = nid1; 
    fnid[1] = nid2; 
    fnid[2] = nid3; 
    fnid[3] = nid4; 
    qsort(fnid, sizeof(int), 4, compare_nodes);
    for (i = 0; i < nfaces; i++)
    {
        int allmatch = 1;
        for (j = 0; j < 4; j++)
        {
            if (faces[4*i+j] != fnid[j])
            {
                allmatch = 0;
                break;
            }
        }
        if (allmatch) return 1;
    }
    return 0;
}

#define HANDLE_FACE(A,B,C,D)						\
    if (!have_face(nodelist[nlidx+A],nodelist[nlidx+B],			\
                   nodelist[nlidx+C],nodelist[nlidx+D],*nfaces,*faces))	\
        add_face(nodelist[nlidx+A],nodelist[nlidx+B],			\
                 nodelist[nlidx+C],nodelist[nlidx+D],nfaces,&maxfaces,faces)

void
build_facelist(int nzones, int ndims, const int *nodelist,
    int lnodelist, int origin, int lo_off, int hi_off,
    const int *st, const int *sz, const int *sc, int nshapes,
    int *nfaces, int **faces)
{
    int nlidx = 0;
    int maxfaces = 0;
    int shape;

    *nfaces = 0;
    *faces = 0;
    for (shape = 0; shape < nshapes; shape++)
    {
        int zonetype = st[shape];
        int zonesize = sz[shape];
        int zone;
        for (zone = 0; zone < sc[shape]; zone++)
        {
            switch (zonetype)
            {
                case DB_ZONETYPE_HEX:
                {
                    HANDLE_FACE(0,1,5,4);
                    HANDLE_FACE(0,3,2,1);
                    HANDLE_FACE(0,4,7,3);
                    HANDLE_FACE(1,2,6,5);
                    HANDLE_FACE(2,3,7,6);
                    HANDLE_FACE(4,5,6,7);
                    break;
                }
            }
            nlidx += zonesize;
        }
    }
}

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    char           *coordnames[3];
    float          *coords[3];
    float           x[64], y[64], z[64];
    int             shapesize[1];
    int             shapecnt[1];
    DBfacelist     *facelist = NULL;
    int             matnos[1], matlist[1], dims[3];
    int             i, j, k, len;
    float           evar2d[2*16], evar3d[3*64], fvar3d[3*64];
    int		    driver = DB_PDB;
    char 	    *filename = "efcentering.silo";
    int             layer, zone;
    int             nodelist2[9*4] = {0,1,5,4,    1,2,6,5,    2,3,7,6,
                                      4,5,9,8,    5,6,10,9,   6,7,11,10,
                                      8,9,13,12,  9,10,14,13, 10,11,15,14};
    int st2 = DB_ZONETYPE_QUAD;
    int ss2 = 4;
    int sc2 = 9;
    int nodelist3[27*8];
    int st3 = DB_ZONETYPE_HEX;
    int ss3 = 8;
    int sc3 = 27;

    int nedges;
    int *edges;
    int nfaces;
    int *faces;
    int ndims;

    /* Parse command-line */
    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    
    DBShowErrors(DB_ABORT, NULL);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "edge and face centered data", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    dims[0] = 4;
    dims[1] = 4;
    dims[2] = 4;
    for (k = 0; k < 4; k++)
    {
        for (j = 0; j < 4; j++)
        {
            for (i = 0; i < 4; i++)
            {
                x[k*4*4+j*4+i] = (float) i;
                y[k*4*4+j*4+i] = (float) j;
                z[k*4*4+j*4+i] = (float) k;
                evar2d[0*16+j*4+i] = (float) i;
                evar2d[1*16+j*4+i] = (float) j;
                evar3d[0*64+k*4*4+j*4+i] = (float) i;
                evar3d[1*64+k*4*4+j*4+i] = (float) j;
                evar3d[2*64+k*4*4+j*4+i] = (float) k;
                fvar3d[0*64+k*4*4+j*4+i] = (float) 10*i;
                fvar3d[1*64+k*4*4+j*4+i] = (float) 100*j;
                fvar3d[2*64+k*4*4+j*4+i] = (float) 1000*k;
            }
        }
    }

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    /* build 3d zonelist by layering 2d zonelist */
    for (layer = 0; layer < 3; layer++)
    {
        for (zone = 0; zone < 9; zone++)
        {
            nodelist3[layer*9*8+zone*8+0] = nodelist2[zone*4+0]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+1] = nodelist2[zone*4+0]+layer*16;
            nodelist3[layer*9*8+zone*8+2] = nodelist2[zone*4+1]+layer*16;
            nodelist3[layer*9*8+zone*8+3] = nodelist2[zone*4+1]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+4] = nodelist2[zone*4+3]+(layer+1)*16;
            nodelist3[layer*9*8+zone*8+5] = nodelist2[zone*4+3]+layer*16;
            nodelist3[layer*9*8+zone*8+6] = nodelist2[zone*4+2]+layer*16;
            nodelist3[layer*9*8+zone*8+7] = nodelist2[zone*4+2]+(layer+1)*16;
        }
    }

    DBPutQuadmesh(dbfile, "qmesh2", coordnames, coords, dims, 2, DB_FLOAT, DB_NONCOLLINEAR, 0);
    DBPutQuadmesh(dbfile, "qmesh3", coordnames, coords, dims, 3, DB_FLOAT, DB_NONCOLLINEAR, 0);
    DBPutQuadvar1(dbfile, "qevar2", "qmesh2", evar2d, dims, 2, 0, 0, DB_FLOAT, DB_EDGECENT, 0);
    DBPutQuadvar1(dbfile, "qevar3", "qmesh3", evar3d, dims, 3, 0, 0, DB_FLOAT, DB_EDGECENT, 0);
    DBPutQuadvar1(dbfile, "qfvar3", "qmesh3", fvar3d, dims, 3, 0, 0, DB_FLOAT, DB_FACECENT, 0);

    DBPutUcdmesh(dbfile, "umesh2", 2, coordnames, coords, 16, 9,  "um2zl", 0, DB_FLOAT, 0);
    DBPutUcdmesh(dbfile, "umesh3", 3, coordnames, coords, 64, 27, "um3zl", 0, DB_FLOAT, 0);
    DBPutZonelist2(dbfile, "um2zl", 9, 2, nodelist2, ss2*sc2, 0, 0, 0, &st2, &ss2, &sc2, 1, 0);
    DBPutZonelist2(dbfile, "um3zl", 27, 3, nodelist3, ss3*sc3, 0, 0, 0, &st3, &ss3, &sc3, 1, 0);

    /* Only reason we build an edgelist is so we know the number of unique edges in the mesh */
    build_edgelist(27, 3, nodelist3, ss3*sc3, 0, 0, 0, &st3, &ss3, &sc3, 1, &nedges, &edges);
    for (i = 0; i < nedges; i++)
        evar3d[i] = i;
    DBPutUcdvar1(dbfile, "uevar3", "umesh3", evar3d, nedges, 0, 0, DB_FLOAT, DB_EDGECENT, 0);
    ndims = 2;
    dims[0] = nedges;
    dims[1] = 2;
    DBWrite(dbfile, "edges", edges, dims, ndims, DB_INT);
    free(edges);

    /* Only reason we build a facelist is so we know the number of unique faces in the mesh */
    build_facelist(27, 3, nodelist3, ss3*sc3, 0, 0, 0, &st3, &ss3, &sc3, 1, &nfaces, &faces);
    for (i = 0; i < nfaces; i++)
        fvar3d[i] = i;
    DBPutUcdvar1(dbfile, "ufvar3", "umesh3", fvar3d, nfaces, 0, 0, DB_FLOAT, DB_FACECENT, 0);
    dims[0] = nfaces;
    dims[1] = 4;
    DBWrite(dbfile, "faces", faces, dims, ndims, DB_INT);
    free(faces);

    DBClose(dbfile);

    return (0);
}

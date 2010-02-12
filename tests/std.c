#include <stdio.h>
#include <string.h>

static int StringToDriver(const char *str)
{
    if (!strcmp(str, "DB_PDB"))
        return DB_PDB;
    else if (!strcmp(str, "DB_HDF5"))
        return DB_HDF5;
    else if (!strcmp(str, "DB_HDF5_SEC2"))
        return DB_HDF5_SEC2;
    else if (!strcmp(str, "DB_HDF5_STDIO"))
        return DB_HDF5_STDIO;
    else if (!strcmp(str, "DB_HDF5_MPIO"))
        return DB_HDF5_MPIO;
    else if (!strcmp(str, "DB_HDF5_MPIOP"))
        return DB_HDF5_MPIOP;
    else if (!strcmp(str, "DB_H5VFD_CORE"))
        return DB_H5VFD_CORE;
    else if (!strcmp(str, "DB_H5VFD_SEC2"))
        return DB_H5VFD_SEC2;
    else if (!strcmp(str, "DB_H5VFD_STDIO"))
        return DB_H5VFD_STDIO;
    else if (!strncmp(str, "DB_HDF5_CORE(", 13))
    {
        int inc;
        sscanf(str, "DB_HDF5_CORE(%d)", &inc);
        return DB_HDF5_CORE(inc);
    }
    else if (!strncmp(str, "DB_HDF5_SPLIT(", 14))
    {
        char mvfds[64], rvfds[64];
        int mvfd, rvfd, minc, rinc, extpair;
        sscanf(str, "DB_HDF5_SPLIT(%s %d %s %d %d)", mvfds, &minc, rvfds, &rinc, &extpair);
        mvfd = StringToDriver(mvfds);
        rvfd = StringToDriver(rvfds);
        return DB_HDF5_SPLIT(mvfd,minc,rvfd,rinc,extpair);
    }

    fprintf(stderr, "Unable to determine driver from string \"%s\"\n", str);
    exit(-1);
}

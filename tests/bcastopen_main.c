#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif
#include <silo.h>

extern void PrintFileComponentTypes(DBfile *dbfile, FILE* outf);
#ifdef HAVE_MPI
extern DBfile *DBOpenByBcast(char const *, MPI_Comm, int);
#else
extern DBfile *DBOpenByBcast(char const *, int, int);
#endif

int main(int argc, char **argv)
{
    int i;
    int rank = 0;

    if (argc < 2)
    {
        printf("Usage: listtypes [options] filename [filename ...]\n");
        exit(0);
    }

#ifdef HAVE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    DBShowErrors(DB_NONE, NULL);

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "show-all-errors"))
            DBShowErrors (DB_ALL_AND_DRVR, NULL);
        else
        {
            DBfile *dbfile;
            char outfile[256];
            FILE* outf;

#ifdef HAVE_MPI
            dbfile = DBOpenByBcast(argv[i], MPI_COMM_WORLD, 0);
#else
            dbfile = DBOpenByBcast(argv[i], 0, 0);
#endif
            snprintf(outfile, sizeof(outfile), "%s-%05d-typelist.txt", argv[i], rank);
            outf = fopen(outfile, "w");
            fprintf(outf, "File: %s\n", argv[i]);
            PrintFileComponentTypes(dbfile, outf);
            DBClose(dbfile);
        }
    }

    if (argc > 1 && rank == 0)
        printf("Examine results in files with names of the form \"%s-%05d-typelist.txt\"\n", argv[1], 0);

#ifdef HAVE_MPI
    MPI_Finalize();
#endif

    return 0;
}

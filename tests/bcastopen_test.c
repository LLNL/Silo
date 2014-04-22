#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <silo.h>

extern void PrintFileComponentTypes(DBfile *dbfile, FILE* outf);
extern DBfile *DBOpenByBcast(char const *, MPI_Comm, int);

int main(int argc, char **argv)
{
    int i;
    int rank;

    if (argc < 2)
    {
        printf("Usage: listtypes [options] filename [filename ...]\n");
        exit(0);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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

            dbfile = DBOpenByBcast(argv[i], MPI_COMM_WORLD, 0);
            snprintf(outfile, sizeof(outfile), "%s-%05d-typelist.txt", argv[i], rank);
            outf = fopen(outfile, "w");
            fprintf(outf, "File: %s\n", argv[i]);
            PrintFileComponentTypes(dbfile, outf);
            DBClose(dbfile);
        }
    }

    MPI_Finalize();

    return 0;
}

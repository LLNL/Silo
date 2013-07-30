int main(int argc, char **argv)
{
    int i;
    int rank;
    DBfile *dbfile;

    if (argc < 2)
    {
        printf("Usage: listtypes [options] filename [filename ...]\n");
        exit(0);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    DBShowErrors (DB_NONE, NULL);

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "show-all-errors"))
            DBShowErrors (DB_ALL_AND_DRVR, NULL);
        else
        {
            dbfile = DBOpenByBcast(argv[i], MPI_COMM_WORLD, 0);
            DBtoc = DBGetToc(dbfile);
        }
    }

    return 0;
}

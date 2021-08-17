#include <silo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>
#ifdef HAVE_HDF5_H
#include <hdf5.h>
#define HDF5_VERSION_GE(Maj,Min,Rel)  \
        (((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR==Min) && (H5_VERS_RELEASE>=Rel)) || \
         ((H5_VERS_MAJOR==Maj) && (H5_VERS_MINOR>Min)) || \
         (H5_VERS_MAJOR>Maj))
#endif

extern void PrintFileComponentTypes(DBfile *dbfile, FILE* outf);

static void
ProcessSiloFile(char const *filename, int test_fic_vfd)
{
    DBfile *dbfile;

    if (test_fic_vfd)
    {
#ifdef HAVE_HDF5_H
#if HDF5_VERSION_GE(1,8,9)
        hid_t fapl, fid;
        int file_len;
        ssize_t read_len;
        void *file_buf;
        DBoptlist *file_optlist;
        int fic_optset;
        int fic_vfd;

        /* Open the file using default (sec2) and get file image from it */
        fid = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
        file_len = (int) H5Fget_file_image(fid, NULL, (size_t)0);
        file_buf = malloc((size_t)file_len);
        read_len = H5Fget_file_image(fid, file_buf, (size_t)file_len);
        H5Fclose(fid);

        /* now, try to open the above buffer as a 'file' */
        file_optlist = DBMakeOptlist(10);
        fic_vfd = DB_H5VFD_FIC;
        DBAddOption(file_optlist, DBOPT_H5_VFD, &fic_vfd);
        DBAddOption(file_optlist, DBOPT_H5_FIC_SIZE, &file_len);
        DBAddOption(file_optlist, DBOPT_H5_FIC_BUF, file_buf);
        fic_optset = DBRegisterFileOptionsSet(file_optlist);

        /* Ok, now test silo opening this 'buffer' */
        if((dbfile = DBOpen("dummy", DB_HDF5_OPTS(fic_optset), DB_READ)) == NULL)
        {
            fprintf(stderr, "File: %s\n    <could not be opened>\n\n", "dummy");
            return;
        }

        /* free up the file options set */
        DBUnregisterFileOptionsSet(fic_optset);
        DBFreeOptlist(file_optlist);

#endif
#else

        fprintf(stderr, "Cannot test FIC vfd without HDF5 library\n");
        exit(-1);

#endif

    }
    else
    {
        /* Open the data file. Return if it cannot be read. */
        if((dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ)) == NULL)
        {
            fprintf(stderr, "File: %s\n    <could not be opened>\n\n", filename);
            return;
        }
    }

    fprintf(stdout, "File: %s\n", filename);

    PrintFileComponentTypes(dbfile, stdout);

    DBClose(dbfile);
}

/*********************************************************************
 *
 * Purpose: Main function for listtypes.c. This function iterates
 *          over the command line arguments and supplies them to
 *          a function that prints the component types for a file.
 *          This program tests the DBGetComponentType function.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:05:37 PST 2000
 *
 * Input Arguments:
 *     argc : The number of command line arguments.
 *     argv : An array containing the command line arguments.
 *
 * Modifications:
 *     Thomas R. Treadway, Thu Jul  5 16:33:38 PDT 2007
 *     Chaneged main's return type to int, to stop gcc-4.x whining.
 *
 ********************************************************************/

int
main(int argc, char *argv[])
{
    int i;
    int test_fic_vfd = 0;
    int show_all_errors = 0;

    if (argc < 2)
    {
        printf("Usage: listtypes filename [filename ...]\n");
        return 0;
    }

    DBShowErrors(DB_NONE, NULL);

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "show-all-errors"))
            DBShowErrors(DB_ALL_AND_DRVR, NULL);
        else if (!strcmp(argv[i], "test-fic-vfd"))
            test_fic_vfd = 1;
        else
        {
            ProcessSiloFile(argv[i], test_fic_vfd);
        }
    }
    
    return 0;
}

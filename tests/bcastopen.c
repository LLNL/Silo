#include <stdlib.h>
#include <mpi.h>
#include <hdf5.h>
#include <silo.h>

DBfile *DBOpenByBcast(char const *filename, MPI_Comm comm, int rank_of_root)
{
    int rank;
    int file_len;
    void *file_buf;
    DBfile *retval = 0;
    DBoptlist *file_optlist;
    int fic_vfd;
    int fic_optset;

    MPI_Comm_rank(comm, &rank);

    if (rank == rank_of_root)
    {
        hid_t fapl, fid;
        ssize_t read_len;

        /* Just open the file at the HDF5 level
           using default (sec2) and get file image from it */
        /* Could just use stat/open to read the whole file too? */
        fid = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
        file_len = (int) H5Fget_file_image(fid, NULL, (size_t)0);
        file_buf = malloc((size_t)file_len);
        read_len = H5Fget_file_image(fid, file_buf, (size_t)file_len);
        H5Fclose(fid);

        /* bcast the size */
        MPI_Bcast(&file_len, 1, MPI_INT, rank_of_root, comm);

        /* bcast the buffer */
        MPI_Bcast(file_buf, file_len, MPI_BYTE, rank_of_root, comm);
    }
    else
    {
        /* recv the size */
        MPI_Bcast(&file_len, 1, MPI_INT, rank_of_root, comm);

        /* recv the buffer */
        file_buf = (void *) malloc(file_len);
        MPI_Bcast(file_buf, file_len, MPI_BYTE, rank_of_root, comm);
    }

    /* Set up a file options set to use this buffer as the file */
    file_optlist = DBMakeOptlist(10);
    fic_vfd = DB_H5VFD_FIC;
    DBAddOption(file_optlist, DBOPT_H5_VFD, &fic_vfd);
    DBAddOption(file_optlist, DBOPT_H5_FIC_SIZE, &file_len);
    DBAddOption(file_optlist, DBOPT_H5_FIC_BUF, file_buf);
    fic_optset = DBRegisterFileOptionsSet(file_optlist);
        
    /* Ok, now have Silo open this buffer */
    retval = DBOpen("dummy", DB_HDF5_OPTS(fic_optset), DB_READ);
    
    /* free up the file options set */
    DBUnregisterFileOptionsSet(fic_optset);
    DBFreeOptlist(file_optlist);

    return retval;
}

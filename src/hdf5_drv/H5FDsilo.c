/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Programmer:	Robb Matzke <matzke@llnl.gov>
 *	      	Wednesday, October 22, 1997
 *
 * Purpose:   	This is the Posix stdio.h I/O subclass of H5Flow.
 *		It also serves as an example of coding a simple file driver,
 *		therefore, it should not use any non-public definitions.
 *
 * Notes:  Ported to the new H5FD architecture on 10/18/99 - QAK
 *
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Disable certain warnings in PC-Lint: */
/*lint --emacro( {534, 830}, H5P_FILE_ACCESS) */
/*lint --emacro( {534, 830}, H5F_ACC_RDWR, H5F_ACC_EXCL) */
/*lint -esym( 534, H5Eclear2, H5Epush2) */

#include "hdf5.h"
#include "H5FDsilo.h"

#ifdef H5_HAVE_STDIO_H
#include <stdio.h> /* for snprintf */
#endif
#ifdef H5_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

#ifdef MAX
#undef MAX
#endif /* MAX */
#define MAX(X,Y)	((X)>(Y)?(X):(Y))

/* File operations */
#define OP_UNKNOWN      0
#define OP_READ         1
#define OP_WRITE        2

/* definitions related to the file stat utilities.
 * For Unix, if off_t is not 64bit big, try use the pseudo-standard
 * xxx64 versions if available.
 */
#if !defined(HDfstat) || !defined(HDstat)
    #if H5_SIZEOF_OFF_T!=8 && H5_SIZEOF_OFF64_T==8 && defined(H5_HAVE_STAT64)
        #ifndef HDfstat
            #define HDfstat(F,B)        fstat64(F,B)
        #endif /* HDfstat */
        #ifndef HDstat
            #define HDstat(S,B)         stat64(S,B)
        #endif /* HDstat */
        typedef struct stat64       h5_stat_t;
        typedef off64_t             h5_stat_size_t;
        #define H5_SIZEOF_H5_STAT_SIZE_T H5_SIZEOF_OFF64_T
    #else /* H5_SIZEOF_OFF_T!=8 && ... */
        #ifndef HDfstat
            #define HDfstat(F,B)        fstat(F,B)
        #endif /* HDfstat */
        #ifndef HDstat
            #define HDstat(S,B)         stat(S,B)
        #endif /* HDstat */
        typedef struct stat         h5_stat_t;
        typedef off_t               h5_stat_size_t;
        #define H5_SIZEOF_H5_STAT_SIZE_T H5_SIZEOF_OFF_T
    #endif /* H5_SIZEOF_OFF_T!=8 && ... */
#endif /* !defined(HDfstat) || !defined(HDstat) */
#ifndef HDlseek
    #ifdef H5_HAVE_LSEEK64
       #define HDlseek(F,O,W)   lseek64(F,O,W)
    #else
       #define HDlseek(F,O,W)   lseek(F,O,W)
    #endif
#endif /* HDlseek */
#ifndef HDassert
    #define HDassert(X)         assert(X)
#endif /* HDassert */
#ifndef HDopen
    #ifdef _O_BINARY
        #define HDopen(S,F,M)           open(S,F|_O_BINARY,M)
    #else
        #define HDopen(S,F,M)           open(S,F,M)
    #endif
#endif /* HDopen */
#ifndef HDread
    #define HDread(F,M,Z)               read(F,M,Z)
#endif /* HDread */
#ifndef HDwrite
    #define HDwrite(F,M,Z)              write(F,M,Z)
#endif /* HDwrite */
#ifndef HDftruncate
  #ifdef H5_HAVE_FTRUNCATE64
    #define HDftruncate(F,L)        ftruncate64(F,L)
  #else
    #define HDftruncate(F,L)        ftruncate(F,L)
  #endif
#endif /* HDftruncate */
#ifndef HDmemset
    #define HDmemset(X,C,Z)             memset(X,C,Z)
#endif /* HDmemset */
#define H5F_addr_eq(X,Y)        ((X)!=HADDR_UNDEF &&                          \
                                 (X)==(Y))


#ifdef H5_HAVE_SNPRINTF
#define H5E_PUSH_HELPER(Func,Cls,Maj,Min,Msg,Ret,Errno)			\
{									\
    char msg[256] = Msg;						\
    if (Errno != 0)							\        
        snprintf(msg, (int) sizeof(msg), Msg "(errno=%d, \"%s\")",	\
            Errno, strerror(Errno));					\
    H5Epush_ret(Func, Cls, Maj, Min, msg, Ret)				\
}
#else
#define H5E_PUSH_HELPER(Func,Cls,Maj,Min,Msg,Ret,Errno)			\
{									\
    H5Epush_ret(Func, Cls, Maj, Min, msg, Ret)				\
}
#endif



/* The driver identification number, initialized at runtime */
static hid_t H5FD_SILO_g = 0;

/*
 * The description of a file belonging to this driver. The `eoa' and `eof'
 * determine the amount of hdf5 address space in use and the high-water mark
 * of the file (the current size of the underlying Unix file). The `pos'
 * value is used to eliminate file position updates when they would be a
 * no-op. Unfortunately we've found systems that use separate file position
 * indicators for reading and writing so the lseek can only be eliminated if
 * the current operation is the same as the previous operation.  When opening
 * a file the `eof' will be set to the current file size, `eoa' will be set
 * to zero, `pos' will be set to H5F_ADDR_UNDEF (as it is when an error
 * occurs), and `op' will be set to H5F_OP_UNKNOWN.
 */
typedef struct H5FD_silo_t {
    H5FD_t	pub;			/*public stuff, must be first	*/
    int         fd;			/* file descriptor */
    haddr_t	eoa;			/*end of allocated region	*/
    haddr_t	eof;			/*end of file; current file size*/
    haddr_t	pos;			/*current file I/O position	*/
    int         op;	/*last operation		*/
    unsigned    write_access;  /* Flag to indicate the file was opened with write access */
#ifndef _WIN32
    /*
     * On most systems the combination of device and i-node number uniquely
     * identify a file.
     */
    dev_t	device;			/*file device number		*/
#ifdef H5_VMS
    ino_t       inode[3];               /*file i-node number            */
#else
    ino_t       inode;                  /*file i-node number            */
#endif /*H5_VMS*/
#else
    /*
     * On _WIN32 the low-order word of a unique identifier associated with the
     * file and the volume serial number uniquely identify a file. This number
     * (which, both? -rpm) may change when the system is restarted or when the
     * file is opened. After a process opens a file, the identifier is
     * constant until the file is closed. An application can use this
     * identifier and the volume serial number to determine whether two
     * handles refer to the same file.
     */
    DWORD fileindexlo;
    DWORD fileindexhi;
#endif
    /* Information from properties set by 'h5repart' tool */
    hbool_t     fam_to_sec2;    /* Whether to eliminate the family driver info
                                 * and convert this file to a single file */
} H5FD_silo_t;

#ifdef H5_HAVE_LSEEK64
#   define file_offset_t        off64_t
#elif defined (_WIN32) && !defined(__MWERKS__)
# /*MSVC*/
#   define file_offset_t        __int64
#else
#   define file_offset_t        off_t
#endif

/*
 * These macros check for overflow of various quantities.  These macros
 * assume that file_offset_t is signed and haddr_t and size_t are unsigned.
 *
 * ADDR_OVERFLOW:	Checks whether a file address of type `haddr_t'
 *			is too large to be represented by the second argument
 *			of the file seek function.
 *
 * SIZE_OVERFLOW:	Checks whether a buffer size of type `hsize_t' is too
 *			large to be represented by the `size_t' type.
 *
 * REGION_OVERFLOW:	Checks whether an address and size pair describe data
 *			which can be addressed entirely by the second
 *			argument of the file seek function.
 */
/* adding for windows NT filesystem support. */
#define MAXADDR (((haddr_t)1<<(8*sizeof(file_offset_t)-1))-1)
#define ADDR_OVERFLOW(A)	(HADDR_UNDEF==(A) || ((A) & ~(haddr_t)MAXADDR))
#define SIZE_OVERFLOW(Z)	((Z) & ~(hsize_t)MAXADDR)
#define REGION_OVERFLOW(A,Z)	(ADDR_OVERFLOW(A) || SIZE_OVERFLOW(Z) || \
    HADDR_UNDEF==(A)+(Z) || (file_offset_t)((A)+(Z))<(file_offset_t)(A))

/* Prototypes */
static H5FD_t *H5FD_silo_open(const char *name, unsigned flags,
                 hid_t fapl_id, haddr_t maxaddr);
static herr_t H5FD_silo_close(H5FD_t *lf);
static int H5FD_silo_cmp(const H5FD_t *_f1, const H5FD_t *_f2);
static herr_t H5FD_silo_query(const H5FD_t *_f1, unsigned long *flags);
static haddr_t H5FD_silo_alloc(H5FD_t *_file, H5FD_mem_t type, hid_t dxpl_id, hsize_t size);
static haddr_t H5FD_silo_get_eoa(const H5FD_t *_file, H5FD_mem_t type);
static herr_t H5FD_silo_set_eoa(H5FD_t *_file, H5FD_mem_t type, haddr_t addr);
static haddr_t H5FD_silo_get_eof(const H5FD_t *_file);
static herr_t  H5FD_silo_get_handle(H5FD_t *_file, hid_t fapl, void** file_handle);
static herr_t H5FD_silo_read(H5FD_t *lf, H5FD_mem_t type, hid_t fapl_id, haddr_t addr,
                size_t size, void *buf);
static herr_t H5FD_silo_write(H5FD_t *lf, H5FD_mem_t type, hid_t fapl_id, haddr_t addr,
                size_t size, const void *buf);
static herr_t H5FD_silo_truncate(H5FD_t *_file, hid_t dxpl_id, hbool_t closing);

static const H5FD_class_t H5FD_silo_g = {
    "silo",				        /*name			*/
    MAXADDR,				        /*maxaddr		*/
    H5F_CLOSE_WEAK,				/* fc_degree		*/
    NULL,					/*sb_size		*/
    NULL,					/*sb_encode		*/
    NULL,					/*sb_decode		*/
    0, 						/*fapl_size		*/
    NULL,					/*fapl_get		*/
    NULL,					/*fapl_copy		*/
    NULL, 					/*fapl_free		*/
    0,						/*dxpl_size		*/
    NULL,					/*dxpl_copy		*/
    NULL,					/*dxpl_free		*/
    H5FD_silo_open,		                /*open			*/
    H5FD_silo_close,		                /*close			*/
    H5FD_silo_cmp,			        /*cmp			*/
    H5FD_silo_query,		                /*query			*/
    NULL,					/*get_type_map		*/
    H5FD_silo_alloc,				/*alloc			*/
    NULL,					/*free			*/
    H5FD_silo_get_eoa,		                /*get_eoa		*/
    H5FD_silo_set_eoa, 	                	/*set_eoa		*/
    H5FD_silo_get_eof,		                /*get_eof		*/
    H5FD_silo_get_handle,                       /*get_handle            */
    H5FD_silo_read,		                /*read			*/
    H5FD_silo_write,		                /*write			*/
    NULL,		                	/*flush			*/
    H5FD_silo_truncate,				/*truncate		*/
    NULL,                                       /*lock                  */
    NULL,                                       /*unlock                */
    H5FD_FLMAP_SINGLE 		                /*fl_map		*/
};


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_init
 *
 * Purpose:	Initialize this driver by registering the driver with the
 *		library.
 *
 * Return:	Success:	The driver ID for the silo driver.
 *
 *		Failure:	Negative.
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 29, 1999
 *
 * Modifications:
 *      Stolen from the sec2 driver - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5FD_silo_init(void)
{
    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if (H5I_VFL!=H5Iget_type(H5FD_SILO_g))
        H5FD_SILO_g = H5FDregister(&H5FD_silo_g);
    return(H5FD_SILO_g);
}


/*---------------------------------------------------------------------------
 * Function:	H5FD_silo_term
 *
 * Purpose:	Shut down the VFD
 *
 * Return:	<none>
 *
 * Programmer:  Quincey Koziol
 *              Friday, Jan 30, 2004
 *
 * Modification:
 *
 *---------------------------------------------------------------------------
 */
void
H5FD_silo_term(void)
{
    /* Reset VFL ID */
    H5FD_SILO_g=0;

} /* end H5FD_silo_term() */


/*-------------------------------------------------------------------------
 * Function:	H5Pset_fapl_silo
 *
 * Purpose:	Modify the file access property list to use the H5FD_SILO
 *		driver defined in this source file.  There are no driver
 *		specific properties.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Thursday, February 19, 1998
 *
 * Modifications:
 *      Stolen from the sec2 driver - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_fapl_silo(hid_t fapl_id)
{
    static const char *func="H5FDset_fapl_silo";  /*for error reporting*/

    /*NO TRACE*/

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if(0 == H5Pisa_class(fapl_id, H5P_FILE_ACCESS))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_BADTYPE, "not a file access property list", -1, -1)

    return H5Pset_driver(fapl_id, H5FD_SILO, NULL);
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_open
 *
 * Purpose:	Create and/or opens a Standard C file as an HDF5 file.
 *
 * Bugs:	H5F_ACC_EXCL has a race condition. (? -QAK)
 *
 * Errors:
 *		IO	  CANTOPENFILE	File doesn't exist and CREAT wasn't
 *					specified.
 *		IO	  CANTOPENFILE	Fopen failed.
 *		IO	  FILEEXISTS	File exists but CREAT and EXCL were
 *					specified.
 *
 * Return:	Success:	A pointer to a new file data structure. The
 *				public fields will be initialized by the
 *				caller, which is always H5FD_open().
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, October 22, 1997
 *
 * Modifications:
 *      Ported to VFL/H5FD layer - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static H5FD_t *
H5FD_silo_open( const char *name, unsigned flags, hid_t fapl_id,
    haddr_t maxaddr)
{
    int         o_flags;
    int         fd = -1;
    unsigned    write_access = 0;     /* File opened with write access? */
    H5FD_silo_t	*file = NULL;
    h5_stat_t   sb;
    static const char *func = "H5FD_silo_open";  /* Function Name for error reporting */

    /* Sanity check on file offsets */
    assert(sizeof(file_offset_t)>=sizeof(size_t));

    /* Shut compiler up */
    fapl_id=fapl_id;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Check arguments */
    if (!name || !*name)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_BADVALUE, "invalid file name", NULL, -1)
    if (0==maxaddr || HADDR_UNDEF==maxaddr)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_BADRANGE, "bogus maxaddr", NULL, -1)
    if (ADDR_OVERFLOW(maxaddr))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_OVERFLOW, "maxaddr too large", NULL, -1)

    /* Build the open flags */
    o_flags = (H5F_ACC_RDWR & flags) ? O_RDWR : O_RDONLY;
    if (H5F_ACC_TRUNC & flags) o_flags |= O_TRUNC;
    if (H5F_ACC_CREAT & flags) o_flags |= O_CREAT;
    if (H5F_ACC_EXCL & flags) o_flags |= O_EXCL;

    errno = 0;
    if ((fd = HDopen(name, o_flags, 0666)) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_CANTOPENFILE, "HDopen failed", NULL, errno)

    if (HDfstat(fd, &sb)<0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_FILE, H5E_BADFILE, "HDfstat failed", NULL, errno)

    if (flags & H5F_ACC_RDWR)
        write_access = 1;

    /* Build the return value */
    if(NULL == (file = (H5FD_silo_t *)calloc((size_t)1, sizeof(H5FD_silo_t))))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_RESOURCE, H5E_NOSPACE, "calloc failed", NULL, errno)

    file->fd = fd;
    file->eof = (haddr_t)sb.st_size;
    file->pos = HADDR_UNDEF;
    file->op = OP_UNKNOWN;
    file->write_access = write_access;

    /* The unique key */
    {
#ifdef _WIN32
	struct _BY_HANDLE_FILE_INFORMATION fileinfo;
        (void)GetFileInformationByHandle((HANDLE)_get_osfhandle(file->fd), &fileinfo);
        file->fileindexhi = fileinfo.nFileIndexHigh;
        file->fileindexlo = fileinfo.nFileIndexLow;
#else
        file->device = sb.st_dev;
#ifdef H5_VMS
        file->inode[0] = sb.st_ino[0];
        file->inode[1] = sb.st_ino[1];
        file->inode[2] = sb.st_ino[2];
#else
        file->inode = sb.st_ino;
#endif /*H5_VMS*/

#endif
    }

#warning FAMILY TO SEC2 LOGIC ON FAPL MISSING

    return((H5FD_t*)file);
}   /* end H5FD_silo_open() */


/*-------------------------------------------------------------------------
 * Function:	H5F_silo_close
 *
 * Purpose:	Closes a file.
 *
 * Errors:
 *		IO	  CLOSEERROR	Fclose failed.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, October 22, 1997
 *
 * Modifications:
 *      Ported to VFL/H5FD layer - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_close(H5FD_t *_file)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    static const char *func="H5FD_silo_close";  /* Function Name for error reporting */

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    errno = 0;
    if (close(file->fd) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_CLOSEERROR, "close failed", -1, errno)

    free(file);

    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_cmp
 *
 * Purpose:	Compares two files belonging to this driver using an
 *		arbitrary (but consistent) ordering.
 *
 * Return:	Success:	A value like strcmp()
 *
 *		Failure:	never fails (arguments were checked by the
 *				caller).
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 29, 1999
 *
 * Modifications:
 *      Stolen from the sec2 driver - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static int
H5FD_silo_cmp(const H5FD_t *_f1, const H5FD_t *_f2)
{
    const H5FD_silo_t	*f1 = (const H5FD_silo_t*)_f1;
    const H5FD_silo_t	*f2 = (const H5FD_silo_t*)_f2;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

#ifdef _WIN32
    if (f1->fileindexhi < f2->fileindexhi) return -1;
    if (f1->fileindexhi > f2->fileindexhi) return 1;

    if (f1->fileindexlo < f2->fileindexlo) return -1;
    if (f1->fileindexlo > f2->fileindexlo) return 1;

#else
#ifdef H5_DEV_T_IS_SCALAR
    if (f1->device < f2->device) return -1;
    if (f1->device > f2->device) return 1;
#else /* H5_DEV_T_IS_SCALAR */
    /* If dev_t isn't a scalar value on this system, just use memcmp to
     * determine if the values are the same or not.  The actual return value
     * shouldn't really matter...
     */
    if(HDmemcmp(&(f1->device),&(f2->device),sizeof(dev_t))<0) return -1;
    if(HDmemcmp(&(f1->device),&(f2->device),sizeof(dev_t))>0) return 1;
#endif /* H5_DEV_T_IS_SCALAR */

#ifndef H5_VMS
    if (f1->inode < f2->inode) return -1;
    if (f1->inode > f2->inode) return 1;
#else
    if(HDmemcmp(&(f1->inode),&(f2->inode),3*sizeof(ino_t))<0) return -1;
    if(HDmemcmp(&(f1->inode),&(f2->inode),3*sizeof(ino_t))>0) return 1;
#endif /*H5_VMS*/

#endif

    return 0;

}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_query
 *
 * Purpose:	Set the flags that this VFL driver is capable of supporting.
 *              (listed in H5FDpublic.h)
 *
 * Return:	Success:	non-negative
 *
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Friday, August 25, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_query(const H5FD_t *_f, unsigned long *flags /* out */)
{
    /* Shut compiler up */
    _f=_f;

    /* Set the VFL feature flags that this driver supports */
    if(flags) {
        *flags = 0;
        *flags|=H5FD_FEAT_AGGREGATE_METADATA; /* OK to aggregate metadata allocations */
        *flags|=H5FD_FEAT_ACCUMULATE_METADATA; /* OK to accumulate metadata for faster writes */
        *flags|=H5FD_FEAT_DATA_SIEVE;       /* OK to perform data sieving for faster raw data reads & writes */
        *flags|=H5FD_FEAT_AGGREGATE_SMALLDATA; /* OK to aggregate "small" raw data allocations */
    }

    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_alloc
 *
 * Purpose:	Allocates file memory. If fseeko isn't available, makes
 *              sure the file size isn't bigger than 2GB because the
 *              parameter OFFSET of fseek is of the type LONG INT, limiting
 *              the file size to 2GB.
 *
 * Return:	Success:	Address of new memory
 *
 *		Failure:	HADDR_UNDEF
 *
 * Programmer:	Raymond Lu
 *              30 March 2007
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static haddr_t
H5FD_silo_alloc(H5FD_t *_file, H5FD_mem_t /*UNUSED*/ type, hid_t /*UNUSED*/ dxpl_id, hsize_t size)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    haddr_t		addr;
    haddr_t ret_value;          /* Return value */

    /* Shut compiler up */
    type = type;
    dxpl_id = dxpl_id;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Compute the address for the block to allocate */
    addr = file->eoa;

    /* Check if we need to align this block */
    if(size >= file->pub.threshold) {
        /* Check for an already aligned block */
        if((addr % file->pub.alignment) != 0)
            addr = ((addr / file->pub.alignment) + 1) * file->pub.alignment;
    } /* end if */

    file->eoa = addr + size;

    /* Set return value */
    ret_value = addr;

    return(ret_value);
}   /* H5FD_silo_alloc() */


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_get_eoa
 *
 * Purpose:	Gets the end-of-address marker for the file. The EOA marker
 *		is the first address past the last byte allocated in the
 *		format address space.
 *
 * Return:	Success:	The end-of-address marker.
 *
 *		Failure:	HADDR_UNDEF
 *
 * Programmer:	Robb Matzke
 *              Monday, August  2, 1999
 *
 * Modifications:
 *              Stolen from the sec2 driver - QAK, 10/18/99
 *
 *              Raymond Lu
 *              21 Dec. 2006
 *              Added the parameter TYPE.  It's only used for MULTI driver.
 *
 *-------------------------------------------------------------------------
 */
static haddr_t
H5FD_silo_get_eoa(const H5FD_t *_file, H5FD_mem_t /*unused*/ type)
{
    const H5FD_silo_t	*file = (const H5FD_silo_t *)_file;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Shut compiler up */
    type = type;

    return(file->eoa);
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_set_eoa
 *
 * Purpose:	Set the end-of-address marker for the file. This function is
 *		called shortly after an existing HDF5 file is opened in order
 *		to tell the driver where the end of the HDF5 data is located.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 29, 1999
 *
 * Modifications:
 *              Stolen from the sec2 driver - QAK, 10/18/99
 *
 *              Raymond Lu
 *              21 Dec. 2006
 *              Added the parameter TYPE.  It's only used for MULTI driver.
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_set_eoa(H5FD_t *_file, H5FD_mem_t /*unused*/ type, haddr_t addr)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Shut compiler up */
    type = type;

    file->eoa = addr;

    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_get_eof
 *
 * Purpose:	Returns the end-of-file marker, which is the greater of
 *		either the Unix end-of-file or the HDF5 end-of-address
 *		markers.
 *
 * Return:	Success:	End of file address, the first address past
 *				the end of the "file", either the Unix file
 *				or the HDF5 file.
 *
 *		Failure:	HADDR_UNDEF
 *
 * Programmer:	Robb Matzke
 *              Thursday, July 29, 1999
 *
 * Modifications:
 *      Stolen from the sec2 driver - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static haddr_t
H5FD_silo_get_eof(const H5FD_t *_file)
{
    const H5FD_silo_t	*file = (const H5FD_silo_t *)_file;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    return(MAX(file->eof, file->eoa));
}


/*-------------------------------------------------------------------------
 * Function:       H5FD_silo_get_handle
 *
 * Purpose:        Returns the file handle of silo file driver.
 *
 * Returns:        Non-negative if succeed or negative if fails.
 *
 * Programmer:     Raymond Lu
 *                 Sept. 16, 2002
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_get_handle(H5FD_t *_file, hid_t fapl, void** file_handle)
{
    H5FD_silo_t       *file = (H5FD_silo_t *)_file;
    static const char  *func="H5FD_silo_get_handle";  /* Function Name for error reporting */

    /* Shut compiler up */
    fapl=fapl;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if (file_handle)
        *file_handle = &(file->fd);

    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5F_silo_read
 *
 * Purpose:	Reads SIZE bytes beginning at address ADDR in file LF and
 *		places them in buffer BUF.  Reading past the logical or
 *		physical end of file returns zeros instead of failing.
 *
 * Errors:
 *		IO	  READERROR	Fread failed.
 *		IO	  SEEKERROR	Fseek failed.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, October 22, 1997
 *
 * Modifications:
 *		June 2, 1998	Albert Cheng
 *		Added xfer_mode argument
 *
 *      Ported to VFL/H5FD layer - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_read(H5FD_t *_file, H5FD_mem_t type, hid_t dxpl_id, haddr_t addr, size_t size,
    void *buf/*out*/)
{
    size_t		n;
    ssize_t             nbytes;
    H5FD_silo_t		*file = (H5FD_silo_t*)_file;
    static const char *func="H5FD_silo_read";  /* Function Name for error reporting */

    /* Shut compiler up */
    type=type;
    dxpl_id=dxpl_id;

    HDassert(file && file->pub.cls);
    HDassert(buf);

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Check for overflow */
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (addr+size>file->eoa)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)

    /* Check easy cases */
    if (0 == size)
        return(0);
    if ((haddr_t)addr >= file->eof) {
        memset(buf, 0, size);
        return(0);
    }

    /* Seek to the correct location */
    if((addr != file->pos || OP_READ != file->op) &&
        HDlseek(file->fd, (file_offset_t)addr, SEEK_SET) < 0)
    {
        file->op = OP_UNKNOWN;
        file->pos = HADDR_UNDEF;
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)
    }

    /*
     * Read data, being careful of interrupted system calls, partial results,
     * and the end of the file.
     */
    while(size > 0) {
        do {
            nbytes = HDread(file->fd, buf, size);
        } while(-1 == nbytes && EINTR == errno);
        if(-1 == nbytes) {/* error */
            file->op = OP_UNKNOWN;
            file->pos = HADDR_UNDEF;
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_READERROR, "HDread failed", -1, errno)
        }
        if(0 == nbytes) {
            /* end of file but not end of format address space */
            HDmemset(buf, 0, size);
            break;
        } /* end if */
        HDassert(nbytes >= 0);
        HDassert((size_t)nbytes <= size);
        size -= (size_t)nbytes;
        addr += (haddr_t)nbytes;
        buf = (char *)buf + nbytes;
    } /* end while */

    /* Update current position */
    file->pos = addr;
    file->op = OP_READ;
    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5F_silo_write
 *
 * Purpose:	Writes SIZE bytes from the beginning of BUF into file LF at
 *		file address ADDR.
 *
 * Errors:
 *		IO	  SEEKERROR	Fseek failed.
 *		IO	  WRITEERROR	Fwrite failed.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		Wednesday, October 22, 1997
 *
 * Modifications:
 *		June 2, 1998	Albert Cheng
 *		Added xfer_mode argument
 *
 *      Ported to VFL/H5FD layer - QAK, 10/18/99
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_write(H5FD_t *_file, H5FD_mem_t type, hid_t dxpl_id, haddr_t addr,
		size_t size, const void *buf)
{
    H5FD_silo_t		*file = (H5FD_silo_t*)_file;
    static const char *func="H5FD_silo_write";  /* Function Name for error reporting */
    ssize_t             nbytes;

    /* Shut compiler up */
    dxpl_id=dxpl_id;
    type=type;

    HDassert(file && file->pub.cls);
    HDassert(buf);

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Check for overflow conditions */
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (addr+size>file->eoa)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)

{
    static int last_size = 0;
    static int last_addr = 0;
    static int run_size = 0;
    static int run_cnt = 0;

    if (addr == last_addr + last_size)
    {
        fprintf(stderr, "\n");
        if (run_size == 0) run_size = size + last_size;
        else run_size += (int) size;
        if (run_cnt == 0) run_cnt=2;
        else run_cnt++;
    } 
    else
    {
        if (run_size > 0)
            fprintf(stderr, "; missed aggregating %d bytes in %d writes\n", run_size, run_cnt);
        else
            fprintf(stderr, "\n");
        run_size = 0; 
        run_cnt = 0;
    }

    last_size = (int) size;
    last_addr = (int) addr;
}

    /* Seek to the correct location */
    errno = 0;
    if((addr != file->pos || OP_WRITE != file->op) &&
        HDlseek(file->fd, (file_offset_t)addr, SEEK_SET) < 0)
    {
        file->op = OP_UNKNOWN;
        file->pos = HADDR_UNDEF;
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)
    }

    /*
     * Write the data, being careful of interrupted system calls and partial
     * results
     */
    while(size > 0) {
        do {
            nbytes = HDwrite(file->fd, buf, size);
        } while(-1 == nbytes && EINTR == errno);
        if(-1 == nbytes) {/* error */
            file->op = OP_UNKNOWN;
            file->pos = HADDR_UNDEF;
            H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_WRITEERROR, "HDwrite failed", -1, errno)
        }
        HDassert(nbytes > 0);
        HDassert((size_t)nbytes <= size);
        size -= (size_t)nbytes;
        addr += (haddr_t)nbytes;
        buf = (const char *)buf + nbytes;
    } /* end while */

    /* Update current position and eof */
    file->pos = addr;
    file->op = OP_WRITE;
    if(file->pos > file->eof)
        file->eof = file->pos;

    return(0);
}


/*-------------------------------------------------------------------------
 * Function:	H5F_silo_truncate
 *
 * Purpose:	Makes sure that the true file size is the same (or larger)
 *		than the end-of-address.
 *
 * Errors:
 *		IO	  SEEKERROR     fseek failed.
 *		IO	  WRITEERROR    fflush or fwrite failed.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Thursday, January 31, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_truncate(H5FD_t *_file, hid_t dxpl_id, hbool_t closing)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    static const char *func = "H5FD_silo_truncate";  /* Function Name for error reporting */

    /* Shut compiler up */
    dxpl_id = dxpl_id;
    closing = closing;

    HDassert(file);

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Extend the file to make sure it's large enough */
    if(file->write_access && !H5F_addr_eq(file->eoa, file->eof)) {
#ifdef _WIN32
        HFILE filehandle;   /* Windows file handle */
        LARGE_INTEGER li;   /* 64-bit integer for SetFilePointer() call */

        /* Map the posix file handle to a Windows file handle */
        filehandle = _get_osfhandle(file->fd);

        /* Translate 64-bit integers into form Windows wants */
        /* [This algorithm is from the Windows documentation for SetFilePointer()] */
        li.QuadPart = (LONGLONG)file->eoa;
        (void)SetFilePointer((HANDLE)filehandle, li.LowPart, &li.HighPart, FILE_BEGIN);
        if(SetEndOfFile((HANDLE)filehandle) == 0)
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "unable to extend file properly", -1, -1)
#else /* _WIN32 */
#ifdef H5_VMS
        /* Reset seek offset to the beginning of the file, so that the file isn't 
         * re-extended later.  This may happen on Open VMS. */
        if(-1 == HDlseek(file->fd, (file_offset_t)0, SEEK_SET))
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)
#endif

        if(-1 == HDftruncate(file->fd, (file_offset_t)file->eoa))
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDftruncate failed", -1, errno)
#endif /* _WIN32 */

        /* Update the eof value */
        file->eof = file->eoa;

        /* Reset last file I/O information */
        file->pos = HADDR_UNDEF;
        file->op = OP_UNKNOWN;
    } /* end if */

    return(0);
} /* end H5FD_silo_truncate() */


#ifdef _H5private_H
/*
 * This is not related to the functionality of the driver code.
 * It is added here to trigger warning if HDF5 private definitions are included
 * by mistake.  The code should use only HDF5 public API and definitions.
 */
#error "Do not use HDF5 private definitions"
#endif


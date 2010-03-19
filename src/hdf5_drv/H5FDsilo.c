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

/* Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
   in this file uses version 1.6 of the HDF5 API. This is harmless for
   versions of HDF5 before 1.8 and ensures correct compilation with
   version 1.8 and thereafter. When, and if, the HDF5 code in this file
   is explicitly upgraded to the 1.8 API, this symbol should be removed. */
#define H5_USE_16_API

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
#define OP_READMETA	3


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


#define SB_OFFSET	512
#define META_OFFSET     0
#warning DEFINE THIS OFFSET BASED ON MAXADDR SIZE
#define RAW_OFFSET      (1<<27)

static const char *flavors(H5F_mem_t m)
{
    static char tmp[32];

    if (m == H5FD_MEM_DEFAULT)
        return "H5FD_MEM_DEFAULT";
    if (m == H5FD_MEM_SUPER)
        return "H5FD_MEM_SUPER";
    if (m == H5FD_MEM_BTREE)
        return "H5FD_MEM_BTREE";
    if (m == H5FD_MEM_DRAW)
        return "H5FD_MEM_DRAW";
    if (m == H5FD_MEM_GHEAP)
        return "H5FD_MEM_GHEAP";
    if (m == H5FD_MEM_LHEAP)
        return "H5FD_MEM_LHEAP";
    if (m == H5FD_MEM_OHDR)
        return "H5FD_MEM_OHDR";

    sprintf(tmp, "Unknown (%d)", (int) m);
    return tmp;
}


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
    char       *mbuf;
    hsize_t     mbuf_size;
    haddr_t	mbuf_eoa;
    haddr_t     mbuf_eof;
    haddr_t     mbuf_faddr;
    unsigned    mbuf_dirty;
    unsigned    first_read;
    hsize_t     increment;
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
static hsize_t H5FD_silo_sb_size(H5FD_t *file);
static herr_t H5FD_silo_sb_encode(H5FD_t *file, char *name/*out*/,
                                   unsigned char *buf/*out*/);
static herr_t H5FD_silo_sb_decode(H5FD_t *file, const char *name,
                                   const unsigned char *buf);
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
    H5FD_silo_sb_size,                          /*sb_size               */
    H5FD_silo_sb_encode,                        /*sb_encode             */
    H5FD_silo_sb_decode,                        /*sb_decode             */
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
    H5FD_FLMAP_DICHOTOMY 			/*fl_map		*/
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
    static const char *func="H5FD_silo_init";

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

herr_t
H5Pset_silo_max_meta(hid_t fapl_id, hsize_t max_meta)
{
    static const char *func="H5FDset_silo_max_meta";

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if(0 == H5Pisa_class(fapl_id, H5P_FILE_ACCESS))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_BADTYPE, "not a file access property list", -1, -1)
    if (H5Pset(fapl_id, "silo_max_meta", &max_meta) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_max_meta", -1, -1)
#if 0
    if (H5Pset_meta_block_size(fapl_id, 0) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set meta_block_size", -1, -1)
#endif

    return 0;
}

herr_t
H5Pset_silo_raw_buf_size(hid_t fapl_id, hsize_t raw_buf_size)
{
    static const char *func="H5FDset_silo_raw_buf_size";

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if(0 == H5Pisa_class(fapl_id, H5P_FILE_ACCESS))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_BADTYPE, "not a file access property list", -1, -1)
    if (H5Pset(fapl_id, "silo_raw_buf_size", &raw_buf_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_raw_buf_size", -1, -1)
#if 0
    if (H5Pset_small_data_block_size(fapl_id, 0) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set small_data_block_size", -1, -1)
#endif

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_sb_size
 *
 * Purpose:	Returns the size of the private information to be stored in
 *		the superblock.
 *
 * Return:	Success:	The super block driver data size.
 *
 *		Failure:	never fails
 *
 * Programmer:	Robb Matzke
 *              Monday, August 16, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static hsize_t
H5FD_silo_sb_size(H5FD_t *_file)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    unsigned		nseen = 0;
    hsize_t		nbytes = 8; /*size of header*/

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Address of metadata at end of file */
    nbytes += 8;

    return nbytes;
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_sb_encode
 *
 * Purpose:	Encode driver information for the superblock. The NAME
 *		argument is a nine-byte buffer which will be initialized with
 *		an eight-character name/version number and null termination.
 *
 *		The encoding is a six-byte member mapping followed two bytes
 *		which are unused. For each unique file in usage-type order
 *		encode all the starting addresses as unsigned 64-bit integers,
 *		then all the EOA values as unsigned 64-bit integers, then all
 *		the template names as null terminated strings which are
 *		siloples of 8 characters.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Monday, August 16, 1999
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_sb_encode(H5FD_t *_file, char *name/*out*/,
		     unsigned char *buf/*out*/)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    unsigned char	*p;
    static const char *func="H5FD_silo_sb_encode";  /* Function Name for error reporting */
    haddr_t tmp_eoa = file->eoa + SB_OFFSET;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Name and version number */
    strncpy(name, "LLNLsilo", (size_t)8);
    name[8] = '\0';

    /*
     * Copy the starting addresses and EOA values into the buffer in order of
     * usage type but only for types which map to something unique.
     */

    /* Encode all starting addresses and EOA values */
    p = buf+8;
    assert(sizeof(haddr_t)<=8);
    memcpy(p, &tmp_eoa, sizeof(haddr_t));
    if (H5Tconvert(H5T_NATIVE_HADDR, H5T_STD_U64LE, 1, buf+8, NULL,
		   H5P_DEFAULT)<0)
        H5Epush_ret(func, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTCONVERT, "can't convert superblock info", -1)

    return 0;
} /* end H5FD_silo_sb_encode() */


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_sb_decode
 *
 * Purpose:	Decodes the superblock information for this driver. The NAME
 *		argument is the eight-character (plus null termination) name
 *		stored in the file.
 *
 *		The FILE argument is updated according to the information in
 *		the superblock. This may mean that some member files are
 *		closed and others are opened.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *              Monday, August 16, 1999
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_sb_decode(H5FD_t *_file, const char *name, const unsigned char *buf)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    char		x[2*H5FD_MEM_NTYPES*8];
    haddr_t		*ap;
    static const char *func="H5FD_silo_sb_decode";  /* Function Name for error reporting */

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Make sure the name/version number is correct */
    if (strcmp(name, "LLNLsilo"))
        H5Epush_ret(func, H5E_ERR_CLS, H5E_FILE, H5E_BADVALUE, "invalid silo superblock", -1)

    buf += 8;
    /* Decode Address and EOA values */
    assert(sizeof(haddr_t)<=8);
    memcpy(x, buf, 8);
    buf += 8;
    if (H5Tconvert(H5T_STD_U64LE, H5T_NATIVE_HADDR, 1, x, NULL, H5P_DEFAULT)<0)
        H5Epush_ret(func, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTCONVERT, "can't convert superblock info", -1)
    ap = (haddr_t*)x;
    file->mbuf_faddr = *ap;

    /* Set the EOA marker for all open files */
#warning FIXME

    return 0;
} /* end H5FD_silo_sb_decode() */

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
    hsize_t silo_max_meta;
    hsize_t meta_block_size;
    hsize_t small_data_block_size;
    hsize_t raw_buf_size;

    /* Sanity check on file offsets */
    assert(sizeof(file_offset_t)>=sizeof(size_t));

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Check arguments */
    if (!name || !*name)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_BADVALUE, "invalid file name", NULL, -1)
    if (0==maxaddr || HADDR_UNDEF==maxaddr)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_BADRANGE, "bogus maxaddr", NULL, -1)
    if (ADDR_OVERFLOW(maxaddr))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_ARGS, H5E_OVERFLOW, "maxaddr too large", NULL, -1)

    /* get some properties and sanity check them */
#if 0
    if (H5Pget_meta_block_size(fapl_id, &meta_block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get meta_block_size", 0, -1)
    if (meta_block_size != 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_NONE_MINOR, "meta_block_size!=0", 0, -1)
    if (H5Pget_small_data_block_size(fapl_id, &small_data_block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get meta_block_size", 0, -1)
    if (small_data_block_size != 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_NONE_MINOR, "small_data_block_size!=0", 0, -1)
#endif

    if (H5Pget(fapl_id, "silo_max_meta", &silo_max_meta) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get silo_max_meta", 0, -1)
    if (H5Pget(fapl_id, "silo_raw_buf_size", &raw_buf_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get silo_max_meta", 0, -1)

#if 0
    /* fudge for super block */
    silo_max_meta += 512;
#endif

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
#warning SHOULD THIS HAVE SILO_MAX_META SUBTRACTED OUT
    file->eof = (haddr_t)sb.st_size;
    file->pos = HADDR_UNDEF;
    file->op = OP_UNKNOWN;
    file->write_access = write_access;
    file->first_read = 1;
#warning MAKE INCREMENT A USER SETTABLE PARAMETEA
    file->increment = (1<<18); /* 1/4 megabyte */

    /* read md in one junk if its an existing file */
#if 0
    file->op = OP_READMETA;
    if (file->eof > 0)
    {
        if (H5FD_silo_read((H5FD_t*)file, H5FD_MEM_DEFAULT, (hid_t)-1, file->mbuf_faddr, file->mbuf_size, file->mbuf)<0)
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_READERROR, "Reading of metadata failed", 0, -1)
    }
#endif

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

    /* write any buffered raw data to file */

    /* write meta data to file */
    if (file->mbuf_dirty > 0)
    {
        /* write enough of header of MD that we produce a good superblock at head of file */
        H5FD_silo_write(_file, H5FD_MEM_NOLIST, (hid_t) -1, (haddr_t)0, (size_t) SB_OFFSET, file->mbuf);

        /* now, write all of md, including the tidbit we just wrote above for superblock at end of file. */
        file->mbuf_faddr = file->eoa+SB_OFFSET;
        H5FD_silo_write(_file, H5FD_MEM_NOLIST, (hid_t) -1, (haddr_t)file->mbuf_faddr, (size_t) file->mbuf_eoa, file->mbuf);
    }

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
    static const char  *func="H5FD_silo_alloc";  /* Function Name for error reporting */

    /* Shut compiler up */
    type = type;
    dxpl_id = dxpl_id;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if (type == H5FD_MEM_DRAW)
    {
        addr = file->eoa;
#warning INCLUDE ALIGNMENT LOGIC HERE LATER
        file->eoa = addr + size;
        return(addr+RAW_OFFSET);
    }
    else
    {
        addr = file->mbuf_eoa;
        file->mbuf_eoa = addr + size;
        if (file->mbuf_eoa > file->mbuf_size)
        {
            void *x;
            size_t new_mbuf_size;

            /* Determine new size of memory buffer */
            new_mbuf_size = (size_t) (file->increment * ((addr + size) / file->increment));
            if((addr + size) % file->increment)
                new_mbuf_size += file->increment;

            x = realloc(file->mbuf, new_mbuf_size);
            if (!x)
                H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_RESOURCE, H5E_NOSPACE, "out of memory for metadata", -1, -1)

            file->mbuf = x;
            file->mbuf_size = new_mbuf_size;
        }
        return(addr+META_OFFSET);
    }

   return(HADDR_UNDEF);

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
    haddr_t eoa;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Shut compiler up */
    type = type;

    if (type == H5FD_MEM_DEFAULT)
    {
        haddr_t eoar = file->eoa;
        haddr_t eoam = file->mbuf_eoa;
        return(MAX(eoar,eoam));
    }
    else if (type == H5FD_MEM_DRAW)
    {
        eoa = file->eoa;
        if (eoa > 0) eoa += RAW_OFFSET;
        return(eoa);
    }
    else
    {
        eoa = file->mbuf_eoa;
        if (eoa > 0) eoa += META_OFFSET;
        return (eoa);
    }
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


    if (type == H5FD_MEM_DRAW)
    {
        file->eoa = addr-RAW_OFFSET;
    }
    else
    {
        file->mbuf_eoa = addr-META_OFFSET;
    }
    return 0;
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
    haddr_t tmp_eoa, tmp_eoam, tmp_eoar;
    haddr_t tmp_eof, tmp_eofm, tmp_eofr;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

#if 1
    /* meta data part */
    tmp_eofm = file->mbuf_eof;
    if (tmp_eofm > 0) tmp_eofm += META_OFFSET;

    tmp_eoam = file->mbuf_eoa;
    if (tmp_eoam > 0) tmp_eoam += META_OFFSET;

    /* raw data part */
    tmp_eofr = file->eof;
    if (tmp_eofr > 0) tmp_eofr += RAW_OFFSET;

    tmp_eoar = file->eoa;
    if (tmp_eoar > 0) tmp_eoar += RAW_OFFSET;

    tmp_eof = MAX(tmp_eofm, tmp_eofr);
    tmp_eoa = MAX(tmp_eoam, tmp_eoar);

    return(MAX(tmp_eof, tmp_eoa));
#endif

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

    if (file->first_read && file->eof > 0 && file->mbuf_faddr > 0)
    {
        size_t mbuf_size, tmp_size;;

        assert(file->eof > file->mbuf_faddr);
        mbuf_size = file->eof - file->mbuf_faddr;
        tmp_size = mbuf_size;

        /* Determine new size of memory buffer */
        mbuf_size = (size_t) (file->increment * (tmp_size / file->increment));
        if(tmp_size % file->increment)
            mbuf_size += file->increment;

        if(NULL == (file->mbuf = (char*)calloc((size_t)mbuf_size, 1)))
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_RESOURCE, H5E_NOSPACE, "calloc failed", 0, -1)

        file->op = OP_READMETA;
        file->first_read = 0;
        if (H5FD_silo_read(_file, H5FD_MEM_NOLIST, (hid_t)-1, file->mbuf_faddr, tmp_size, file->mbuf)<0)
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_READERROR, "Reading of metadata failed", 0, -1)
        file->mbuf_eoa = tmp_size;
        file->mbuf_size = mbuf_size;
    }

    HDassert(file && file->pub.cls);
    HDassert(buf);

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Check for overflow */
#if 0
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (file->op != OP_READMETA && (addr+size)>file->eoa)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
#endif

    if (type == H5FD_MEM_DRAW)
    {
        addr -= RAW_OFFSET;
        addr += SB_OFFSET;
    }
    else
    {
        addr -= META_OFFSET;
    }

    /* Check easy cases */
    if (0 == size)
        return(0);
    if ((haddr_t)addr >= file->eof) {
        memset(buf, 0, size);
        return(0);
    }

    if (file->mbuf && type != H5FD_MEM_NOLIST && type != H5FD_MEM_DRAW)
    {
        memcpy(buf, file->mbuf+addr, size);
        return 0;
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

    if (type == H5FD_MEM_DRAW)
    {
        addr -= RAW_OFFSET;
    }
    else
    {
        addr -= META_OFFSET;
    }

    /* Check for overflow conditions */
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    if (type == H5FD_MEM_DRAW)
    {
        if (addr+size>file->eoa)
            H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)
    }
    else if (type != H5FD_MEM_NOLIST)
    {
        if (addr+size>file->mbuf_size)
        {
            void *x;
            size_t new_mbuf_size;

            /* Determine new size of memory buffer */
            new_mbuf_size = (size_t) (file->increment * ((addr + size) / file->increment));
            if((addr + size) % file->increment)
                new_mbuf_size += file->increment;

            x = realloc(file->mbuf, new_mbuf_size);
            if (!x)
                H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_RESOURCE, H5E_NOSPACE, "out of memory for metadata", -1, -1)

            file->mbuf = x;
            file->mbuf_size = new_mbuf_size;
        }
    }

    /* handle non-close/non-draw write */ 
    if (type != H5FD_MEM_NOLIST && type != H5FD_MEM_DRAW)
    {
        memcpy(file->mbuf+addr, buf, size);
        if (addr+size>file->mbuf_eof)
            file->mbuf_eof = addr+size;
        file->mbuf_dirty = 1;
        return 0;
    }

    if (type == H5FD_MEM_DRAW)
        addr += SB_OFFSET;


{
    static int last_size = 0;
    static int last_addr = 0;
    static int run_size = 0;
    static int run_cnt = 0;

    if (addr == last_addr + last_size)
    {
        if (run_size == 0) run_size = size + last_size;
        else run_size += (int) size;
        if (run_cnt == 0) run_cnt=2;
        else run_cnt++;
    } 
    else
    {
        run_size = 0; 
        run_cnt = 0;
    }

    last_size = (int) size;
    last_addr = (int) addr;
}



    /* Seek to the correct location */
    errno = 0;
    if((addr != file->pos || OP_WRITE != file->op))
    {
        if (HDlseek(file->fd, (file_offset_t)addr, SEEK_SET) < 0)
        {
            file->op = OP_UNKNOWN;
            file->pos = HADDR_UNDEF;
            H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)
        }
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

#warning SKIPPING TRUNCATE
    return 0;

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

        if(-1 == HDftruncate(file->fd, (file_offset_t)file->eoa+file->mbuf_size))
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDftruncate failed", -1, errno)
#endif /* _WIN32 */

        /* Update the eof value */
        file->eof = file->eoa+file->mbuf_eoa;

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


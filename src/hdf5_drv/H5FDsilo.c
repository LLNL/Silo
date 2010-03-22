#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#endif
#define MAX(X,Y)	((X)>(Y)?(X):(Y))

/* File operations */
#define OP_UNKNOWN      0
#define OP_READ         1
#define OP_WRITE        2

#define EXACT		0
#define CLOSEST		1

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
    char msg[256];							\
    if (Errno != 0)							\
        snprintf(msg, sizeof(msg), Msg "(errno=%d, \"%s\")",	\
            Errno, strerror(Errno));					\
    ret_value = Ret;							\
    H5Epush_ret(Func, Cls, Maj, Min, msg, Ret)				\
}
#else
#define H5E_PUSH_HELPER(Func,Cls,Maj,Min,Msg,Ret,Errno)			\
{									\
    ret_value = Ret;							\
    H5Epush_ret(Func, Cls, Maj, Min, msg, Ret)				\
}
#endif

typedef struct silo_vfd_relevant_blocks_t_
{
    hsize_t  id0,  id1;
    int     off0, off1;
} silo_vfd_relevant_blocks_t;

typedef struct silo_vfd_block_t_
{
    hsize_t id;
    hsize_t age;
    void *buf;
    unsigned dirty;
} silo_vfd_block_t;

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
    haddr_t     file_eof;
    haddr_t	pos;			/*current file I/O position	*/
    int         op;			/*last operation		*/
    unsigned    write_access;  		/* Flag to indicate the file was opened with write access */
    hsize_t     block_size;
    hsize_t     op_counter;
    silo_vfd_block_t *block_list;
    int         max_blocks;
    int         num_blocks;
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
#define H5_CHECK_OVERFLOW(var, vartype, casttype) \
{                                                 \
    casttype _tmp_overflow = (casttype)(var);     \
    assert((var) == (vartype)_tmp_overflow);      \
}


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
    NULL,					/*alloc			*/
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
    H5FD_FLMAP_SINGLE				/*fl_map		*/
};

static silo_vfd_relevant_blocks_t
relevant_blocks(hsize_t block_size, haddr_t addr, hsize_t size)
{
    silo_vfd_relevant_blocks_t ret_value;

    /* first address of first block, inclusive */
    ret_value.id0 = addr / block_size;
    ret_value.off0 = addr % block_size;

    /* last address of last block, inclusive */
    ret_value.id1 = (addr + size - 1) / block_size;
    ret_value.off1 = (addr + size - 1) % block_size;

    return(ret_value);
}

static int find_block_by_id(H5FD_silo_t *file, hsize_t id, unsigned leq)
{
    int bot = 0, top = file->num_blocks - 1, mid;
    silo_vfd_block_t *bl = file->block_list;
    while (bot <= top)
    {
        mid = (bot + top) >> 1;

        if (id > bl[mid].id)
	{
            if (leq)
            {
                if (mid == file->num_blocks-1) return mid;
                if (id < bl[mid+1].id) return mid;
            }
            bot = mid + 1;
	}
        else if (id < bl[mid].id)
	{
            if (leq)
            {
                if (mid == 0) return -1;
                if (id > bl[mid-1].id) return mid-1;
            }
            top = mid - 1;
	}
        else
            return mid;
    }
    return -1; 
}

static int find_block_by_smallest_age(H5FD_silo_t *file)
{
    int i;
    int min_idx = 0;
    silo_vfd_block_t *bl = file->block_list;
    hsize_t min_age = bl[0].age;
    for (i = 0; i < file->num_blocks; i++)
    {
        if (bl[i].age < min_age)
        {
            min_age = bl[i].age;
            min_idx = i;
        }
    }
    return min_idx;
}

static herr_t put_data_to_block_by_index(H5FD_silo_t *file, const void *srcbuf, hsize_t size,
    int blidx, int off)
{
    silo_vfd_block_t *block;
    haddr_t addr;

    HDassert(blidx < file->num_blocks);
    block = &(file->block_list[blidx]);

    HDassert(block->buf);

    HDassert((hsize_t)off+size<=file->block_size);
    memcpy(block->buf+off, srcbuf, size);

    block->dirty = 1;
    block->age = file->op_counter++;

    addr = block->id * file->block_size + off + size;
    if (addr > file->eof) file->eof = addr;

    return 0;
}

static herr_t get_data_from_block_by_index(H5FD_silo_t *file, void *dstbuf, hsize_t size,
    int blidx, int off)
{
    silo_vfd_block_t *block;

    HDassert(blidx < file->num_blocks);
    block = &(file->block_list[blidx]);

    HDassert(block->buf);
    
    HDassert((hsize_t)off+size<=file->block_size);
    memcpy(dstbuf, block->buf+off, size);

    block->age = file->op_counter++;

    return 0;
}

static herr_t file_write(H5FD_silo_t *file, haddr_t addr, size_t size, const void *buf)
{
    static const char  *func = "file_write";
    ssize_t		nbytes;
    herr_t              ret_value = 0;

    HDassert(file && file->pub.cls);
    HDassert(buf);

    H5Eclear2(H5E_DEFAULT);

    /* Check for overflow conditions */
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr undefined", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr overflow", -1, -1)
#if 0
    /* need to account for block being larger than eoa */
    if (addr+size>file->eoa)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr overflow", -1, -1)
#endif

    /* Seek to the correct location */
    if((addr != file->pos || OP_WRITE != file->op) &&
        HDlseek(file->fd, (file_offset_t)addr, SEEK_SET) < 0)
            H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)

    /* Write data, being careful of interrupted system calls and partial results */
    while(size > 0) {
        do {
            nbytes = HDwrite(file->fd, buf, size);
        } while(-1 == nbytes && EINTR == errno);
        if(-1 == nbytes) /* error */
            H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_WRITEERROR, "HDwrite failed", -1, errno)
        HDassert(nbytes > 0);
        HDassert((size_t)nbytes <= size);
        H5_CHECK_OVERFLOW(nbytes, ssize_t, size_t);
        size -= (size_t)nbytes;
        H5_CHECK_OVERFLOW(nbytes, ssize_t, haddr_t);
        addr += (haddr_t)nbytes;
        buf = (const char *)buf + nbytes;
    }

    if (ret_value < 0)
    {
        file->pos = HADDR_UNDEF;
        file->op = OP_UNKNOWN;
    }
    else
    {
        file->pos = addr;
        file->op = OP_WRITE;
        if (file->pos > file->file_eof)
            file->file_eof = file->pos;
    }

    return(ret_value);
}

static herr_t file_read(H5FD_silo_t *file, haddr_t addr, size_t size, void *buf)
{
    static const char  *func = "file_read";
    ssize_t		nbytes;
    herr_t              ret_value = 0;

    HDassert(file && file->pub.cls);
    HDassert(buf);

    H5Eclear2(H5E_DEFAULT);

    /* Check for overflow conditions */
    if (HADDR_UNDEF==addr)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr undefined", -1, -1)
    if (REGION_OVERFLOW(addr, size))
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr overflow", -1, -1)
#if 0
#warning WAS FILE ARROW EOA
    if (addr+size>file->eof)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "addr overflow", -1, -1)
#endif

    /* Seek to the correct location */
    if ((addr != file->pos || OP_READ != file->op) &&
        HDlseek(file->fd, (file_offset_t)addr, SEEK_SET) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDlseek failed", -1, errno)

    /* Read data, careful of interrupted system calls, partial results and eof */
    while(size > 0) {
        do {
            nbytes = HDread(file->fd, buf, size);
        } while(-1 == nbytes && EINTR == errno);
        if(-1 == nbytes) /* error */
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_READERROR, "HDread failed", -1, errno)
        if(0 == nbytes) {
            /* end of file but not end of format address space */
            HDmemset(buf, 0, size);
            break;
        } /* end if */
        HDassert(nbytes >= 0);
        HDassert((size_t)nbytes <= size);
        H5_CHECK_OVERFLOW(nbytes, ssize_t, size_t);
        size -= (size_t)nbytes;
        H5_CHECK_OVERFLOW(nbytes, ssize_t, haddr_t);
        addr += (haddr_t)nbytes;
        buf = (char *)buf + nbytes;
    }

    if (ret_value < 0)
    {
        file->pos = HADDR_UNDEF;
        file->op = OP_UNKNOWN;
    }
    else
    {
        file->pos = addr;
        file->op = OP_READ;
    }

    return(ret_value);
}

static herr_t file_write_block(H5FD_silo_t *file, int blidx)
{
    static const char  *func = "file_write_block";
    silo_vfd_block_t *b = &(file->block_list[blidx]);
    herr_t ret_value = 0;
    haddr_t addr;

    HDassert(b->dirty);
    HDassert(b->buf);

    H5Eclear2(H5E_DEFAULT);

    addr = b->id * file->block_size;

    if (file_write(file, addr, file->block_size, b->buf) < 0)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_WRITEERROR, "file_write_block failed", -1, -1)

    if (ret_value == 0)
        b->dirty = 0;

    return(ret_value);
}

static herr_t file_read_block(H5FD_silo_t *file, int blidx)
{
    static const char  *func = "file_read_block";
    silo_vfd_block_t *b = &(file->block_list[blidx]);
    herr_t ret_value = 0;
    haddr_t addr;

    HDassert(b->buf);

    H5Eclear2(H5E_DEFAULT);

    addr = b->id * file->block_size;

    if (file_read(file, addr, file->block_size, b->buf) < 0)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_READERROR, "file_read_block failed", -1, -1)

    if (ret_value == 0)
        b->dirty = 0;

    return(ret_value);
}

static herr_t remove_block_by_index(H5FD_silo_t *file, int blidx)
{
    int i;
    silo_vfd_block_t *bl = file->block_list;

    HDassert(file->num_blocks>0);

    for (i = blidx; i < file->num_blocks-1; i++)
        bl[i] = bl[i+1];

    file->num_blocks--;

    return 0;
}

/* blidx refers to the block in the list JUST BEFORE the block we're inserting */ 
/* a -1 implies it comes JUST BEFORE the 0th block */
static herr_t insert_block_by_index(H5FD_silo_t *file, int blidx)
{
    int i;
    silo_vfd_block_t *bl = file->block_list;

    HDassert(file->num_blocks<file->max_blocks);

    for (i = file->num_blocks; i > blidx+1; i--)
        bl[i] = bl[i-1];
    memset(&bl[blidx+1], 0, sizeof(silo_vfd_block_t));

    file->num_blocks++;

    return 0;
}

static int alloc_block_by_id(H5FD_silo_t *file, hsize_t id)
{
    haddr_t addr0 = id * file->block_size;
    silo_vfd_block_t *b;
    int blidx = find_block_by_id(file, id, CLOSEST);

    /* blidx refers to the block in the list JUST BEFORE the block we're inserting */ 
    insert_block_by_index(file, blidx);

    /* update blidx to point to the block we're inserting */
    blidx++;

    b = &(file->block_list[blidx]);

    b->buf = malloc(file->block_size);
    b->id = id;
    b->age = file->op_counter++;

#warning MAYBE JUST LESS THAN
    if (addr0<=file->file_eof)
        file_read_block(file, blidx);

    return blidx;
}

static herr_t free_block_by_index(H5FD_silo_t *file, int blidx)
{
    silo_vfd_block_t *b;
    
    HDassert(blidx<file->num_blocks);

    b = &(file->block_list[blidx]);

    HDassert(b->buf);

    if (b->dirty)
        file_write_block(file, blidx);

    free(b->buf);

    remove_block_by_index(file, blidx);

    return 0;
}

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
    static const char *func = "H5FDset_fapl_silo"; 
    herr_t ret_value = 0;
    hsize_t block_size = 16384;
    int block_count = 16;

    H5Eclear2(H5E_DEFAULT);

    if(0 == H5Pisa_class(fapl_id, H5P_FILE_ACCESS))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_BADTYPE, "not a file access property list", -1, -1)
    if (H5Pset(fapl_id, "silo_block_size", &block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_block_size", -1, -1)
    if (H5Pset(fapl_id, "silo_block_count", &block_count) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_block_count", -1, -1)
    if (H5Pset_meta_block_size(fapl_id, block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set meta_block_size", -1, -1)

    return H5Pset_driver(fapl_id, H5FD_SILO, NULL);
}

herr_t
H5Pset_silo_block_size_and_count(hid_t fapl_id, hsize_t block_size, int max_blocks_in_mem)
{
    static const char *func="H5FDset_silo_block_size_and_count";
    herr_t ret_value = 0;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    if(0 == H5Pisa_class(fapl_id, H5P_FILE_ACCESS))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_BADTYPE, "not a file access property list", -1, -1)
    if (H5Pset(fapl_id, "silo_block_size", &block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_block_size", -1, -1)
    if (H5Pset_meta_block_size(fapl_id, block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set meta_block_size", -1, -1)
    if (H5Pset(fapl_id, "silo_block_count", &max_blocks_in_mem) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTSET, "can't set silo_block_count", -1, -1)

    return ret_value;
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
    hsize_t		nbytes = 8; /*size of "LLNLsilo" header*/

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* block size for file */
    nbytes += 8;

    return nbytes;
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_sb_encode
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_sb_encode(H5FD_t *_file, char *name/*out*/,
		     unsigned char *buf/*out*/)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    unsigned char	*p;
    static const char *func = "H5FD_silo_sb_encode";

    H5Eclear2(H5E_DEFAULT);

    /* Name and version number */
    strncpy(name, "LLNLsilo", (size_t)8);
    name[8] = '\0';

    /* Encode block size into sb */
    p = buf+8;
    assert(sizeof(hsize_t)<=8);
    memcpy(p, &file->block_size, sizeof(hsize_t));
    if (H5Tconvert(H5T_NATIVE_HSIZE, H5T_STD_U64LE, 1, buf+8, NULL,
		   H5P_DEFAULT)<0)
        H5Epush_ret(func, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTCONVERT, "can't convert superblock info", -1)

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:	H5FD_silo_sb_decode
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5FD_silo_sb_decode(H5FD_t *_file, const char *name, const unsigned char *buf)
{
    H5FD_silo_t	*file = (H5FD_silo_t*)_file;
    char	       x[8];
    hsize_t           *ap;
    static const char *func = "H5FD_silo_sb_decode";

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* Make sure the name/version number is correct */
    if (strcmp(name, "LLNLsilo"))
        H5Epush_ret(func, H5E_ERR_CLS, H5E_FILE, H5E_BADVALUE, "invalid silo superblock", -1)

    buf += 8;
    /* Decode block size */
    assert(sizeof(hsize_t)<=8);
    memcpy(x, buf, 8);
    if (H5Tconvert(H5T_STD_U64LE, H5T_NATIVE_HSIZE, 1, x, NULL, H5P_DEFAULT)<0)
        H5Epush_ret(func, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTCONVERT, "can't convert superblock info", -1)
    ap = (hsize_t*)x;
    file->block_size = *ap;

    return 0;
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
    int silo_block_count;
    hsize_t silo_block_size;
    hsize_t meta_block_size;
    H5FD_t *ret_value = 0;

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
    if (H5Pget_meta_block_size(fapl_id, &meta_block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get meta_block_size", 0, -1)
    if (H5Pget(fapl_id, "silo_block_size", &silo_block_size) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get silo_block_size", 0, -1)
    if (meta_block_size != silo_block_size)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "meta_block_size!=silo_block_size", 0, -1)
    if (H5Pget(fapl_id, "silo_block_count", &silo_block_count) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "can't get silo_block_count", 0, -1)
    if (silo_block_count < 1)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_PLIST, H5E_CANTGET, "silo_block_count<1", 0, -1)

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
    if(NULL == (file->block_list = (silo_vfd_block_t *)calloc((size_t)silo_block_count, sizeof(silo_vfd_block_t))))
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_RESOURCE, H5E_NOSPACE, "calloc failed", NULL, errno)

    file->fd = fd;
    file->file_eof = (haddr_t)sb.st_size;
#warning IS EOF LAST ADDRESS OR A COUNT
    file->eof = (haddr_t)sb.st_size;
    file->pos = HADDR_UNDEF;
    file->op = OP_UNKNOWN;
    file->write_access = write_access;
    file->block_size = silo_block_size;
    file->max_blocks = silo_block_count;

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

typedef struct silo_vfd_pair_t_ 
{
    int i;
    hsize_t id;
} silo_vfd_pair_t;

static int compare_silo_vfd_pairs(const void *a, const void *b)
{
    silo_vfd_pair_t *paira = (silo_vfd_pair_t*)a;
    silo_vfd_pair_t *pairb = (silo_vfd_pair_t*)b;
    if (paira->id < pairb->id) return 1;
    if (paira->id > pairb->id) return -1;
    return 0;
}

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
    herr_t ret_value = 0;

    /* Clear the error stack */
    H5Eclear2(H5E_DEFAULT);

    /* write any dirty blocks to file */
    if (file->write_access)
    {
        int i;
        silo_vfd_pair_t *sorted_pairs = (silo_vfd_pair_t*) malloc(file->num_blocks*sizeof(silo_vfd_pair_t));
        for (i = 0; i < file->num_blocks; i++)
        {
            sorted_pairs[i].i = i;
            sorted_pairs[i].id = file->block_list[i].id; 
        }
        qsort(sorted_pairs, file->num_blocks, sizeof(silo_vfd_pair_t), compare_silo_vfd_pairs);
        for (i = 0; i < file->num_blocks; i++)
        {
            silo_vfd_block_t *b = &(file->block_list[sorted_pairs[i].i]);
            if (!(b->buf))
                continue;
            if (b->dirty)
                file_write_block(file, sorted_pairs[i].i);
            free(b->buf);
        }
        free(sorted_pairs);
    }

    errno = 0;
    if (close(file->fd) < 0)
        H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_CLOSEERROR, "close failed", -1, errno)

    free(file->block_list);
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
    H5FD_silo_t		*file = (H5FD_silo_t*)_file;
    static const char *func="H5FD_silo_read";  /* Function Name for error reporting */
    herr_t ret_value = 0;
    silo_vfd_block_t *bl;
    silo_vfd_relevant_blocks_t rb;
    hsize_t id, nbytes;
    int blidx, bufoff;

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
    if ((addr+size)>file->eoa)
        H5E_PUSH_HELPER (func, H5E_ERR_CLS, H5E_IO, H5E_OVERFLOW, "file address overflowed", -1, -1)

    if (size == 0)
        return 0;

    rb = relevant_blocks(file->block_size, addr, size);
    blidx = -1; 
    bl = file->block_list;
    bufoff = 0;
    for (id = rb.id0; id <= rb.id1; id++)
    {
        /* look for the block in the list */
        if (blidx < 0 || blidx >= file->num_blocks-1)
            blidx = find_block_by_id(file, id, EXACT); 
        else if (bl[blidx+1].id != id)
            blidx = find_block_by_id(file, id, EXACT); 
        else
            blidx++;

        if (blidx < 0)
        {
            if (file->num_blocks == file->max_blocks)
            {
                int tmpblidx = find_block_by_smallest_age(file);
                free_block_by_index(file, tmpblidx);
            }
            blidx = alloc_block_by_id(file, id);
        }

        /* put the data in the block */
	if (id == rb.id0 && id == rb.id1)
	{
	    get_data_from_block_by_index(file, buf, size, blidx, rb.off0);
	    bufoff += size;
	}
        else if (id == rb.id0)
	{
	    nbytes = file->block_size - rb.off0;
	    get_data_from_block_by_index(file, buf+bufoff, nbytes, blidx, rb.off0);
	    bufoff += nbytes;
	}
	else if (id == rb.id1)
	{
	    nbytes = rb.off1+1;
	    get_data_from_block_by_index(file, buf+bufoff, nbytes, blidx, 0);
	    bufoff += nbytes;
	}
	else
	{
	    get_data_from_block_by_index(file, buf+bufoff, file->block_size, blidx, 0);
	    bufoff += file->block_size;
	}
    }

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
    herr_t ret_value = 0;
    silo_vfd_block_t *bl;
    silo_vfd_relevant_blocks_t rb;
    hsize_t id, nbytes;
    int blidx, bufoff;

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

    if (size == 0)
        return 0;

    rb = relevant_blocks(file->block_size, addr, size);
    blidx = -1; 
    bl = file->block_list;
    bufoff = 0;
    for (id = rb.id0; id <= rb.id1; id++)
    {
        /* look for the block in the list */
        if (blidx < 0 || blidx >= file->num_blocks-1)
            blidx = find_block_by_id(file, id, EXACT); 
        else if (bl[blidx+1].id != id)
            blidx = find_block_by_id(file, id, EXACT); 
        else
            blidx++;

        if (blidx < 0)
        {
            if (file->num_blocks == file->max_blocks)
            {
                int tmpblidx = find_block_by_smallest_age(file);
                free_block_by_index(file, tmpblidx);
            }
            blidx = alloc_block_by_id(file, id);
        }

        /* put the data in the block */
	if (id == rb.id0 && id == rb.id1)
	{
	    put_data_to_block_by_index(file, buf, size, blidx, rb.off0);
	    bufoff += size;
	}
        else if (id == rb.id0)
	{
	    nbytes = file->block_size - rb.off0;
	    put_data_to_block_by_index(file, buf+bufoff, nbytes, blidx, rb.off0);
	    bufoff += nbytes;
	}
	else if (id == rb.id1)
	{
	    nbytes = rb.off1+1;
	    put_data_to_block_by_index(file, buf+bufoff, nbytes, blidx, 0);
	    bufoff += nbytes;
	}
	else
	{
	    put_data_to_block_by_index(file, buf+bufoff, file->block_size, blidx, 0);
	    bufoff += file->block_size;
	}
    }

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
    herr_t ret_value = 0;

    /* Shut compiler up */
    dxpl_id = dxpl_id;
    closing = closing;

#warning SKIPPING TRUNCATE
#warning SKIPPING TRUNCATE
#warning SKIPPING TRUNCATE
#warning SKIPPING TRUNCATE
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

        if(-1 == HDftruncate(file->fd, (file_offset_t)file->eoa))
            H5E_PUSH_HELPER(func, H5E_ERR_CLS, H5E_IO, H5E_SEEKERROR, "HDftruncate failed", -1, errno)
#endif /* _WIN32 */

        /* Update the eof value */
        file->file_eof = file->eoa;

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


/* Use C pre-processor to re-define the 3 public functions for the driver */
#define db_pdb_Open(NAME, MODE, OPTS) db_pdbp_Open(NAME, MODE, OPTS)
#define db_pdb_Create(NAME, MODE, TARGET, OPTS, FINFO) db_pdbp_Create(NAME, MODE, TARGET, OPTS, FINFO)
#define db_pdb_ForceSingle(STATUS) db_pdbp_ForceSingle(STATUS)

/* Use C pre-processor to map all 'lite' PDB symbols to PDB proper symbols */
#define lite_CRAY_STD CRAY_STD
#define lite_IEEEA_STD IEEEA_STD
#define lite_INTELA_ALIGNMENT INTELA_ALIGNMENT
#define lite_M68000_ALIGNMENT M68000_ALIGNMENT
#define lite_MIPS_ALIGNMENT MIPS_ALIGNMENT
#define lite_RS6000_ALIGNMENT RS6000_ALIGNMENT
#define lite_SPARC_ALIGNMENT SPARC_ALIGNMENT
#define lite_UNICOS_ALIGNMENT UNICOS_ALIGNMENT
#define lite_LAST LAST

#define lite_PD_cd PD_cd
#define lite_PD_close PD_close
#define lite_PD_defent_alt PD_defent_alt
#define lite_PD_defstr PD_defstr
#define lite_PD_err PD_err
#define lite_PD_get_attribute PD_get_attribute
#define lite_PD_inquire_entry PD_inquire_entry
#define lite_PD_ls PD_ls
#define lite_PD_mkdir PD_mkdir
#define lite_PD_open PD_open
#define lite_PD_pwd PD_pwd
#define lite_PD_read PD_read
#define lite_PD_read_alt PD_read_alt
#define lite_PD_read_as PD_read_as
#define lite_PD_read_as_alt PD_read_as_alt
#define lite_PD_reset_ptr_list PD_reset_ptr_list
#define lite_PD_target PD_target
#define lite_PD_write PD_write
#define lite_PD_write_alt PD_write_alt

#define lite_SC_alloc SC_alloc
#define lite_SC_arrlen SC_arrlen
#define lite_SC_free SC_free
#define lite_SC_hash_dump SC_hash_dump
#define lite_SC_lookup SC_lookup

/* Ok, now we're ready to include the PDB (lite) driver code. The macros defined
   above will result in renaming the driver's public functions as well as the
   PDB functions it calls to use PDB proper. */
#define PDB_WRITE        /* Include code to write to pdb files */
#define USING_PDB_PROPER /* turn on extra code blocks specific to PDB proper */
#include "silo_pdb.c"

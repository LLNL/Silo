#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CHECK_SYMBOL(A)  if (!strncmp(str, #A, strlen(str))) return A

#define CHECK_SYMBOLN_INT(A)				\
if (!strncmp(tok, #A, strlen(#A)))			\
{							\
    int n = sscanf(tok, #A"=%d", &driver_ints[driver_nints]);\
    if (n == 1)						\
    {							\
        DBAddOption(opts, A, &driver_ints[driver_nints]);\
        driver_nints++;					\
        got_it = 1;					\
    }							\
}

#define CHECK_SYMBOLN_STR(A)				\
if (!strncmp(tok, #A, strlen(#A)))			\
{							\
    driver_strs[driver_nstrs] = strdup(&tok[strlen(#A)]+1);\
    DBAddOption(opts, A, driver_strs[driver_nstrs]);	\
    driver_nstrs++;					\
    got_it = 1;						\
}

#define CHECK_SYMBOLN_SYM(A)				\
if (!strncmp(tok, #A, strlen(#A)))			\
{							\
    driver_ints[driver_nints] = StringToDriver(&tok[strlen(#A)]+1);\
    DBAddOption(opts, A, &driver_ints[driver_nints]);	\
    driver_nints++;					\
    got_it = 1;						\
}


static DBoptlist *driver_opts[] = {0,0,0,0,0,0,0,0,0,0};
static int driver_opts_ids[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static int driver_ints[100];
static int driver_nints = 0;
static char *driver_strs[] = {0,0,0,0,0,0,0,0,0,0};
static int driver_nstrs = 0;
static const int driver_nopts = sizeof(driver_opts)/sizeof(driver_opts[0]);

static void CleanupDriverStuff()
{
    int i;
    for (i = 0; i < driver_nopts; i++)
    {
        if (driver_opts_ids[i] != -1) DBUnregisterFileOptionsSet(driver_opts_ids[i]);
        if (driver_opts[i]) DBFreeOptlist(driver_opts[i]);
    }
    for (i = 0; i < sizeof(driver_strs)/sizeof(driver_strs[0]); i++)
        if (driver_strs[i]) free(driver_strs[i]);
}

static void MakeDriverOpts(DBoptlist **_opts, int *opts_id)
{
    DBoptlist *opts = DBMakeOptlist(30);
    int i;

    for (i = 0; i < driver_nopts; i++)
    {
        if (driver_opts[i] == 0)
        {
            driver_opts[i] = opts;
            break;
        }
    }

    *_opts = opts;
    *opts_id = DBRegisterFileOptionsSet(opts);
     driver_opts_ids[i] = *opts_id;
}

static int StringToDriver(const char *str)
{
    DBoptlist *opts = 0;
    int opts_id = -1;

    CHECK_SYMBOL(DB_PDB);
    CHECK_SYMBOL(DB_PDBP);
    CHECK_SYMBOL(DB_HDF5);
    CHECK_SYMBOL(DB_HDF5_SEC2);
    CHECK_SYMBOL(DB_HDF5_STDIO);
    CHECK_SYMBOL(DB_HDF5_CORE);
    CHECK_SYMBOL(DB_HDF5_SPLIT);
    CHECK_SYMBOL(DB_HDF5_MPIO);
    CHECK_SYMBOL(DB_HDF5_MPIP);
    CHECK_SYMBOL(DB_HDF5_LOG);
    CHECK_SYMBOL(DB_HDF5_DIRECT);
    CHECK_SYMBOL(DB_HDF5_FAMILY);
    CHECK_SYMBOL(DB_HDF5_SILO);
    
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_DEFAULT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SEC2);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_STDIO);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_CORE);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_LOG);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SPLIT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_DIRECT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_FAMILY);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_MPIP);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_MPIO);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SILO);

    CHECK_SYMBOL(DB_H5VFD_DEFAULT);
    CHECK_SYMBOL(DB_H5VFD_SEC2);
    CHECK_SYMBOL(DB_H5VFD_STDIO);
    CHECK_SYMBOL(DB_H5VFD_CORE);
    CHECK_SYMBOL(DB_H5VFD_LOG);
    CHECK_SYMBOL(DB_H5VFD_SPLIT);
    CHECK_SYMBOL(DB_H5VFD_DIRECT);
    CHECK_SYMBOL(DB_H5VFD_FAMILY);
    CHECK_SYMBOL(DB_H5VFD_MPIO);
    CHECK_SYMBOL(DB_H5VFD_MPIP);
    CHECK_SYMBOL(DB_H5VFD_SILO);

    if (!strncmp(str, "DB_HDF5_OPTS(", 13))
    {
        char *tok, *tmpstr;;

        MakeDriverOpts(&opts, &opts_id);

	tmpstr = strdup(&str[13]);
	errno = 0;
	tok = strtok(tmpstr, ",)");

        while (tok)
        {
            int got_it = 0;

            CHECK_SYMBOLN_SYM(DBOPT_H5_VFD)
            CHECK_SYMBOLN_SYM(DBOPT_H5_RAW_FILE_OPTS)
            CHECK_SYMBOLN_STR(DBOPT_H5_RAW_EXTENSION)
            CHECK_SYMBOLN_SYM(DBOPT_H5_META_FILE_OPTS)
            CHECK_SYMBOLN_STR(DBOPT_H5_META_EXTENSION)
            CHECK_SYMBOLN_INT(DBOPT_H5_CORE_ALLOC_INC)
            CHECK_SYMBOLN_INT(DBOPT_H5_CORE_NO_BACK_STORE)
            CHECK_SYMBOLN_INT(DBOPT_H5_META_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SMALL_RAW_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_ALIGN_MIN)
            CHECK_SYMBOLN_INT(DBOPT_H5_ALIGN_VAL)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_MEM_ALIGN)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_BUF_SIZE)
            CHECK_SYMBOLN_STR(DBOPT_H5_LOG_NAME)
            CHECK_SYMBOLN_INT(DBOPT_H5_LOG_BUF_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SIEVE_BUF_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_CACHE_NELMTS)
            CHECK_SYMBOLN_INT(DBOPT_H5_CACHE_NBYTES)
            CHECK_SYMBOLN_INT(DBOPT_H5_FAM_SIZE)
            CHECK_SYMBOLN_SYM(DBOPT_H5_FAM_FILE_OPTS)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_BLOCK_COUNT)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_LOG_STATS)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_USE_DIRECT)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_DEFAULT)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_SEC2)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_STDIO)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_CORE)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_LOG)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_SPLIT)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_DIRECT)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_FAMILY)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_MPIP)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_MPIO)
            CHECK_SYMBOLN_STR(DB_FILE_OPTS_H5_DEFAULT_SILO)

            if (!got_it)
            {
                fprintf(stderr, "Unable to determine driver from string \"%s\"\n", tok);
	        exit(-1);
            }

	    tok = strtok(0, ",)");
	    if (errno != 0)
	    {
                fprintf(stderr, "Unable to determine driver from string \"%s\"\n", tok);
	        exit(-1);
	    }
        }

        free(tmpstr);

        return DB_HDF5_OPTS(opts_id);
    }

    fprintf(stderr, "Unable to determine driver from string \"%s\"\n", str);
    exit(-1);
}

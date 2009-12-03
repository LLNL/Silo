#include <dlfcn.h>
#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <ioperf.h>

typedef struct _options_t
{
    const char *io_interface;
    int request_size_in_bytes;
    int num_requests;
    int initial_file_size;
    int seek_noise;
    int size_noise;
    int test_read;
    ioflags_t flags;
    int print_details;
} options_t;

typedef struct _timeinfo_t
{
    ioop_t op;
    size_t size;
    double t0; /* time started */
    double t1; /* time completed */
} timeinfo_t;

static int GetSizeFromModifierChar(char c)
{
    int n=1;
    switch (c)
    {
        case 'b': case 'B': n=1; break;
        case 'k': case 'K': n=1024; break;
        case 'm': case 'M': n=1024*1024; break;
        case 'g': case 'G': n=1024*1024*1024; break;
        default:  n=1; break;
    }
    return n;
}

/*
 * This is a default timer, with microsecond accuracy or thereabouts.
 * IO Interface plugins can override this default timer if they wish.
 * First call initializes time zero. Succeeding calls return time since
 * first initialized.
 */
static double GetTime()
{
    static double t0 = -1;
    double t1;
    struct timeval tv1;

    if (t0<0)
    {
        struct timeval tv0;
        gettimeofday(&tv0, 0);
        t0 = (double)tv0.tv_sec*1e+6+(double)tv0.tv_usec;
        return 0;
    }

    gettimeofday(&tv1, 0);
    t1 = (double)tv1.tv_sec*1e+6+(double)tv1.tv_usec;

    return t1-t0;
}

static void ProcessCommandLine(int argc, char *argv[], options_t *opts)
{
    char plugin_opts_delim[256];
    int i,n,nerrors=0;
    for (i=1; i<argc; i++)
    {
        errno=0;
        if (!strcmp(argv[i], "--io-interface"))
        {
            i++;
            opts->io_interface = strdup(argv[i]);
            sprintf(plugin_opts_delim, "--%s-args", opts->io_interface);
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--request-size"))
        {
            i++;
            n=strlen(argv[i])-1;
            n=GetSizeFromModifierChar(argv[i][n]);
            opts->request_size_in_bytes = strtol(argv[i], (char **)NULL, 10)*n;
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--num-requests"))
        {
            i++;
            opts->num_requests = strtol(argv[i], (char **)NULL, 10);
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--seek-noise"))
        {
            i++;
            opts->seek_noise = strtol(argv[i], (char **)NULL, 10);
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--size-noise"))
        {
            i++;
            opts->size_noise = strtol(argv[i], (char **)NULL, 10);
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--initial-size"))
        {
            i++;
            n=strlen(argv[i])-1;
            n=GetSizeFromModifierChar(argv[i][n]);
            opts->initial_file_size = strtol(argv[i], (char **)NULL, 10)*n;
            if (errno) goto fail;
        }
        else if (!strcmp(argv[i], "--test-read"))
        {
            opts->flags = IO_READ;
        }
        else if (!strcmp(argv[i], "--print-details"))
        {
            opts->print_details = 1;
        }
        else if (!strcmp(argv[i], plugin_opts_delim))
        {
            break;
        }
        else
        {
fail:
	    fprintf(stderr, "%s: bad argument `%s' (\"%s\")\n", argv[0], argv[i],
                errno?strerror(errno):"");
            exit(1);
	}
    }

    /* sanity check some values */
    if (opts->io_interface == 0)
        fprintf(stderr, "%d: no io-interface specified\n", nerrors++);
    if (nerrors)
        exit(nerrors);
}

#ifdef STATIC_PLUGINS
extern iointerface_t* CreateInterface_silo(int argc, char *argv[], const char *filename);
extern iointerface_t* CreateInterface_hdf5(int argc, char *argv[], const char *filename);
#endif

static iointerface_t* GetIOInterface(int argc, char *argv[], const char *ifacename)
{
    char testfilename[256];
    void *dlhandle=0;
    iointerface_t *retval=0;

    /* First, get rid of the old data file */
    sprintf(testfilename, "iop_test_%s.dat", ifacename);
    unlink(testfilename);

    /* First, attempt to create interface using static approach, if that
       is enabled. */
#ifdef STATIC_PLUGINS
    if (!strcmp(ifacename, "silo"))
        retval = CreateInterface_silo(argc, argv, testfilename);
    else if (!strcmp(ifacename, "hdf5"))
        retval = CreateInterface_hdf5(argc, argv, testfilename);
#else
    /* Fall back to dynamic approach */
    if (!retval)
    {
        char libfilename[256];
        sprintf(libfilename, "./libiop_%s.so", ifacename);
        dlhandle = dlopen(libfilename, RTLD_LAZY);
        if (dlhandle)
        {
            CreateInterfaceFunc createFunc = (CreateInterfaceFunc) dlsym(dlhandle, "CreateInterface");
            if (!createFunc)
            {
                fprintf(stderr,"Encountered dlsym error \"%s\"\n", dlerror());
                exit(1);
            }

            /* we allow the io-interface plugin to process command line args too */
            retval = createFunc(argc, argv, testfilename);

        }
        else
        {
            fprintf(stderr,"Encountered dlopen error \"%s\"\n", dlerror());
            exit(1);
        }
    }
#endif

    if (!retval)
    {
        fprintf(stderr,"Encountered error instantiating IO interface\n");
        exit(1);
    }

    /* store off the handle to this plugin so we can close it later */
    retval->dlhandle = dlhandle;

    /* slip in the default time function */
    if (!retval->Time)
        retval->Time = GetTime;

    return retval;
}

static void AddTimingInfo(ioop_t op, size_t size, double t0, double t1)
{
    static timeinfo_t *tinfo=0;
    static int i=0;
    static int max=100;

    if (op == OP_OUTPUT_TIMINGS || op == OP_OUTPUT_SUMMARY)
    {
        int j;
        const char *opnms[] = {"WRITE", "READ", "OPEN", "CLOSE", "SEEK", "ERROR"};
        double tottime=0, totwrtime=0, totrdtime=0, toterrtime=0;
        size_t totwrbytes=0, totrdbytes=0;
        double wrfastest=0, wrslowest=DBL_MAX, rdfastest=0, rdslowest=DBL_MAX;
        int wrfastesti=-1, wrslowesti=-1, rdfastesti=-1, rdslowesti=-1;
        double wravg, rdavg;

        if (op == OP_OUTPUT_TIMINGS)
            fprintf(stdout, "i\top\tt0\t\tt1\t\tsize\n");
        for (j=0; j<i; j++)
        {
            if (op == OP_OUTPUT_TIMINGS)
            {
                fprintf(stdout, "%d\t%s\t%f\t%f\t%zd\n",
                    j,opnms[tinfo[j].op],tinfo[j].t0,tinfo[j].t1,tinfo[j].size);
            }

            if (tinfo[j].op != OP_ERROR)
            {
                tottime += (tinfo[j].t1-tinfo[j].t0);
            }
            else
            {
                toterrtime += (tinfo[j].t1-tinfo[j].t0);
            }

            if (tinfo[j].op == OP_WRITE)
            {
                double wrtime = tinfo[j].t1-tinfo[j].t0;
                double wrspeed = wrtime?(tinfo[j].size/wrtime):0;
                totwrtime += wrtime;
                totwrbytes += tinfo[j].size;
                if (wrspeed > wrfastest)
                {
                    wrfastest = wrspeed;
                    wrfastesti = j;
                }
                if (wrspeed < wrslowest)
                {
                    wrslowest = wrspeed;
                    wrslowesti = j;
                }
            }
            else if (tinfo[j].op == OP_READ)
            {
                double rdtime = tinfo[j].t1-tinfo[j].t0;
                double rdspeed = rdtime?(tinfo[j].size/rdtime):0;
                totrdtime += rdtime;
                totrdbytes += tinfo[j].size;
                if (rdspeed > rdfastest)
                {
                    rdfastest = rdspeed;
                    rdfastesti = j;
                }
                if (rdspeed < rdslowest)
                {
                    rdslowest = rdspeed;
                    rdslowesti = j;
                }
            }
        }

        fprintf(stdout, "=============================================================\n");
        fprintf(stdout, "==========================Summary============================\n");
        fprintf(stdout, "=============================================================\n");
        if (totwrbytes)
        {
            fprintf(stdout, "**************************Writes****************************\n");
            fprintf(stdout, "Total:   %zd bytes in %f seconds = %f Mb/s\n",
                totwrbytes, tottime*1e-6, totwrbytes/(tottime*1e-6)/(1<<20));
            fprintf(stdout, "Average: %zd bytes in %f seconds = %f Mb/s\n",
                totwrbytes, totwrtime*1e-6, totwrbytes/(totwrtime*1e-6)/(1<<20));
            if (wrfastesti>=0)
                fprintf(stdout, "Fastest: %zd bytes in %f seconds = %f Mb/s (iter=%d)\n",
                tinfo[wrfastesti].size, (tinfo[wrfastesti].t1-tinfo[wrfastesti].t0)*1e-6,
                wrfastest*1e+6/(1<<20), wrfastesti);
            if (wrslowesti>=0)
                fprintf(stdout, "Slowest: %zd bytes in %f seconds = %f Mb/s (iter=%d)\n",
                tinfo[wrslowesti].size, (tinfo[wrslowesti].t1-tinfo[wrslowesti].t0)*1e-6,
                wrslowest*1e+6/(1<<20), wrslowesti);
        }
        if (totrdbytes)
        {
            fprintf(stdout, "**************************Reads*****************************\n");
            fprintf(stdout, "Total:   %zd bytes in %f seconds = %f Mb/s\n",
                totrdbytes, tottime*1e-6, totrdbytes/(tottime*1e-6)/(1<<20));
            fprintf(stdout, "Average: %zd bytes in %f seconds = %f Mb/s\n",
                totrdbytes, totrdtime*1e-6, totrdbytes/(totrdtime*1e-6)/(1<<20));
            if (rdfastesti>=0)
                fprintf(stdout, "Fastest: %zd bytes in %f seconds = %f Mb/s (iter=%d)\n",
                tinfo[rdfastesti].size, (tinfo[rdfastesti].t1-tinfo[rdfastesti].t0)*1e-6,
                rdfastest*1e+6/(1<<20),rdfastesti);
            if (rdslowesti>=0)
                fprintf(stdout, "Slowest: %zd bytes in %f seconds = %f Mb/s (iter=%d)\n",
                tinfo[rdslowesti].size, (tinfo[rdslowesti].t1-tinfo[rdslowesti].t0)*1e-6,
                rdslowest*1e+6/(1<<20), rdslowesti);
        } 

        return;
    }

    if (tinfo==0 || i==max-1)
    {
        max = max*1.5;
        tinfo = (timeinfo_t*) realloc(tinfo, max*sizeof(timeinfo_t));
    }

    tinfo[i].op = op;
    tinfo[i].t0 = t0;
    tinfo[i].t1 = t1;
    tinfo[i].size = size;
    i++;
}

static void TestWrites(iointerface_t *ioiface, const options_t *opts)
{
    int i,n;
    double t0,t1;
    double *buf;
    int num_doubles;

    /* allocate and initialize a buffer of data to write */
    num_doubles = opts->request_size_in_bytes / sizeof(double);
    buf = (double*) calloc(opts->request_size_in_bytes,1);
    for (i=0; i<num_doubles;i++)
        buf[i] = i;

    for (i=0; i<opts->num_requests; i++)
    {
        if (opts->size_noise && i && (i%(opts->size_noise))==0)
        {
            t0 = ioiface->Time();
            n = ioiface->Write(buf, 8);
            t1 = ioiface->Time();
            AddTimingInfo(n==8?OP_WRITE:OP_ERROR, n, t0, t1);
        }
        t0 = ioiface->Time();
        n = ioiface->Write(buf, opts->request_size_in_bytes);
        t1 = ioiface->Time();
        AddTimingInfo(n==opts->request_size_in_bytes?OP_WRITE:OP_ERROR, n, t0, t1);
    }
}

int
main(int argc, char *argv[])
{
    options_t      options;
    iointerface_t  *ioiface;
    double         t0,t1;

    /* setup default options */
    options.io_interface = 0;
    options.request_size_in_bytes = 4096;
    options.initial_file_size = 0;
    options.num_requests = 100;
    options.seek_noise = 0;
    options.size_noise = 0;
    options.flags = IO_WRITE|IO_TRUNCATE;
    options.print_details = 0;

    ProcessCommandLine(argc, argv, &options);

    /* GetIOInterface either exits or returns valid pointer */
    ioiface = GetIOInterface(argc, argv, options.io_interface);

    /* First call initializes timer */
    t0 = ioiface->Time();

    /* open the file */
    if (!ioiface->Open(0))
    {
        fprintf(stderr, "Problem opening file\n");
        exit(1);
    }
    t1 = ioiface->Time();
    AddTimingInfo(OP_OPEN, 0, t0, t1);

    if (options.flags&IO_WRITE)
        TestWrites(ioiface, &options);

/*
    if (options.flags&IO_READ)
        TestReads(ioiface, &options);
*/
        
    /* close the file */
    t0 = ioiface->Time();
    if (!ioiface->Close())
    {
        fprintf(stderr, "Problem closing file\n");
    }
    t1 = ioiface->Time();
    AddTimingInfo(OP_CLOSE, 0, t0, t1);

#ifndef STATIC_PLUGINS
    /* close the interface */
    if (ioiface->dlhandle)
        dlclose(ioiface->dlhandle);
#endif
    
    /* output timing info */
    if (options.print_details)
        AddTimingInfo(OP_OUTPUT_TIMINGS, 0, 0, 0);
    else
        AddTimingInfo(OP_OUTPUT_SUMMARY, 0, 0, 0);

    return (0);
}

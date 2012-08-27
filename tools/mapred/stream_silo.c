#include <silo.h>

#include <map>
#include <string>
#include <iostream>

#include <assert.h>
#include <string.h>

using std::map;
using std::string;
using std::cout;
using std::endl;
using std::cerr;

typedef map<string, string> strmap_t;

#define ARG_OPT(A) (!strncmp(argv[i],#A,sizeof(#A)))

static void get_coord(DBucdmesh const *ucdm, int nid, double* x, double* y, double* z)
{
    if (ucdm->datatype == DB_DOUBLE)
    {
        *x = ((double*)(ucdm->coords[0]))[nid];
        *y = ((double*)(ucdm->coords[1]))[nid];
        *z = ((double*)(ucdm->coords[2]))[nid];
    }
    else if (ucdm->datatype == DB_FLOAT)
    {
        *x = ((float*)(ucdm->coords[0]))[nid];
        *y = ((float*)(ucdm->coords[1]))[nid];
        *z = ((float*)(ucdm->coords[2]))[nid];
    }
    else
    {
        assert(0);
    }
}

static unsigned long long get_zoneid(DBzonelist const *zl, int zidx)
{
    if (!zl->gzoneno) return (unsigned long long)-1;

    switch (zl->gnznodtype)
    {
        case DB_INT: {int *p = (int *) zl->gzoneno; return (unsigned long long) p[zidx]; }
        case DB_LONG: { long *p = (long *) zl->gzoneno; return (unsigned long long) p[zidx]; }
        case DB_LONG_LONG: { long long *p = (long long *) zl->gzoneno; return (unsigned long long) p[zidx]; }
        default: assert(0);
    }
}

static double get_var_value(void *vdata, int datatype, int idx)
{
    switch (datatype)
    {
        case DB_INT: {int *p = (int*)vdata; return p[idx];} 
        case DB_FLOAT: {float *p = (float*)vdata; return p[idx];} 
        case DB_DOUBLE: {double *p = (double*)vdata; return p[idx];} 
        default: assert(0);
    }
}

static string genkey(DBucdmesh const* ucdm, string const& prefix, int dom, int zidx)
{
    unsigned long long gzoneno = get_zoneid(ucdm->zones, zidx);
    char tmp[256];
    if (gzoneno != (unsigned long long)-1)
    {
        if (dom == -1)
            sprintf(tmp, "%llx:%d", gzoneno, zidx);
        else
            sprintf(tmp, "%llx:%d:%d", gzoneno, dom, zidx);
    }
    else
    {
        if (dom == -1)
            sprintf(tmp, "%d", zidx);
        else
            sprintf(tmp, "%d:%d", dom, zidx);
    }
    string zkey = prefix + ":" + string(tmp); 
    return zkey;
}

static double retvals[256];
static double *get_var_values(void **vdata, int nvars, int datatype, int idx)
{
    for (int i = 0; i < nvars; i++)
        retvals[i] = get_var_value(vdata[i], datatype, idx);
    return retvals;
}

static void stream_ucdvar(DBucdmesh const *ucdm, DBucdvar const *ucdv, string const& prefix, int dom, strmap_t& fmap)
{
    if (ucdv->centering == DB_ZONECENT)
    {
        for (int i = 0; i < ucdm->zones->nzones; i++)
        {
            string zkey = genkey(ucdm, prefix, dom, i);
            double *vals = get_var_values(ucdv->vals, ucdv->nvals, ucdv->datatype, i);
            for (int j = 0; j < ucdv->nvals; j++)
            {
                char tmp[256];
                sprintf(tmp, "%s%1d=%f", ucdv->name, j, vals[j]);
                fmap[zkey] += string(tmp);
                if (j < ucdv->nvals-1) fmap[zkey] += ";";
            }
            fmap[zkey] += ":";
        }
    }
    else if (ucdv->centering == DB_NODECENT)
    {
        int zlidx = 0;
        int zidx = 0;
        DBzonelist const *zl = ucdm->zones;
        for (int i = 0; i < zl->nshapes; i++)
        {
            const int shapecnt = zl->shapecnt[i];
            const int shapesize = zl->shapesize[i];
            const int shapetype = zl->shapetype[i];
            for (int j = 0; j < shapecnt; j++)
            {
                char tmp[256];
                string zkey = genkey(ucdm, prefix, dom, zidx);
                for (int k = 0; k < shapesize; k++)
                {
                    int nodeid = zl->nodelist[zlidx+k];
                    double *vals = get_var_values(ucdv->vals, ucdv->nvals, ucdv->datatype, nodeid);
                    for (int j = 0; j < ucdv->nvals; j++)
                    {
                        char tmp[256];
                        sprintf(tmp, "%s%1d=%f", ucdv->name, j, vals[j]);
                        fmap[zkey] += string(tmp);
                        if (j < ucdv->nvals-1) fmap[zkey] += ";";
                    }
                    if (k < shapesize-1) fmap[zkey] += "$";
                }
                zlidx += shapesize;
                zidx++;
                fmap[zkey] += ":";
            }
        }
    }
    else
    {
        assert(0);
    }
}

static void stream_ucdmesh(DBucdmesh const *ucdm, string const& prefix, int dom, strmap_t& fmap)
{
    int zlidx = 0;
    int zidx = 0;
    DBzonelist const *zl = ucdm->zones;
    for (int i = 0; i < zl->nshapes; i++)
    {
        const int shapecnt = zl->shapecnt[i];
        const int shapesize = zl->shapesize[i];
        const int shapetype = zl->shapetype[i];
        for (int j = 0; j < shapecnt; j++)
        {
            // Zone key section
            char tmp[256];
            string zkey = genkey(ucdm, prefix, dom, zidx);
            fmap[zkey] = "";
            // nodes  section (probaly not necessary)
            for (int k = 0; k < shapesize; k++)
            {
                int nodeid = zl->nodelist[zlidx+k];
                sprintf(tmp, "n%1d=%d", k, nodeid);
                fmap[zkey] += string(tmp);
                if (k < shapesize-1) fmap[zkey] += ":";
            }
            fmap[zkey] += "|";
            // coordinates section
            for (int k = 0; k < shapesize; k++)
            {
                int nodeid = zl->nodelist[zlidx+k];
                double x, y, z;
                get_coord(ucdm, nodeid, &x, &y, &z);
                sprintf(tmp, "x%1d=%f$y%1d=%f$z%1d=%f", k, x, k, y, k, z);
                fmap[zkey] += string(tmp);
                if (k < shapesize-1) fmap[zkey] += ":";
            }
            fmap[zkey] += "|";
            zlidx += shapesize;
            zidx++;
        }
    }
}

static void stream_cwdir(DBfile *dbfile, string prefix, int dom, strmap_t &fmap)
{
    DBtoc *toc = DBGetToc (dbfile);

    for (int i = 0; i < toc->nucdmesh; i++)
    {
        DBucdmesh *m = DBGetUcdmesh(dbfile, toc->ucdmesh_names[i]);
        string newprefix = prefix + "/" + string(toc->ucdmesh_names[i]);
        stream_ucdmesh(m, newprefix, dom, fmap);
        long oldmask = DBGetDataReadMask();
        for (int j = 0; j < toc->nucdvar; j++)
        {
            DBSetDataReadMask(DBNone); 
            DBucdvar *v = DBGetUcdvar(dbfile, toc->ucdvar_names[j]);
            if (!strcmp(v->meshname, toc->ucdmesh_names[i]))
            {
                DBFreeUcdvar(v);
                DBSetDataReadMask(DBAll); 
                v = DBGetUcdvar(dbfile, toc->ucdvar_names[j]);
                stream_ucdvar(m, v, newprefix, dom, fmap);
                DBFreeUcdvar(v);
            }
        }
        DBSetDataReadMask(oldmask);
        DBFreeUcdmesh(m);
    }

    if (!toc->ndir) return;

    //
    // Copy relevant info from the toc. Otherwise, it'll get lost on
    // successive calls to DBSetDir().
    //
    int      norigdir      = toc->ndir;
    char   **origdir_names = new char*[norigdir];
    for (int i = 0 ; i < norigdir ; i++)
    {
        origdir_names[i] = new char[strlen(toc->dir_names[i])+1];
        strcpy(origdir_names[i], toc->dir_names[i]);
    }

    for (int i = 0; i <norigdir; i++)
    {
        int newdom;
        int n = sscanf(origdir_names[i], "domain_%d", &newdom);
        if (n != 1)
            n = sscanf(origdir_names[i], "block%d", &newdom);
        if (n != 1) continue;
        string new_prefix = prefix + "/" + string(origdir_names[i]);
        DBSetDir(dbfile, origdir_names[i]);
        stream_cwdir(dbfile, new_prefix, newdom, fmap);
        DBSetDir(dbfile, "..");
    }

    for (int i = 0; i <norigdir; i++)
        delete [] origdir_names[i];
    delete [] origdir_names;

}

static void stream_file(char const *filename)
{
    DBfile *dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ);
    strmap_t stream_map;
    stream_cwdir(dbfile, "", -1, stream_map);

    for (strmap_t::const_iterator cit = stream_map.begin(); cit != stream_map.end(); cit++)
        cout << cit->first << "\t" << cit->second << endl;
}

int main(int argc, char **argv)
{
    char filename[1024];
    char varname[256];

    filename[0] = '\0';
    varname[0] = '\0';
    for (int i = 0; i < argc; i++)
    {
        if (ARG_OPT(--fn))
        {
            i++;
            assert(strlen(argv[i])<sizeof(filename));
            strcpy(filename, argv[i]);
        }
        else if (ARG_OPT(--var))
        {
            i++;
            assert(strlen(argv[i])<sizeof(varname));
            strcpy(varname, argv[i]);
        }
    }

    if (!strcmp(filename, ""))
    {
        cerr << "filename not specified" << endl;
        exit(1);
    }

    stream_file(filename);

    return 0;
}

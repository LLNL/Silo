//
// todo:
//
// 1) (leak)  still need to free some memory....  (strings only, not too bad)
//                                                (oh, facelists too...)
// 2) (opt)   do nodeset reading only once
// 3) (opt)   do distfact reading only once
// 4) (opt)   any more sideset stuff can be done once?
// 5) (genl)  allow displacement vector components passed in on command line
// 7) (clean) maybe put sideset face/zonelists into a root level "sideset" subdirectory?
// 8) (genl)  finish shape types (e.g. sphere)

// notes:
// 1) had to dumb down for stupid compilers and STL implementations
//    a) no default template parameters (e.g. for map<>)
//    b) it-> is not defined for iterators, must use (*it). instead
//    c) no string::npos, must use NPOS and #define it for smart compilers
//    d) no resize() or clear() in vector, and an incorrect insert()
//    e) no empty() in string
//    f) no logic_error standard exception, so i removed ALL std exceptions

#include <exodusII.h>
#include <silo.h>

#ifndef DUMB_COMPILER

#include <vector>
#include <algorithm>
#include <string>
#include <map>
using namespace std;
#ifndef NPOS
#define NPOS string::npos
#endif

#else // dumb compiler

#include <vector.h>
#include <algo.h>
#include <bstring.h>
#include <map.h>
using namespace std;

#endif // dump compiler

#include <string.h>
#include <iostream.h>
#include <fstream.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

// dumb compiler functions
template <class T>
inline void vec_clear(vector<T> &v)
{
    while (!v.empty())
        v.erase(v.begin());
}

template <class T>
inline void vec_resize(vector<T> &v, int n)
{
    T t;
    vec_clear(v);
    for (int i=0;i<n;i++)
        v.push_back(t);
}

template <class T, class U>
inline void vec_insert_at_end(vector<T> &v, U a, U b )
{
    for ( ; a!=b; a++)
        v.push_back(*a);
}
// end dumb compiler functions


// -- exceptions
class LogicException {
  private:
    char *text;
  public:
    LogicException(char *text_) : text(text_) {};
    virtual char *what() {return text;};
};


class ExodusException {
  private:
    char *text;
  public:
    ExodusException(char *text_) : text(text_) {};
    virtual char *what() {return text;};
};

class SiloException {
  private:
    char *text;
  public:
    SiloException(char *text_) : text(text_) {};
    virtual char *what() {return text;};
};

// -- global
namespace {
    int  debug_level = 1;
    bool do_nodesets = true;
    bool do_sidesets = true;
    bool use_sel_vars = false;
    vector<string> sel_vars;
    vector<bool>   sel_vars_z;
    vector<bool>   sel_vars_n;

#ifdef PARALLEL
    bool  ui_process;
    int   n_proc;
    int   rank;
#endif

    int   CPU_word_size;
    int   IO_word_size;

    float           displ_scale = 1.0;

    bool            use_given_rootname = false;
    bool            use_given_directory = false;
    string          out_rootname;
    string          out_directory;

    bool            info_only = false;

    bool            use_cstart = false;
    bool            use_cstop = false;
    int             cstart;
    int             cstop;
    bool            use_tstart = false;
    bool            use_tstop = false;
    float           tstart;
    float           tstop;

    int             n_files;
    bool            singlefile;
    char            problem_title[4096];
    int             ndims;
    int             ntimes;
    bool            timeless;
    vector<float>   ftimes;
    int             n_vars_n;
    int             n_vars_z;
    vector<char*>   var_names_n;
    vector<char*>   var_names_z;
    vector<string>  var_names;

    vector<map<string,vector<string>,less<string> > >    m_mesh;
    vector<map<string,vector<int>,less<string> > >       m_meshtype;
    vector<map<string,vector<string>,less<string> > >    m_mat;
    vector<map<string,vector<string>,less<string> > >    m_var;
    vector<map<string,vector<int>,less<string> > >       m_vartype;

    map<string,int,less<string> >               element_num_nodes;
    map<string,int*,less<string> >              element_node_map;
    map<string,int,less<string> >               element_silo_type;
    map<string,vector<int*>,less<string> >      element_side_map_nodes;
    map<string,int*,less<string> >              element_side_map_counts;
    map<string,int*,less<string> >              element_side_map_types;

    char silo_defvar[4096];

    //int node_map_circle[]  = {1};
    //int node_map_sphere[]  = {1};
    int node_map_truss[]   = {1, 2};
    int node_map_beam[]    = {1, 2};
    int node_map_shell2[]  = {1, 2};
    int node_map_quad[]    = {1, 2, 3, 4};
    int node_map_shell3[]  = {1, 2, 3, 4};
    int node_map_triangle[]= {1, 2, 3};
    int node_map_tetra[]   = {1, 2, 4, 3};
    int node_map_wedge[]   = {4, 1, 2, 5, 6, 3};
    int node_map_hex[]     = {4, 3, 2, 1, 8, 7, 6, 5};

    //int side_map_circle_1[]  = ??;
    //int side_map_sphere_1[]  = ??;
    //int side_map_truss_1[]   = ??;
    //int side_map_beam_1[]    = ??
    //int side_map_shell2_1[]  = ??
    int side_map_quad_1[]     = {1, 2};
    int side_map_quad_2[]     = {2, 3};
    int side_map_quad_3[]     = {3, 4};
    int side_map_quad_4[]     = {4, 1};
    int side_map_shell3_1[]   = {1, 2, 3, 4};
    int side_map_shell3_2[]   = {4, 3, 2, 1};
    int side_map_shell3_3[]   = {1, 2};
    int side_map_shell3_4[]   = {2, 3};
    int side_map_shell3_5[]   = {3, 4};
    int side_map_shell3_6[]   = {4, 1};
    int side_map_triangle_1[] = {1, 2};
    int side_map_triangle_2[] = {2, 3};
    int side_map_triangle_3[] = {3, 1};
    int side_map_tetra_1[]    = {1, 2, 4};
    int side_map_tetra_2[]    = {2, 3, 4};
    int side_map_tetra_3[]    = {3, 1, 4};
    int side_map_tetra_4[]    = {1, 3, 2};
    int side_map_wedge_1[]    = {1, 2, 5, 4};
    int side_map_wedge_2[]    = {2, 3, 6, 5};
    int side_map_wedge_3[]    = {3, 1, 4, 6};
    int side_map_wedge_4[]    = {3, 2, 1};
    int side_map_wedge_5[]    = {4, 5, 6};
    int side_map_hex_1[]      = {1, 2, 6, 5};
    int side_map_hex_2[]      = {2, 3, 7, 6};
    int side_map_hex_3[]      = {3, 4, 8, 7};
    int side_map_hex_4[]      = {4, 1, 5, 8};
    int side_map_hex_5[]      = {4, 3, 2, 1};
    int side_map_hex_6[]      = {5, 6, 7, 8};

    //int side_map_circle_c[]  = ??;
    //int side_map_sphere_c[]  = ??;
    //int side_map_truss_c[]   = ??;
    //int side_map_beam_c[]    = ??;
    //int side_map_shell2_c[]  = ??;
    int side_map_quad_c[]    = {2, 2, 2, 2};
    int side_map_shell3_c[]  = {4, 4, 2, 2, 2, 2};
    int side_map_triangle_c[]= {2, 2, 2};
    int side_map_tetra_c[]   = {3, 3, 3, 3};
    int side_map_wedge_c[]   = {4, 4, 4, 3, 3};
    int side_map_hex_c[]     = {4, 4, 4, 4, 4, 4};

    //int side_map_circle_t[]  = ??;
    //int side_map_sphere_t[]  = ??;
    //int side_map_truss_t[]   = ??;
    //int side_map_beam_t[]    = ??;
    //int side_map_shell2_t[]  = ??;
    int side_map_quad_t[]    = {DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
    int side_map_shell3_t[]  = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
    int side_map_triangle_t[]= {DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
    int side_map_tetra_t[]   = {DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE};
    int side_map_wedge_t[]   = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE};
    int side_map_hex_t[]     = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD};

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
make_silo_friendly(char *s_)
{
    char *s = s_;
    for (; *s; s++)
    {
        if ((*s<'A' || *s>'Z') && 
            (*s<'a' || *s>'z') &&
            (s==s_ || (*s<'0' || *s>'9')))
        {
            *s = '_';
        }
        else if (*s>='A' && *s<='Z') // convert to lower case
        {
            *s += 'a' - 'A';
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
PutMultimesh(DBfile *db, const string &fullname, const int n,
             const vector<string> names, vector<int> types,
             DBoptlist *opt)
{
    if (debug_level >= 2)
        cout << "Putting multi_mesh " << fullname << endl;

    if (n==0)
    {
        cout << "Warning: empty mesh: " << fullname << endl;
        return;
    }

    vector<const char*> c_names;
    c_names.reserve(n);

    for (int j=0; j < n; j++)
        c_names.push_back(names[j].c_str());

    int nsubdir = 0;

    string name(fullname);
    while (name.find('/') != NPOS)
    {
        int p = name.find('/');
        string subdir = name.substr(0,p);
        name = name.substr(p+1);
        DBShowErrors(DB_NONE,NULL);
        DBMkDir(db, (char*)subdir.c_str());
        DBShowErrors(DB_TOP,NULL);
        DBSetDir(db, (char*)subdir.c_str());
        nsubdir++;
    }

    DBPutMultimesh(db, (char*)name.c_str(), n,
                   (char**)&c_names[0], &types[0], opt);

    while (nsubdir)
    {
        DBSetDir(db,"..");
        nsubdir--;
    }
}


void
PutMultivar(DBfile *db, const string &fullname, const int n,
            const vector<string> names, vector<int> types,
            DBoptlist *opt)
{
    if (debug_level >= 2)
        cout << "Putting multi var " << fullname << endl;

    if (n==0)
    {
        if (! (fullname.substr(fullname.length()-8) == "distfact"))
            cout << "Warning: empty variable: " << fullname << endl;
        return;
    }

    vector<const char*> c_names;
    c_names.reserve(n);

    for (int j=0; j < n; j++)
        c_names.push_back(names[j].c_str());

    int nsubdir = 0;

    string name(fullname);
    while (name.find('/') != NPOS)
    {
        int p = name.find('/');
        string subdir = name.substr(0,p);
        name = name.substr(p+1);
        DBShowErrors(DB_NONE,NULL);
        DBMkDir(db, (char*)subdir.c_str());
        DBShowErrors(DB_TOP,NULL);
        DBSetDir(db, (char*)subdir.c_str());
        nsubdir++;
    }

    DBPutMultivar(db, (char*)name.c_str(), n,
                  (char**)&c_names[0], &types[0], opt);

    while (nsubdir)
    {
        DBSetDir(db,"..");
        nsubdir--;
    }
}

void
PutMultimat(DBfile *db, const string &fullname, const int n,
             const vector<string> names, DBoptlist *opt)
{
    if (debug_level >= 2)
        cout << "Putting multi mat " << fullname << endl;

    if (n==0)
    {
        cout << "Warning: empty material: " << fullname << endl;
        return;
    }

    vector<const char*> c_names;
    c_names.reserve(n);

    for (int j=0; j < n; j++)
        c_names.push_back(names[j].c_str());

    int nsubdir = 0;

    string name(fullname);
    while (name.find('/') != NPOS)
    {
        int p = name.find('/');
        string subdir = name.substr(0,p);
        name = name.substr(p+1);
        DBShowErrors(DB_NONE,NULL);
        DBMkDir(db, (char*)subdir.c_str());
        DBShowErrors(DB_TOP,NULL);
        DBSetDir(db, (char*)subdir.c_str());
        nsubdir++;
    }

    DBPutMultimat(db, (char*)name.c_str(), n,
                  (char**)&c_names[0], opt);

    while (nsubdir)
    {
        DBSetDir(db,"..");
        nsubdir--;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
calc_defvars(vector<string> &vars)
{
    sort(vars.begin(),vars.end());

    silo_defvar[0] = '\0';

    int i=0;
    int l=vars.size();
    while (i<l)
    {
        if (ndims==3  &&  i+2 < l)
        {
            int l1=vars[i  ].length();
            int l2=vars[i+1].length();
            int l3=vars[i+2].length();
            int c1=vars[i  ][l1-1];
            int c2=vars[i+1][l2-1];
            int c3=vars[i+2][l3-1];

            if ((c1=='x' && c2=='y' && c3=='z') &&
                vars[i].substr(0,l1-1) == vars[i+1].substr(0,l2-1) &&
                vars[i].substr(0,l1-1) == vars[i+1].substr(0,l3-1))
            {
                string s(vars[i].substr(0,l1-1) +
                         " vector {" +
                         vars[i]     + "," +
                         vars[i+1]   + "," +
                         vars[i+2]   + "};" );

                strcat(silo_defvar, s.c_str());
                i+=3;
                continue;
            }
        }
        if (ndims==2  &&  i+1 < l)
        {
            int l1=vars[i  ].length();
            int l2=vars[i+1].length();
            int c1=vars[i  ][l1-1];
            int c2=vars[i+1][l2-1];

            if ((c1=='x' && c2=='y') &&
                vars[i].substr(0,l1-1) == vars[i+1].substr(0,l2-1))
            {
                string s(vars[i].substr(0,l1-1) +
                         " vector {" +
                         vars[i]     + "," +
                         vars[i+1]   + "};" );

                strcat(silo_defvar, s.c_str());
                i+=2;
                continue;
            }
        }
        i++;
    }
    if (strlen(silo_defvar) > 0)
    {
        // truncate the trailing semicolon
        silo_defvar[strlen(silo_defvar)-1] = '\0';
    }
}

void
write_defvars(DBfile *db)
{
    int l = strlen(silo_defvar);
    if (l>0)
    {
        l++; // trailing '\0'
        DBWrite(db, "_meshtv_defvars", silo_defvar, &l, 1, DB_CHAR);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
init(string fname)
{
    CPU_word_size = sizeof(float);
    IO_word_size  = 0;
    float version;
    
#ifdef PARALLEL
    if (ui_process)
#endif
    if (debug_level >= 1)
        cout << "Using \"" << fname << "\" for initialization." << endl;
    int   ex_oid = ex_open (fname.c_str(), EX_READ,
                            &CPU_word_size, &IO_word_size, &version);

    int   err;
    int   idummy;
    float fdummy;
    int   n_elem_blks;
    int   n_node_sets;
    int   n_side_sets;
    err = ex_get_init(ex_oid, problem_title, &ndims,
                      &idummy, &idummy,
                      &n_elem_blks, &n_node_sets, &n_side_sets);
    if (err<0)
        throw ExodusException("couldn't get initialization from exodus file");

#ifdef PARALLEL
    if (ui_process)
#endif
    if (debug_level >= 1)
        cout << "Problem title: " << problem_title << endl;

    ex_inquire(ex_oid, EX_INQ_TIME, &ntimes, &fdummy, NULL);

    timeless = (ntimes == 0);

    if (! timeless)
    {
        vec_resize(ftimes, ntimes);
        ex_get_all_times(ex_oid, &ftimes[0]);
    }
    else
    {
        // We normally treat this as a file with 1 cycle, but we
        // use the `timeless' flag when we need to know for sure.
        ntimes = 1;
        ftimes.push_back(0);
    }

    // create vectors to store multivars
    m_mesh.resize(ntimes);
    m_meshtype.resize(ntimes);
    m_mat.resize(ntimes);
    m_var.resize(ntimes);
    m_vartype.resize(ntimes);


    // get the variable lists
    ex_get_var_param(ex_oid, "n", &n_vars_n);
    ex_get_var_param(ex_oid, "e", &n_vars_z);

    // nodal
    if (n_vars_n)
    {
        var_names_n.reserve(n_vars_n);
        for (int v=0; v<n_vars_n; v++)
            var_names_n.push_back(new char[1024]);
        ex_get_var_names(ex_oid, "n", n_vars_n, &var_names_n[0]);
        for (int v=0; v<n_vars_n; v++)
        {
            make_silo_friendly(var_names_n[v]);
            if (use_sel_vars)
            {
                if (find(sel_vars.begin(), sel_vars.end(),
                         string(var_names_n[v])) != sel_vars.end())
                    sel_vars_n.push_back(true);
                else
                    sel_vars_n.push_back(false);
            }
        }
    }

    // zonal
    if (n_vars_z)
    {
        var_names_z.reserve(n_vars_z);
        for (int v=0; v<n_vars_z; v++)
            var_names_z.push_back(new char[1024]);
        ex_get_var_names(ex_oid, "e", n_vars_z, &var_names_z[0]);
        for (int v=0; v<n_vars_z; v++)
        {
            make_silo_friendly(var_names_z[v]);
            if (use_sel_vars)
            {
                if (find(sel_vars.begin(), sel_vars.end(),
                         string(var_names_z[v])) != sel_vars.end())
                    sel_vars_z.push_back(true);
                else
                    sel_vars_z.push_back(false);
            }
        }
    }

    vec_clear(var_names);
    for (int v=0; v<n_vars_n; v++)
    {
        if (!use_sel_vars || sel_vars_n[v])
            var_names.push_back(var_names_n[v]);
    }
    for (int v=0; v<n_vars_z; v++)
    {
        if (!use_sel_vars || sel_vars_z[v])
            var_names.push_back(var_names_z[v]);
    }

    calc_defvars(var_names);

#ifdef PARALLEL
    if (ui_process)
#endif
    if (debug_level >= 1)
    {
        cout << "num element blocks == " << n_elem_blks << endl;
        cout << "num node sets      == " << n_node_sets << endl;
        cout << "num side set       == " << n_side_sets << endl;
        cout << "nodal variables:" << endl;
        for (int v=0; v<n_vars_n; v++)
            if (use_sel_vars && !sel_vars_n[v])
                cout << "    (skipping " << var_names_n[v] << ")" << endl;
            else
                cout << "    " << var_names_n[v] << endl;
        cout << "zonal variables:" << endl;
        for (int v=0; v<n_vars_z; v++)
            if (use_sel_vars && !sel_vars_z[v])
                cout << "    (skipping " << var_names_z[v] << ")" << endl;
            else
                cout << "    " << var_names_z[v] << endl;
    }


    ex_close(ex_oid);

    element_num_nodes["NULL"]     = 0;
    //element_num_nodes["CIRCLE"]   = 1;
    //element_num_nodes["SPHERE"]   = 1;
    element_num_nodes["TRUSS"]    = 2;
    element_num_nodes["BEAM"]     = 2;
    element_num_nodes["SHELL"]    = ndims==2 ? 2 : 4;
    element_num_nodes["TRIANGLE"] = 3;
    element_num_nodes["TRIAN"]    = 3;
    element_num_nodes["QUAD"]     = 4;
    element_num_nodes["TETRA"]    = 4;
    element_num_nodes["TETRA10"]  = 4;
    element_num_nodes["WEDGE"]    = 6;
    element_num_nodes["HEX"]      = 8;

    element_node_map["NULL"]     = NULL;
    //element_node_map["CIRCLE"]   = node_map_circle;
    //element_node_map["SPHERE"]   = node_map_sphere;
    element_node_map["TRUSS"]    = node_map_truss;
    element_node_map["BEAM"]     = node_map_beam;
    element_node_map["SHELL"]    = ndims==2 ? node_map_shell2 : node_map_shell3;
    element_node_map["TRIANGLE"] = node_map_triangle;
    element_node_map["TRIAN"]    = node_map_triangle;
    element_node_map["QUAD"]     = node_map_quad;
    element_node_map["TETRA"]    = node_map_tetra;
    element_node_map["TETRA10"]  = node_map_tetra;
    element_node_map["WEDGE"]    = node_map_wedge;
    element_node_map["HEX"]      = node_map_hex;

    element_silo_type["NULL"]     = -1;
    //element_silo_type["CIRCLE"]   = DB_ZONETYPE_POINT ?;
    //element_silo_type["SPHERE"]   = DB_ZONETYPE_POINT ?;
    element_silo_type["TRUSS"]    = DB_ZONETYPE_BEAM;
    element_silo_type["BEAM"]     = DB_ZONETYPE_BEAM;
    element_silo_type["SHELL"]    = ndims==2 ? DB_ZONETYPE_BEAM : DB_ZONETYPE_QUAD;
    element_silo_type["TRIANGLE"] = DB_ZONETYPE_TRIANGLE;
    element_silo_type["TRIAN"]    = DB_ZONETYPE_TRIANGLE;
    element_silo_type["QUAD"]     = DB_ZONETYPE_QUAD;
    element_silo_type["TETRA"]    = DB_ZONETYPE_TET;
    element_silo_type["TETRA10"]  = DB_ZONETYPE_TET;
    element_silo_type["WEDGE"]    = DB_ZONETYPE_PRISM;
    element_silo_type["HEX"]      = DB_ZONETYPE_HEX;

    element_side_map_nodes["NULL"];
    //element_side_map_nodes["CIRCLE"];
    //element_side_map_nodes["SPHERE"];
    //element_side_map_nodes["TRUSS"];
    //element_side_map_nodes["BEAM"];
    if (ndims==3)
    {
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_1);
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_2);
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_3);
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_4);
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_5);
        element_side_map_nodes["SHELL"].push_back(side_map_shell3_6);
    }
    element_side_map_nodes["TRIANGLE"].push_back(side_map_triangle_1);
    element_side_map_nodes["TRIANGLE"].push_back(side_map_triangle_2);
    element_side_map_nodes["TRIANGLE"].push_back(side_map_triangle_3);
    element_side_map_nodes["TRIAN"] = element_side_map_nodes["TRIANGLE"];
    element_side_map_nodes["QUAD"].push_back(side_map_quad_1);
    element_side_map_nodes["QUAD"].push_back(side_map_quad_2);
    element_side_map_nodes["QUAD"].push_back(side_map_quad_3);
    element_side_map_nodes["QUAD"].push_back(side_map_quad_4);
    element_side_map_nodes["TETRA"].push_back(side_map_tetra_1);
    element_side_map_nodes["TETRA"].push_back(side_map_tetra_2);
    element_side_map_nodes["TETRA"].push_back(side_map_tetra_3);
    element_side_map_nodes["TETRA"].push_back(side_map_tetra_4);
    element_side_map_nodes["TETRA10"] = element_side_map_nodes["TETRA"];
    element_side_map_nodes["WEDGE"].push_back(side_map_wedge_1);
    element_side_map_nodes["WEDGE"].push_back(side_map_wedge_2);
    element_side_map_nodes["WEDGE"].push_back(side_map_wedge_3);
    element_side_map_nodes["WEDGE"].push_back(side_map_wedge_4);
    element_side_map_nodes["WEDGE"].push_back(side_map_wedge_5);
    element_side_map_nodes["HEX"].push_back(side_map_hex_1);
    element_side_map_nodes["HEX"].push_back(side_map_hex_2);
    element_side_map_nodes["HEX"].push_back(side_map_hex_3);
    element_side_map_nodes["HEX"].push_back(side_map_hex_4);
    element_side_map_nodes["HEX"].push_back(side_map_hex_5);
    element_side_map_nodes["HEX"].push_back(side_map_hex_6);

    element_side_map_counts["NULL"]     = NULL;
    //element_side_map_counts["CIRCLE"]   = NULL;
    //element_side_map_counts["SPHERE"]   = NULL;
    //element_side_map_counts["TRUSS"]    = NULL;
    //element_side_map_counts["BEAM"]     = NULL;
    if (ndims==3)
        element_side_map_counts["SHELL"]    = side_map_shell3_c;
    element_side_map_counts["TRIANGLE"] = side_map_triangle_c;
    element_side_map_counts["TRIAN"]    = side_map_triangle_c;
    element_side_map_counts["QUAD"]     = side_map_quad_c;
    element_side_map_counts["TETRA"]    = side_map_tetra_c;
    element_side_map_counts["TETRA10"]  = side_map_tetra_c;
    element_side_map_counts["WEDGE"]    = side_map_wedge_c;
    element_side_map_counts["HEX"]      = side_map_hex_c;

    element_side_map_types["NULL"]     = NULL;
    //element_side_map_types["CIRCLE"]   = NULL;
    //element_side_map_types["SPHERE"]   = NULL;
    //element_side_map_types["TRUSS"]    = NULL;
    //element_side_map_types["BEAM"]     = NULL;
    if (ndims==3)
        element_side_map_types["SHELL"]    = side_map_shell3_t;
    element_side_map_types["TRIANGLE"] = side_map_triangle_t;
    element_side_map_types["TRIAN"]    = side_map_triangle_t;
    element_side_map_types["QUAD"]     = side_map_quad_t;
    element_side_map_types["TETRA"]    = side_map_tetra_t;
    element_side_map_types["TETRA10"]  = side_map_tetra_t;
    element_side_map_types["WEDGE"]    = side_map_wedge_t;
    element_side_map_types["HEX"]      = side_map_hex_t;


#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
convert(const string &in_name, const string &out_name)
{
    if (debug_level >= 1)
        if (!use_given_directory && in_name.find('/') != NPOS)
        {
#ifdef PARALLEL
            cout << rank << ":";
#endif            
            cout << "\tconverting " << in_name << " ==> .../" 
                 << out_name.substr(out_name.rfind('/')+1) << endl;
        }
        else
        {
#ifdef PARALLEL
            cout << rank << ":";
#endif            
            cout << "\tconverting " << in_name << " ==> " << out_name << endl;
        }

    int ex_oid;

    // open the file
    CPU_word_size = sizeof(float);
    IO_word_size  = 0;
    float version;
    ex_oid = ex_open (in_name.c_str(), EX_READ,
                      &CPU_word_size, &IO_word_size, &version);

    // get general file info
    int   err;
    char  title[4096];
    int   n_nodes;
    int   n_elems;
    int   n_elem_blks;
    int   n_node_sets;
    int   n_side_sets;
    err = ex_get_init(ex_oid, title, &ndims,
                      &n_nodes, &n_elems,
                      &n_elem_blks, &n_node_sets, &n_side_sets);
    if (err<0)
        throw ExodusException("couldn't get initialization from exodus file");    

    if (debug_level >= 9)
    {
#ifdef PARALLEL
        cout << rank << ":";
#endif                    
        cout << "\tn_nodes="<<n_nodes<<"\t";
        cout << "\tn_elems="<<n_elems<<"\n";
    }

    // check to see if skipping node or side sets
    if (!do_nodesets)
        n_node_sets = 0;
    if (!do_sidesets)
        n_side_sets = 0;

    // get the coordinate arrays
    vector<float> X(n_nodes);
    vector<float> Y(n_nodes);
    vector<float> Z(n_nodes);

    err = ex_get_coord(ex_oid, &X[0], &Y[0], &Z[0]);
    if (err<0)
        throw ExodusException("couldn't read coordinates");

    vector<float> Xorig(X);
    vector<float> Yorig(Y);
    vector<float> Zorig(Z);

    // get the coordinate names
    char *coord_names[3] = {new char[4000],new char[4000], new char[4000]};
    err = ex_get_coord_names(ex_oid, coord_names);
    if (err<0)
        throw ExodusException("couldn't read coordinate names");

    vector<int> block_ids(n_elem_blks);
    err = ex_get_elem_blk_ids(ex_oid, &block_ids[0]);
    if (err<0)
        throw ExodusException("couldn't read block ids");

    // set up matnos array
    vector<int> matnos;
    for (int m=0; m<n_elem_blks; m++)
    {
        matnos.push_back(block_ids[m]);
    }

    // get zonal truth table
    vector<int> var_exists_z(n_vars_z * n_elem_blks);
    if (n_vars_z)
        ex_get_elem_var_tab(ex_oid, n_elem_blks, n_vars_z, &var_exists_z[0]);

    // get node set info
    vector<int> nodeset_ids(n_node_sets);
    vector<int> nodeset_n_nodes(n_node_sets);
    vector<int> nodeset_n_distfact(n_node_sets);
    if (n_node_sets)
    {
        err = ex_get_node_set_ids(ex_oid, &nodeset_ids[0]);
        if (err<0)
            throw ExodusException("couldn't read node set ids");
        for (int n=0; n<n_node_sets; n++)
        {
            err = ex_get_node_set_param(ex_oid, nodeset_ids[n],
                                        &nodeset_n_nodes[n],
                                        &nodeset_n_distfact[n]);
            if (err<0)
                throw ExodusException("couldn't read node set params");
        }
    }

    // get side set info
    vector<int> sideset_ids(n_side_sets);
    vector<int> sideset_n_sides(n_side_sets);
    vector<int> sideset_n_distfact(n_side_sets);
    vector<int> ss_nzones(n_side_sets, 0);
    vector<string> ss_zl_name(n_side_sets);
    vector<string> ss_fl_name(n_side_sets);
    vector<vector<int> > ss_material(n_side_sets);
    vector<vector<int> > ss_origzone(n_side_sets);
    vector<vector<int> > ss_nodelist(n_side_sets);

    if (n_side_sets)
    {
        err = ex_get_side_set_ids(ex_oid, &sideset_ids[0]);
        if (err<0)
            throw ExodusException("couldn't read side set ids");
        for (int s=0; s<n_side_sets; s++)
        {
            err = ex_get_side_set_param(ex_oid, sideset_ids[s],
                                        &sideset_n_sides[s],
                                        &sideset_n_distfact[s]);
            if (err<0)
                throw ExodusException("couldn't read side set params");
        }
    }

    // get the connectivity information and convert it
    vector<int> blk_n_elems(n_elem_blks);
    vector<int> blk_elem_size(n_elem_blks);
    vector<int> blk_n_attr(n_elem_blks);
    vector<string> blk_elem_type(n_elem_blks);
    vector<int> blk_first_elem(n_elem_blks);
    vector<int> blk_first_node(n_elem_blks);
    vector<vector<int> > blk_connectivity(n_elem_blks);

    // (silo-style vars)
    int silo_nzones   = 0;
    int silo_nshapes  = 0;
    vector<int> silo_shapecnt;
    vector<int> silo_shapesize;
    vector<int> silo_shapetype;
    vector<int> silo_nodelist;
    vector<int> silo_material;

    vector<int> new_silo_zone(n_elems, -1);  // map ex_elems to silo_zones
    
    for(int i = 0; i < n_elem_blks; i++) {
        if (i)
        {
            blk_first_elem[i] = blk_first_elem[i-1] + blk_n_elems[i-1];
            blk_first_node[i] = blk_first_node[i-1] + blk_n_elems[i-1]*blk_elem_size[i-1];
        }
        else
        {
            blk_first_elem[i] = 0;
            blk_first_node[i] = 0;
        }

        char elem_type[4000];
        err = ex_get_elem_block(ex_oid, block_ids[i],
                                elem_type,
                                &blk_n_elems[i],
                                &blk_elem_size[i],
                                &blk_n_attr[i]);
        if (err<0)
            throw ExodusException("couldn't get block attributes");

        if (!blk_n_elems[i])
            continue;

        transform(elem_type, elem_type+strlen(elem_type), elem_type, toupper);

        blk_elem_type[i] = elem_type;

        if (element_node_map.find(elem_type) == element_node_map.end())
        {
            cerr << "Hmm.  Unknown shape type " << elem_type << ".  Skipping it."
                 << endl;
            continue;
        }

        if (!element_node_map[elem_type])
            continue;

        vec_resize(blk_connectivity[i], blk_n_elems[i] * blk_elem_size[i]);
        err = ex_get_elem_conn(ex_oid, block_ids[i], &(blk_connectivity[i][0]));
        if (err<0)
            throw ExodusException("couldn't get connectivity array");

        // convert this block into another silo shapetype

        silo_nshapes++;
        silo_shapecnt.push_back(blk_n_elems[i]);
        silo_shapetype.push_back(element_silo_type[elem_type]);
        silo_shapesize.push_back(element_num_nodes[elem_type]);
        for (int j=0,k=0; j<blk_n_elems[i]; j++, k+=blk_elem_size[i])
        {
            new_silo_zone[blk_first_elem[i]+j] = silo_nzones;

            int n_verts   = element_num_nodes[elem_type];
            int *node_map = element_node_map[elem_type];
            for (int l=0; l<n_verts; l++)
            {
                int node = blk_connectivity[i][k + node_map[l]-1] - 1;
                silo_nodelist.push_back(node);
            }
            silo_material.push_back(block_ids[i]);
            silo_nzones++;
        }
    }

    // calculate the external facelist
    DBfacelist *fl;
    fl = DBCalcExternalFacelist2(&silo_nodelist[0], silo_nodelist.size(),
                                 0,0,  0,
                                 &silo_shapetype[0], &silo_shapesize[0],
                                 &silo_shapecnt[0], silo_shapetype.size(),
                                 &silo_material[0], 1);

    if (silo_nzones==0 || fl->nfaces==0)
    {
        cout << "Warning in file " << in_name
             << ": Empty output mesh, possible unsupported zone/face types.  "
             << "Skipping..." << endl;
        // free memory here!
        return;
    }

    // create  the file
    DBfile *db = DBCreate((char*)out_name.c_str(), DB_CLOBBER, DB_LOCAL,
                          problem_title, DB_PDB);

    // zone list
    if (DBPutZonelist(db, "zl", silo_nzones, ndims,
                      &silo_nodelist[0],
                      silo_nodelist.size(), 0/*origin*/,
                      &silo_shapesize[0],
                      &silo_shapecnt[0],
                      silo_shapetype.size()) < 0)
        throw SiloException("Zonelist messed");

    // face list
    if (DBPutFacelist(db, "fl", fl->nfaces, ndims,
                      fl->nodelist, fl->lnodelist, fl->origin, fl->zoneno,
                      fl->shapesize, fl->shapecnt, fl->nshapes, fl->types,
                      fl->typelist, fl->ntypes) < 0) 
        throw SiloException("Facelist messed");


    // construct side sets
    for (int ss=0; ss<n_side_sets; ss++)
    {
        char ss_name[1024];
        sprintf(ss_name,"sideset_%d",sideset_ids[ss]);

        if (sideset_n_sides[ss] == 0)
            continue;

        vector<int> sideset_sides(sideset_n_sides[ss]);
        vector<int> sideset_elems(sideset_n_sides[ss]);

        err = ex_get_side_set(ex_oid, sideset_ids[ss],
                              &sideset_elems[0], &sideset_sides[0]);
        if (err < 0)
            throw ExodusException("couldn't read side set");

        // (silo-style vars)
        int ss_nshapes  = 0;
        vector<int> ss_shapecnt;
        vector<int> ss_shapesize;
        vector<int> ss_shapetype;

        vector<int> ss_nodelist_beam;
        vector<int> ss_nodelist_tri;
        vector<int> ss_nodelist_quad;

        for (int s=0; s<sideset_n_sides[ss]; s++)
        {
            int b;
            int elem = sideset_elems[s]-1;
            int side = sideset_sides[s]-1;
            for (b=0; b<n_elem_blks; b++)
                if (blk_first_elem[b] + blk_n_elems[b] > elem)
                    break;

            string elem_type = blk_elem_type[b];
            if (element_side_map_nodes.find(elem_type) == element_side_map_nodes.end())
                continue;

            int e = (sideset_elems[s]-1) - blk_first_elem[b];

            //int firstnode = blk_first_node[b] + e*blk_elem_size[b];
            int firstnode  =  e * blk_elem_size[b];
            int *side_type = element_side_map_types[elem_type];
            int *node_map  = element_side_map_nodes[elem_type][side];

            for (int l=0; l<element_side_map_counts[elem_type][side]; l++)
            {
                int node = blk_connectivity[b][firstnode + node_map[l]-1]-1;
                switch (side_type[l]) 
                {
                  case DB_ZONETYPE_BEAM:
                    //ss_nodelist_beam.push_back(node);
                    break;
                  case DB_ZONETYPE_TRIANGLE:
                    ss_nodelist_tri.push_back(node);
                    break;
                  case DB_ZONETYPE_QUAD:
                    ss_nodelist_quad.push_back(node);
                    break;
                }
            }
            ss_origzone[ss].push_back(sideset_elems[s]-1);
            ss_material[ss].push_back(block_ids[b]+1);
            ss_nzones[ss]++;
        }

        if (ss_nodelist_beam.size())
        {
            ss_nshapes++;
            ss_shapecnt.push_back(ss_nodelist_beam.size()/2);
            ss_shapesize.push_back(2);
            ss_shapetype.push_back(DB_ZONETYPE_BEAM);
            ss_nodelist[ss].insert(ss_nodelist[ss].end(),
                                   ss_nodelist_beam.begin(),
                                   ss_nodelist_beam.end());
        }
        if (ss_nodelist_tri.size())
        {
            ss_nshapes++;
            ss_shapecnt.push_back(ss_nodelist_tri.size()/3);
            ss_shapesize.push_back(3);
            ss_shapetype.push_back(DB_ZONETYPE_TRIANGLE);
            ss_nodelist[ss].insert(ss_nodelist[ss].end(),
                                   ss_nodelist_tri.begin(),
                                   ss_nodelist_tri.end());
        }
        if (ss_nodelist_quad.size())
        {
            ss_nshapes++;
            ss_shapecnt.push_back(ss_nodelist_quad.size()/4);
            ss_shapesize.push_back(4);
            ss_shapetype.push_back(DB_ZONETYPE_QUAD);
            ss_nodelist[ss].insert(ss_nodelist[ss].end(),
                                   ss_nodelist_quad.begin(),
                                   ss_nodelist_quad.end());
        }
            
        // calculate the external facelist
        DBfacelist *ss_fl;
        ss_fl = DBCalcExternalFacelist2(&ss_nodelist[ss][0],
                                        ss_nodelist[ss].size(),
                                        0,0,  0,
                                        &ss_shapetype[0], &ss_shapesize[0],
                                        &ss_shapecnt[0], ss_shapetype.size(),
                                        &ss_material[ss][0], 1);

        ss_zl_name[ss] = string(ss_name) + "_zl";
        ss_fl_name[ss] = string(ss_name) + "_fl";

        // zone list
        if (DBPutZonelist(db, (char*)ss_zl_name[ss].c_str(),
                          ss_nzones[ss], ndims,
                          &ss_nodelist[ss][0],
                          ss_nodelist[ss].size(), 0/*origin*/,
                          &ss_shapesize[0],
                          &ss_shapecnt[0],
                          ss_shapetype.size()) < 0)
            throw SiloException("Zonelist messed");

        // face list
        if (DBPutFacelist(db, (char*)ss_fl_name[ss].c_str(),
                          ss_fl->nfaces, ndims,
                          ss_fl->nodelist, ss_fl->lnodelist,
                          ss_fl->origin, ss_fl->zoneno,
                          ss_fl->shapesize, ss_fl->shapecnt,
                          ss_fl->nshapes, ss_fl->types,
                          ss_fl->typelist, ss_fl->ntypes) < 0) 
            throw SiloException("Facelist messed");

        ss_zl_name[ss] = "/" + ss_zl_name[ss];
        ss_fl_name[ss] = "/" + ss_fl_name[ss];
    }

    //-------------------------------
    //        write the file
    //-------------------------------

    // time-dependent variables
    for (int i=0; i<ntimes; i++)
    {
        if ((use_cstart  &&  i < cstart) ||
            (use_cstop   &&  i > cstop) ||
            (use_tstart  &&  ftimes[i] < tstart) ||
            (use_tstop   &&  ftimes[i] > tstop))
            continue;

        DBoptlist *opt = NULL;
        string subdirname;

        if (!timeless)
        {
            if (debug_level >= 2)
                cout << "\t\tdoing time step " << i << " : "  << ftimes[i] << endl;

            char dirname[1000];
            sprintf(dirname, "cycle_%05d",i);
            DBMkDir(db, dirname);
            DBSetDir(db, dirname);
            subdirname = string(dirname) + "/";

            opt = DBMakeOptlist(10);
            DBAddOption(opt, DBOPT_TIME, &ftimes[i]);
            DBAddOption(opt, DBOPT_CYCLE, &i);

            // mesh
            vector<float> x_displ(n_nodes, 0.0);
            vector<float> y_displ(n_nodes, 0.0);
            vector<float> z_displ(n_nodes, 0.0);
            
            for(int v=0; v<n_vars_n; v++)
            {
                if (use_sel_vars && !sel_vars_n[v])
                    continue;

                err = 0;
                string name(var_names_n[v]);

                if (name=="displx" || name=="disp_x" || name=="disx")
                    err = ex_get_nodal_var(ex_oid, i+1, v+1, n_nodes, &x_displ[0]);
                if (name=="disply" || name=="disp_y" || name=="disy")
                    err = ex_get_nodal_var(ex_oid, i+1, v+1, n_nodes, &y_displ[0]);
                if (name=="displz" || name=="disp_z" || name=="disz")
                    err = ex_get_nodal_var(ex_oid, i+1, v+1, n_nodes, &z_displ[0]);
                if (err<0)
                    throw ExodusException("error reading displacement arrays");
            }

            for (int n=0; n<n_nodes; n++)
            {
                X[n] = Xorig[n] + (x_displ[n] * displ_scale);
                Y[n] = Yorig[n] + (y_displ[n] * displ_scale);
                Z[n] = Zorig[n] + (z_displ[n] * displ_scale);
            }
        }

        float *coords[3] = {&X[0],&Y[0],&Z[0]};
        if (!timeless)
            err = DBPutUcdmesh(db, "mesh", ndims, coord_names, coords,
                               n_nodes, silo_nzones, "../zl", "../fl",
                               DB_FLOAT, opt);
        else
            err = DBPutUcdmesh(db, "mesh", ndims, coord_names, coords,
                               n_nodes, silo_nzones, "zl", "fl",
                               DB_FLOAT, opt);
        if (err < 0)
            throw SiloException("UCD mesh messed");

        m_mesh[i]["mesh"].push_back(out_name + ":/" + subdirname + "mesh");
        m_meshtype[i]["mesh"].push_back(DB_UCDMESH);

        // material
        err = DBPutMaterial(db, "material", "mesh",
                            matnos.size(), &matnos[0], &silo_material[0],
                            &silo_nzones, 1, NULL,NULL,NULL,NULL,0,
                            DB_FLOAT, opt);
        if (err < 0)
            throw SiloException("Material messed");

        m_mat[i]["material"].push_back(out_name + ":/" + subdirname + "material");

        // nodal variables
        vector<float> nval(n_nodes);
        for (int v=0; v<n_vars_n; v++)
        {
            if (use_sel_vars && !sel_vars_n[v])
                continue;

            err = ex_get_nodal_var(ex_oid, i+1, v+1, n_nodes, &nval[0]);
            if (err < 0)
                throw ExodusException("could not read nodal variable");

            err = DBPutUcdvar1(db, var_names_n[v], "mesh", &nval[0], n_nodes,
                               NULL,0, DB_FLOAT, DB_NODECENT, opt);
            if (err < 0)
                throw SiloException("Nodal variable messed");

            m_var[i][var_names_n[v]].push_back(out_name + ":/" + subdirname + var_names_n[v]);
            m_vartype[i][var_names_n[v]].push_back(DB_UCDVAR);
        }

        // zonal variables
        vector<vector<float> > zval(n_vars_z);
        for (int v=0; v<n_vars_z; v++)
        {
            if (use_sel_vars && !sel_vars_z[v])
                continue;

            vec_resize(zval[v], silo_nzones);

            int z=0;
            for (int b=0; b<n_elem_blks; b++)
            {
                if (!blk_n_elems[b])
                    continue;

                if (var_exists_z[b*n_vars_z + v])
                {
                    err = ex_get_elem_var(ex_oid, i+1, v+1, block_ids[b],
                                          blk_n_elems[b], &zval[v][z]);
                    if (err < 0)
                        throw ExodusException("could not read zonal variable");
                }
                else
                {
                    for (int k=z; k<z+blk_n_elems[b]; k++)
                        zval[v][k] = 0.0;
                }

                z += blk_n_elems[b];
            }

            if (silo_nzones != z)
                throw LogicException("Ugh!  Internal error in converting zonal variables.");

            err = DBPutUcdvar1(db, var_names_z[v], "mesh", &zval[v][0],
                               silo_nzones, NULL,0, DB_FLOAT, DB_ZONECENT, opt);
            if (err < 0)
                throw SiloException("Zonal variable messed");

            m_var[i][var_names_z[v]].push_back(out_name + ":/" + subdirname + var_names_z[v]);
            m_vartype[i][var_names_z[v]].push_back(DB_UCDVAR);
        }

        if (n_node_sets)
        {
            DBMkDir(db, "nodesets");
            DBSetDir(db, "nodesets");

            m_mesh[i][string("nodesets/")+"mesh"];
            m_meshtype[i][string("nodesets/")+"mesh"];
            m_var[i][string("nodesets/")+"distfact"];
            m_vartype[i][string("nodesets/")+"distfact"];
            for (int v=0; v<n_vars_n; v++)
            {
                if (use_sel_vars && !sel_vars_n[v])
                    continue;
                m_var[i][string("nodesets/")+var_names_n[v]];
                m_vartype[i][string("nodesets/")+var_names_n[v]];
            }
        }

        // node sets
        for (int ns=0; ns<n_node_sets; ns++)
        {
            char ns_name[1024];
            sprintf(ns_name,"nodeset_%d",nodeset_ids[ns]);
            string ns_subdirname(string(ns_name) + "/");

            DBMkDir(db, (char*)ns_subdirname.c_str());
            DBSetDir(db, (char*)ns_subdirname.c_str());

            ns_subdirname = string("nodesets/") + ns_subdirname;

            if (nodeset_n_nodes[ns] == 0)
            {
                // these items in the map need to exist
                // note -- don't want to clear() them
                m_mesh[i][ns_subdirname+"mesh"];
                m_meshtype[i][ns_subdirname+"mesh"];
                m_var[i][ns_subdirname+"distfact"];
                m_vartype[i][ns_subdirname+"distfact"];
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    m_var[i][ns_subdirname+var_names_n[v]];
                    m_vartype[i][ns_subdirname+var_names_n[v]];
                }
                DBSetDir(db,"..");
                continue;
            }

            vector<int> nodeset_nodes(nodeset_n_nodes[ns]);
            err = ex_get_node_set(ex_oid, nodeset_ids[ns], &nodeset_nodes[0]);
            if (err<0)
                throw ExodusException("couldn't read node set");

            // nodeset pointmesh
            vector<float> ns_X(nodeset_n_nodes[ns]);
            vector<float> ns_Y(nodeset_n_nodes[ns]);
            vector<float> ns_Z(nodeset_n_nodes[ns]);
            for (int n=0; n<nodeset_n_nodes[ns]; n++)
            {
                if (nodeset_nodes[n] == 0)
                    throw LogicException("Internal error -- node set not 1 origin!");
                ns_X[n] = X[nodeset_nodes[n]-1];
                ns_Y[n] = Y[nodeset_nodes[n]-1];
                ns_Z[n] = Z[nodeset_nodes[n]-1];
            }

            float *ns_coords[3] = {&ns_X[0],&ns_Y[0],&ns_Z[0]};
            DBPutPointmesh(db, "mesh", ndims, ns_coords,
                           nodeset_n_nodes[ns], DB_FLOAT, opt);

            m_mesh[i][ns_subdirname+"mesh"].push_back(out_name + ":/" + subdirname + ns_subdirname + "mesh");
            m_meshtype[i][ns_subdirname+"mesh"].push_back(DB_POINTMESH);
            m_mesh[i][string("nodesets/")+"mesh"].push_back(out_name + ":/" + subdirname + ns_subdirname + "mesh");
            m_meshtype[i][string("nodesets/")+"mesh"].push_back(DB_POINTMESH);

            // nodeset distfact
            if (nodeset_n_distfact[ns])
            {
                vector<float> ns_dist_fact(nodeset_n_distfact[ns]);
                err = ex_get_node_set_dist_fact(ex_oid, nodeset_ids[ns],
                                                &ns_dist_fact[0]);
                if (err < 0)
                    throw ExodusException("could not read nodeset distfact");

                err = DBPutPointvar1(db, "distfact", "mesh",
                                     &ns_dist_fact[0], nodeset_n_nodes[ns],
                                     DB_FLOAT, opt);
                if (err < 0)
                    throw SiloException("Nodal distfact messed");
                
                m_var[i][ns_subdirname+"distfact"].push_back(out_name + ":/" + subdirname + ns_subdirname + "distfact");
                m_vartype[i][ns_subdirname+"distfact"].push_back(DB_POINTVAR);
                m_var[i][string("nodesets/")+"distfact"].push_back(out_name + ":/" + subdirname + ns_subdirname + "distfact");
                m_vartype[i][string("nodesets/")+"distfact"].push_back(DB_POINTVAR);
            }
            else
            {
                m_var[i][ns_subdirname+"distfact"];
                m_vartype[i][ns_subdirname+"distfact"];
                m_var[i][string("nodesets/")+"distfact"];
                m_vartype[i][string("nodesets/")+"distfact"];
            }

            // nodeset (nodal) point vars
            vector<float> ns_nval_orig(n_nodes);
            vector<float> ns_nval(nodeset_n_nodes[ns]);
            for (int v=0; v<n_vars_n; v++)
            {
                if (use_sel_vars && !sel_vars_n[v])
                    continue;

                // note -- try only reading this once per file!
                err = ex_get_nodal_var(ex_oid, i+1, v+1, n_nodes,
                                       &ns_nval_orig[0]);
                if (err < 0)
                    throw ExodusException("could not read nodal variable");

                for (int n=0; n<nodeset_n_nodes[ns]; n++)
                {
                    if (nodeset_nodes[n] == 0)
                        cout << "ERROR -- not 1 origin!" << endl;
                    ns_nval[n] = ns_nval_orig[nodeset_nodes[n]-1];
                }

                err = DBPutPointvar1(db, var_names_n[v], "mesh",
                                     &ns_nval[0], nodeset_n_nodes[ns],
                                     DB_FLOAT, opt);
                if (err < 0)
                    throw SiloException("Nodal variable messed");

                m_var[i][ns_subdirname+var_names_n[v]].push_back(out_name + ":/" + subdirname + ns_subdirname + var_names_n[v]);
                m_vartype[i][ns_subdirname+var_names_n[v]].push_back(DB_POINTVAR);
                m_var[i][string("nodesets/")+var_names_n[v]].push_back(out_name + ":/" + subdirname + ns_subdirname + var_names_n[v]);
                m_vartype[i][string("nodesets/")+var_names_n[v]].push_back(DB_POINTVAR);
            }

            DBSetDir(db, "..");
        }
        if (n_node_sets)
        {
            if (singlefile && m_mesh[i][string("nodesets/")+"mesh"].size()>0)
            {
                DBoptlist *opt = NULL;
                if (!timeless)
                {
                    opt = DBMakeOptlist(5);
                    DBAddOption(opt, DBOPT_TIME, &ftimes[i]);
                    DBAddOption(opt, DBOPT_CYCLE, &i);
                }
                PutMultimesh(db, "mesh", 
                             m_mesh[i][string("nodesets/")+"mesh"].size(),
                             m_mesh[i][string("nodesets/")+"mesh"],
                             m_meshtype[i][string("nodesets/")+"mesh"],
                             opt);
                if (m_var[i][string("nodesets/")+"distfact"].size() > 0)
                {
                    PutMultivar(db, "distfact", 
                                m_var[i][string("nodesets/")+"distfact"].size(),
                                m_var[i][string("nodesets/")+"distfact"],
                                m_vartype[i][string("nodesets/")+"distfact"],
                                opt);
                }
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    PutMultivar(db, var_names_n[v], 
                                m_var[i][string("nodesets/")+var_names_n[v]].size(),
                                m_var[i][string("nodesets/")+var_names_n[v]],
                                m_vartype[i][string("nodesets/")+var_names_n[v]],
                                opt);
                }
                if (opt) DBFreeOptlist(opt);

                vec_clear(m_mesh[i][string("nodesets/")+"mesh"]);
                vec_clear(m_meshtype[i][string("nodesets/")+"mesh"]);
                vec_clear(m_var[i][string("nodesets/")+"distfact"]);
                vec_clear(m_vartype[i][string("nodesets/")+"distfact"]);
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    vec_clear(m_var[i][string("nodesets/")+var_names_n[v]]);
                    vec_clear(m_vartype[i][string("nodesets/")+var_names_n[v]]);
                }
            }

            DBSetDir(db, "..");
        }

        // side sets
        if (n_side_sets)
        {
            DBMkDir(db, "sidesets");
            DBSetDir(db, "sidesets");
            m_mesh[i][string("sidesets/")+"mesh"];
            m_meshtype[i][string("sidesets/")+"mesh"];
            m_mat[i][string("sidesets/")+"material"];
            m_var[i][string("sidesets/")+"distfact"];
            m_vartype[i][string("sidesets/")+"distfact"];
            for (int v=0; v<n_vars_n; v++)
            {
                if (use_sel_vars && !sel_vars_n[v])
                    continue;
                m_var[i][string("sidesets/")+var_names_n[v]];
                m_vartype[i][string("sidesets/")+var_names_n[v]];
            }
            for (int v=0; v<n_vars_z; v++)
            {
                if (use_sel_vars && !sel_vars_z[v])
                    continue;
                m_var[i][string("sidesets/")+var_names_z[v]];
                m_vartype[i][string("sidesets/")+var_names_z[v]];
            }
        }
        for (int ss=0; ss<n_side_sets; ss++)
        {
            char ss_name[1024];
            sprintf(ss_name,"sideset_%d",sideset_ids[ss]);
            string ss_subdirname(string(ss_name) + "/");

            DBMkDir(db, (char*)ss_subdirname.c_str());
            DBSetDir(db, (char*)ss_subdirname.c_str());

            ss_subdirname = string("sidesets/") + ss_subdirname;

            if (sideset_n_sides[ss] == 0)
            {
                // these items in the map need to exist
                // note -- don't want to clear() them
                m_mesh[i][ss_subdirname+"mesh"];
                m_meshtype[i][ss_subdirname+"mesh"];
                m_mat[i][ss_subdirname+"material"];
                m_var[i][ss_subdirname+"distfact"];
                m_vartype[i][ss_subdirname+"distfact"];
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    m_var[i][ss_subdirname+var_names_n[v]];
                    m_vartype[i][ss_subdirname+var_names_n[v]];
                }
                for (int v=0; v<n_vars_z; v++)
                {
                    if (use_sel_vars && !sel_vars_z[v])
                        continue;
                    m_var[i][ss_subdirname+var_names_z[v]];
                    m_vartype[i][ss_subdirname+var_names_z[v]];
                }
                continue;
            }

            // write the mesh
            DBobject *obj = DBMakeObject ("mesh", DB_UCDMESH, 25);

            DBAddVarComponent (obj, "coord0", "../../mesh_coord0");
            DBAddVarComponent (obj, "coord1", "../../mesh_coord1");
            DBAddVarComponent (obj, "coord2", "../../mesh_coord2");
            DBAddVarComponent (obj, "min_extents", "../../mesh_min_extents");
            DBAddVarComponent (obj, "max_extents", "../../mesh_max_extents");
            DBAddStrComponent (obj, "zonelist", (char*)ss_zl_name[ss].c_str());
            DBAddStrComponent (obj, "facelist", (char*)ss_fl_name[ss].c_str());
            DBAddIntComponent (obj, "ndims", ndims);
            DBAddIntComponent (obj, "nnodes", n_nodes);
            DBAddIntComponent (obj, "nzones", ss_nzones[ss]);
            DBAddIntComponent (obj, "origin", 0);
            DBAddIntComponent (obj, "datatype", DB_FLOAT);
            if (!timeless)
            {
                DBAddVarComponent (obj, "time", "../../time");
                DBAddVarComponent (obj, "cycle", "../../cycle");
            }

            err = DBWriteObject(db, obj, 1);
            if (err < 0)
                throw SiloException("Couldn't write side set UCD mesh");

            DBFreeObject(obj);

            m_mesh[i][ss_subdirname+"mesh"].push_back(out_name + ":/" + subdirname + ss_subdirname + "mesh");
            m_meshtype[i][ss_subdirname+"mesh"].push_back(DB_UCDVAR);
            m_mesh[i][string("sidesets/")+"mesh"].push_back(out_name + ":/" + subdirname + ss_subdirname + "mesh");
            m_meshtype[i][string("sidesets/")+"mesh"].push_back(DB_UCDVAR);

            // material
            err = DBPutMaterial(db, "material", "mesh",
                                matnos.size(), &matnos[0], &ss_material[ss][0],
                                &ss_nzones[ss],1,  NULL,NULL,NULL,NULL,0,
                                DB_FLOAT, opt);
            if (err < 0)
                throw SiloException("Couldn't write side set material");

            m_mat[i][ss_subdirname+"material"].push_back(out_name + ":/" + subdirname + ss_subdirname + "material");
            m_mat[i][string("sidesets/")+"material"].push_back(out_name + ":/" + subdirname + ss_subdirname + "material");

            // dist fact
            if (sideset_n_distfact[ss])
            {
                static bool ssdf_warn = false;
                // This is overspecified.  These two numbers had better
                // match up, and they are definitely supposed to.
                // Larry Schoof said so. This can be removed if it
                // continues to work, but it's not a bad sanity check.
                if (!ssdf_warn)
                {
                    if (ss_nodelist[ss].size() != sideset_n_distfact[ss])
                    {
                        cout << "Warning: internal error -- the length of the "
                            "side set nodelist and the length of the distfact "
                            "array were different.  Please contact the author "
                            "of this program." << endl;
                        ssdf_warn = true;
                    }
                }

                vector<float> ss_dist_fact(sideset_n_distfact[ss]);
                err = ex_get_side_set_dist_fact(ex_oid, sideset_ids[ss],
                                                &ss_dist_fact[0]);

                vector<float> ss_dist_fact_nodal(n_nodes, 0.0);
                for (int n=0; n<sideset_n_distfact[ss]; n++)
                {
                    // This is overspecified.  These two numbers had better
                    // match up, and they are definitely supposed to.
                    // Larry Schoof said so. This can be removed if it
                    // continues to work, but it's not a bad sanity check.
                    if (!ssdf_warn)
                    {
                        if (ss_dist_fact_nodal[ss_nodelist[ss][n]] &&
                            ss_dist_fact_nodal[ss_nodelist[ss][n]] != ss_dist_fact[n])
                        {
                            cout << "Warning: internal error -- a node was "
                                "duplicated in a sideset distribution "
                                "factor but its value was not the same!  "
                                "Please contact the author of this program."
                                 << endl;
                            ssdf_warn = true;
                        }

                    }
                    ss_dist_fact_nodal[ss_nodelist[ss][n]] = ss_dist_fact[n];
                }
                err = DBPutUcdvar1(db, "distfact", "mesh",
                                   &ss_dist_fact_nodal[0], n_nodes,  NULL,0,
                                   DB_FLOAT, DB_NODECENT, opt);
                if (err < 0)
                    throw SiloException("Couldn't write side set dist fact");

                m_var[i][ss_subdirname+"distfact"].push_back(out_name + ":/" + subdirname + ss_subdirname + "distfact");
                m_vartype[i][ss_subdirname+"distfact"].push_back(DB_UCDVAR);
                m_var[i][string("sidesets/")+"distfact"].push_back(out_name + ":/" + subdirname + ss_subdirname + "distfact");
                m_vartype[i][string("sidesets/")+"distfact"].push_back(DB_UCDVAR);
            }
            else
            {
                m_var[i][ss_subdirname+"distfact"];
                m_vartype[i][ss_subdirname+"distfact"];
                m_var[i][string("sidesets/")+"distfact"];
                m_vartype[i][string("sidesets/")+"distfact"];
            }

            // nodal variables
            for (int v=0; v<n_vars_n; v++)
            {
                if (use_sel_vars && !sel_vars_n[v])
                    continue;

                obj = DBMakeObject (var_names_n[v], DB_UCDVAR, 15);

                string data_name = string("../../") + var_names_n[v] + "_data";
      
                DBAddVarComponent (obj, "value0", (char*)data_name.c_str());
                DBAddStrComponent (obj, "meshid", "mesh");
                DBAddIntComponent (obj, "ndims", ndims);
                DBAddIntComponent (obj, "nvals", 1);
                DBAddIntComponent (obj, "nels", n_nodes);
                DBAddIntComponent (obj, "centering", DB_NODECENT);
                DBAddIntComponent (obj, "origin", 0);
                DBAddIntComponent (obj, "mixlen", 0);
                DBAddIntComponent (obj, "datatype", DB_FLOAT);
                if (!timeless)
                {
                    DBAddVarComponent (obj, "time", "../../time");
                    DBAddVarComponent (obj, "cycle", "../../cycle");
                }

                err = DBWriteObject(db, obj, 1);
                if (err < 0)
                    throw SiloException("Couldn't write side set nodal var");
                DBFreeObject(obj);

                m_var[i][ss_subdirname+var_names_n[v]].push_back(out_name + ":/" + subdirname + ss_subdirname + var_names_n[v]);
                m_vartype[i][ss_subdirname+var_names_n[v]].push_back(DB_UCDVAR);
                m_var[i][string("sidesets/")+var_names_n[v]].push_back(out_name + ":/" + subdirname + ss_subdirname + var_names_n[v]);
                m_vartype[i][string("sidesets/")+var_names_n[v]].push_back(DB_UCDVAR);
            }

            // zonal variables
            for (int v=0; v<n_vars_z; v++)
            {
                if (use_sel_vars && !sel_vars_z[v])
                    continue;

                vector<float> ss_zval(ss_nzones[ss]);

                for (int z=0; z<ss_nzones[ss]; z++)
                {
                    int oz = new_silo_zone[ss_origzone[ss][z]];
                    if (oz >= 0)
                        ss_zval[z] = zval[v][oz];
                }

                err = DBPutUcdvar1(db, var_names_z[v], "mesh", &ss_zval[0],
                                   ss_nzones[ss],  NULL,0,
                                   DB_FLOAT, DB_ZONECENT, opt);
                if (err < 0)
                    throw SiloException("Couldn't write side set zonal var");

                m_var[i][ss_subdirname+var_names_z[v]].push_back(out_name + ":/" + subdirname + ss_subdirname + var_names_z[v]);
                m_vartype[i][ss_subdirname+var_names_z[v]].push_back(DB_UCDVAR);
                m_var[i][string("sidesets/")+var_names_z[v]].push_back(out_name + ":/" + subdirname + ss_subdirname + var_names_z[v]);
                m_vartype[i][string("sidesets/")+var_names_z[v]].push_back(DB_UCDVAR);
            }

            DBSetDir(db, "..");
        }
        if (n_side_sets)
        {
            if (singlefile && m_mesh[i][string("sidesets/")+"mesh"].size()>0)
            {
                DBoptlist *opt = NULL;
                if (!timeless)
                {
                    opt = DBMakeOptlist(5);
                    DBAddOption(opt, DBOPT_TIME, &ftimes[i]);
                    DBAddOption(opt, DBOPT_CYCLE, &i);
                }
                PutMultimesh(db, "mesh", 
                             m_mesh[i][string("sidesets/")+"mesh"].size(),
                             m_mesh[i][string("sidesets/")+"mesh"],
                             m_meshtype[i][string("sidesets/")+"mesh"],
                             opt);
                PutMultimat(db, "material", 
                            m_mat[i][string("sidesets/")+"material"].size(),
                            m_mat[i][string("sidesets/")+"material"],
                            opt);
                if (m_var[i][string("sidesets/")+"distfact"].size() > 0)
                {
                    PutMultivar(db, "distfact", 
                                m_var[i][string("sidesets/")+"distfact"].size(),
                                m_var[i][string("sidesets/")+"distfact"],
                                m_vartype[i][string("sidesets/")+"distfact"],
                                opt);
                }
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    PutMultivar(db, var_names_n[v], 
                                m_var[i][string("sidesets/")+var_names_n[v]].size(),
                                m_var[i][string("sidesets/")+var_names_n[v]],
                                m_vartype[i][string("sidesets/")+var_names_n[v]],
                                opt);
                }
                for (int v=0; v<n_vars_z; v++)
                {
                    if (use_sel_vars && !sel_vars_z[v])
                        continue;
                    PutMultivar(db, var_names_z[v], 
                                m_var[i][string("sidesets/")+var_names_z[v]].size(),
                                m_var[i][string("sidesets/")+var_names_z[v]],
                                m_vartype[i][string("sidesets/")+var_names_z[v]],
                                opt);
                }
                if (opt) DBFreeOptlist(opt);

                vec_clear(m_mesh[i][string("sidesets/")+"mesh"]);
                vec_clear(m_meshtype[i][string("sidesets/")+"mesh"]);
                vec_clear(m_mat[i][string("sidesets/")+"material"]);
                vec_clear(m_var[i][string("sidesets/")+"distfact"]);
                vec_clear(m_vartype[i][string("sidesets/")+"distfact"]);
                for (int v=0; v<n_vars_n; v++)
                {
                    if (use_sel_vars && !sel_vars_n[v])
                        continue;
                    vec_clear(m_var[i][string("sidesets/")+var_names_n[v]]);
                    vec_clear(m_vartype[i][string("sidesets/")+var_names_n[v]]);
                }
                for (int v=0; v<n_vars_z; v++)
                {
                    if (use_sel_vars && !sel_vars_z[v])
                        continue;
                    vec_clear(m_var[i][string("sidesets/")+var_names_z[v]]);
                    vec_clear(m_vartype[i][string("sidesets/")+var_names_z[v]]);
                }
            }
            DBSetDir(db, "..");
        }



        if (singlefile)
            write_defvars(db);

        if (!timeless)
        {
            DBFreeOptlist(opt);
            DBSetDir(db, "..");
        }
    }

    if (singlefile)
        write_defvars(db);

    ex_close(ex_oid);
    DBClose(db);
    
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef PARALLEL
void
par_merge_vector(const string &name, vector<int> &v)
{
    if (ui_process)
    {
        if (debug_level >= 6)
            cout << "intvec parallel-merging " << name << endl;
        for (int r=1; r<min(n_proc, n_files); r++)
        {
            MPI_Status status;
            int l;
            MPI_Recv(&l, 1, MPI_INT, r, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int oldsize = v.size();
            vec_resize(v, oldsize + l);
            MPI_Recv(&v[oldsize], l, MPI_INT, r, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
    }
    else
    {
        int l = v.size();
        MPI_Ssend(&l, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&v[0], l, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

// this is two possible implementations of the string merge.
// the first combines the vector<string> into a char[NxM] so that
// SGIs don't overflow their maximum send message counts
// the second tries to avoid this problem with sync sends and barriers
#if 0
// (this one's not done yet)
void
par_merge_vector(const string &name, vector<string> &v)
{
}

#else
void
par_merge_vector(const string &name, vector<string> &v)
{
    if (ui_process)
    {
        if (debug_level >= 6)
            cout << "parallel-merging " << name << endl;
        for (int r=1; r<min(n_proc, n_files); r++)
        {
            MPI_Status status;
            int vl;
            MPI_Recv(&vl, 1, MPI_INT, r, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            for (int i=0; i<vl; i++)
            {
                int sl;
                MPI_Recv(&sl, 1, MPI_INT, r, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                char buf[1024];
                MPI_Recv(buf, sl, MPI_INT, r, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                buf[sl] = '\0';
                v.push_back(buf);
            }
        }
    }
    else
    {
        int vl = v.size();
        MPI_Ssend(&vl, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        for (int i=0; i<vl; i++)
        {
            int sl = v[i].length();
            MPI_Send(&sl, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send((char*)v[i].c_str(), sl, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
}
#endif
#endif //PARALLEL



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
write_root(const string out_rootname)
{
#ifdef PARALLEL
    if (debug_level >= 8)
        cout << "process " << rank << " at barrier" << endl;
    MPI_Barrier(MPI_COMM_WORLD);

    if (ui_process)
#endif
    if (debug_level >= 1)
        cout << "Writing root file \"" << out_rootname << "\"" << endl;

    map<string,vector<string>,less<string> >::iterator is;
    map<string,vector<int>,less<string> >::iterator    ii;

#ifdef PARALLEL
    for (int i=0; i<ntimes; i++)
    {
        for (is=m_mesh[i].begin();     is != m_mesh[i].end();     is++)
            par_merge_vector((*is).first, (*is).second);
        for (ii=m_meshtype[i].begin(); ii != m_meshtype[i].end(); ii++)
            par_merge_vector((*ii).first, (*ii).second);
        for (is=m_mat[i].begin();      is != m_mat[i].end();      is++)
            par_merge_vector((*is).first, (*is).second);
        for (is=m_var[i].begin();      is != m_var[i].end();      is++)
            par_merge_vector((*is).first, (*is).second);
        for (ii=m_vartype[i].begin();  ii != m_vartype[i].end();  ii++)
            par_merge_vector((*ii).first, (*ii).second);
    }

    if (!ui_process)
        return;
#endif

    bool multi_root = (out_rootname.find('%') != NPOS);

    DBfile *db;
    // create the file
    if (!multi_root)
        db = DBCreate((char*)out_rootname.c_str(), DB_CLOBBER, DB_LOCAL,
                      "exodus root file", DB_PDB);

    for (int i=0; i<ntimes; i++)
    {
        if ((use_cstart  &&  i < cstart) ||
            (use_cstop   &&  i > cstop) ||
            (use_tstart  &&  ftimes[i] < tstart) ||
            (use_tstop   &&  ftimes[i] > tstop))
            continue;

        if (!timeless)
            if (debug_level >= 1)
                cout << "\t\tdoing time step " << i << " : "  << ftimes[i] << endl;

        if (multi_root)
        {
            char fname[1000];
            sprintf(fname, out_rootname.c_str(), i);
            db = DBCreate(fname, DB_CLOBBER, DB_LOCAL,
                          "exodus root file", DB_PDB);
        }
        else if (!timeless)
        {
            char dirname[1000];
            sprintf(dirname, "cycle_%05d", i);
            DBMkDir(db, dirname);
            DBSetDir(db, dirname);
        }

        DBoptlist *opt = NULL;
        if (!timeless)
        {
            opt = DBMakeOptlist(5);
            DBAddOption(opt, DBOPT_TIME, &ftimes[i]);
            DBAddOption(opt, DBOPT_CYCLE, &i);
        }

        // multimeshes
        for (is=m_mesh[i].begin(), ii=m_meshtype[i].begin();
             is!=m_mesh[i].end() && ii != m_meshtype[i].end();
             is++, ii++)
        {
            PutMultimesh(db, (*is).first, (*is).second.size(),
                         (*is).second, (*ii).second, opt);
        }

        // multimats
        for (is=m_mat[i].begin(); is!=m_mat[i].end(); is++)
        {
            PutMultimat(db, (*is).first, (*is).second.size(),
                        (*is).second, opt);
        }

        // multivars
        for (is=m_var[i].begin(), ii=m_vartype[i].begin();
             is!=m_var[i].end() && ii != m_vartype[i].end();
             is++, ii++)
        {
            PutMultivar(db, (*is).first, (*is).second.size(),
                        (*is).second, (*ii).second, opt);
        }

        write_defvars(db);

        if (opt)
            DBFreeOptlist(opt);

        if (multi_root)
            DBClose(db);
        else if  (!timeless)
            DBSetDir(db, "..");
    }

    if (!multi_root)
    {
        write_defvars(db);
        DBClose(db);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void
print_usage(const string &progname)
{
#ifdef PARALLEL
    if (!ui_process)
        return;
#endif

    cout << "Usage: " << progname << "[options] ex_file" << endl;
    cout << "   or: " << progname << "[options] ex_file.1 ex_file.2 ..." << endl;
    cout << endl;
    cout << "options:" << endl;
    cout << "  -cstart <time>      :  Do not convert cycles before <cycle>." << endl;
    cout << "                         There is no default." << endl;
    cout << "  -cstop <time>       :  Do not convert cycles after <cycle>." << endl;
    cout << "                         There is no default." << endl;
    cout << "  -disable-nodesets   :  Do not convert node sets." << endl;
    cout << "  -disable-sidesets   :  Do not convert side sets." << endl;
    cout << "  -debug <level>      :  Set debug info level to <level>, range 0-10." << endl;
    cout << "                         Default is 1." << endl;
    cout << "  -filelist <filename>:  Use <filename> as the list of files to process." << endl;
    cout << "  -info               :  Only output file info.  Don't convert." << endl;
    cout << "  -out <directory>    :  Output individual file parts into given directoy ." << endl;
    cout << "                         Default is the same directory as each exodus file." << endl;
    cout << "  -quiet              :  Set debug info level to 0 (no diagnostic info)." << endl;
    cout << "  -root <filename>    :  Use <filename> as the root file." << endl;
    cout << "                         Default is derived from base of individual filenames." << endl;
    cout << "                         If this contains a printf-style format string" << endl;
    cout << "                         containing something like %04d, it will split the " << endl;
    cout << "                         main root file into one root file per cycle." << endl;
    cout << "  -scale <factor>     :  Use <factor> as the scale for the mesh displacement." << endl;
    cout << "                         The default value is `1'." << endl;
    cout << "  -tstart <time>      :  Do not convert times before <time>." << endl;
    cout << "                         There is no default." << endl;
    cout << "  -tstop <time>       :  Do not convert times after <time>." << endl;
    cout << "                         There is no default." << endl;
    cout << "  -varlist <filename> :  Add the contents of <filename> to list of variables" << endl;;
    cout << "                         to process.  The default is to convert all variables." << endl;
    cout << endl;
}

int
main(int argc_, char *argv_[])
{
#ifdef PARALLEL
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ui_process = (rank==0);
#endif

    string progname(argv_[0]);
    vector<string> args;
    vec_insert_at_end(args, &argv_[1], &argv_[argc_]);
    
    while (!args.empty() && args[0][0]=='-')
    {
        if (args[0][1]=='-')
            args[0]=args[0].substr(1);

        if (args[0]=="-root")
        {
            use_given_rootname = true;
            out_rootname = args[1];
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "using given rootname == " << out_rootname << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-out")
        {
            use_given_directory = true;
            out_directory = args[1] + '/';
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "using given output dir == " << out_directory << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-scale")
        {
            displ_scale = atof(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "using scale == " << displ_scale << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-disable-nodesets")
        {
            do_nodesets = false;
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping node sets" << endl;
            args.erase(args.begin());
        }
        else if (args[0]=="-disable-sidesets")
        {
            do_sidesets = false;
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping side sets" << endl;
            args.erase(args.begin());
        }
        else if (args[0]=="-filelist")
        {
            string fname(args[1]);
            args.erase(args.begin());
            args.erase(args.begin());

            ifstream in(fname.c_str(),ios::in);
            if (!in)
            {
#ifdef PARALLEL
                if (ui_process)
#endif
                cout << "Error: File "<<fname<<" does not exist or is not readable." << endl;
#ifdef PARALLEL
                MPI_Finalize();
#endif
                exit(-1);
            }
            
            char tmp[1024];
            while (in >> tmp)
            {
                args.push_back(tmp);
            }

            in.close();
        }
        else if (args[0]=="-varlist")
        {
            use_sel_vars = true;
            string fname(args[1]);
            args.erase(args.begin());
            args.erase(args.begin());

            ifstream in(fname.c_str(),ios::in);
            if (!in)
            {
#ifdef PARALLEL
                if (ui_process)
#endif
                cout << "Error: File "<<fname<<" does not exist or is not readable." << endl;
#ifdef PARALLEL
                MPI_Finalize();
#endif
                exit(-1);
            }
            
            char tmp[1024];
            while (in >> tmp)
            {
                make_silo_friendly(tmp);
                sel_vars.push_back(tmp);
            }

            in.close();
        }
        else if (args[0]=="-quiet")
        {
            debug_level = 0;
            args.erase(args.begin());
        }
        else if (args[0]=="-debug")
        {
            debug_level = atoi(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "output debug level == " << debug_level << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-cstart")
        {
            use_cstart = true;
            cstart = atoi(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping cycles before  " << cstart << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-cstop")
        {
            use_cstop = true;
            cstop = atoi(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping cycles after  " << cstop << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-tstart")
        {
            use_tstart = true;
            tstart = atof(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping times before  " << tstart << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-tstop")
        {
            use_tstop = true;
            tstop = atof(args[1].c_str());
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "skipping times after  " << tstop << endl;
            args.erase(args.begin());
            args.erase(args.begin());
        }
        else if (args[0]=="-info")
        {
            info_only = true;
            debug_level = 10;
#ifdef PARALLEL
            if (ui_process)
#endif
            cout << "output data set info only" << endl;
            args.erase(args.begin());
        }
        else
        {
            print_usage(progname);
#ifdef PARALLEL
            MPI_Finalize();
#endif
            exit(-1);
        }
    }

    if (args.empty())
    {
        print_usage(progname);
#ifdef PARALLEL
        MPI_Finalize();
#endif
        exit(-1);
    }



    // Convert every file
    try {
        singlefile = (args.size() == 1);

        if (singlefile)
        {
            // initialize 
            init(args[0]);

            if (info_only)
            {
#ifdef PARALLEL
                MPI_Finalize();
#endif
                exit(0);
            }

            string ex_name(args[0]);
            string silo_name;
            if (!use_given_directory)
                silo_name = ex_name + ".silo";
            else
            {
                if (ex_name.rfind('/') != NPOS)
                    silo_name = out_directory +
                                ex_name.substr(ex_name.rfind('/')+1) +
                                ".silo";
                else
                    silo_name = out_directory +
                                ex_name +
                                ".silo";
            }


            // convert
#ifdef PARALLEL
            if (ui_process && n_proc>1)
                cout << "Warning: single file; no parallelism" << endl;
            if (ui_process)
#endif
            convert(ex_name, silo_name);
        }
        else
        {
            // get the input subfilenames
            n_files = args.size();
            vector<string> in_filenames;
            vector<string> out_filenames;
            for (int i=0; i<args.size(); i++)
            {
                in_filenames.push_back(args[i]);

                string silo_name;
                if (!use_given_directory)
                    silo_name = args[i] + ".silo";
                else
                {
                    if (args[i].rfind('/') != NPOS)
                        silo_name = out_directory +
                                    args[i].substr(args[i].rfind('/')+1) +
                                    ".silo";
                    else
                        silo_name = out_directory +
                                    args[i] +
                                    ".silo";
                }
                out_filenames.push_back(silo_name);
            }


            // remove paths from input filenames
            for (int i=0; i<args.size(); i++)
            {
                if (args[i].find('/') != NPOS)
                    args[i] = args[i].substr(args[i].rfind('/')+1);
            }

            // find the matching prefix
            string file_base;
            int   c = 0;
            bool  done = false;
            while (!done)
            {
                char m = args[0][c];
                if (m == '\0' || m == '.')
                    done = true;

                for (int i=1; i<args.size() && !done; i++)
                {
                    if (m != args[i][c])
                        done = true;
                }
                
                if (done)
                    break;

                file_base += m;
                c++;
            }
            
            // if the names are not similar, error out
            if (file_base.length()==0)
            {
#ifdef PARALLEL
                if (ui_process)
#endif
                cerr << "This doesn't appear to be a family of files...." << endl;
#ifdef PARALLEL
                MPI_Finalize();
#endif
                exit(-1);
            }

            // get the new root filename if needed
            if (!use_given_rootname)
                out_rootname = file_base + ".root";

#ifdef PARALLEL
            if (ui_process)
#endif
            if (debug_level >= 1)
                cout << "Using \"" << out_rootname << "\" for a root file" << endl;


            // initialize
            init(in_filenames[0]);

            if (info_only)
            {
#ifdef PARALLEL
                MPI_Finalize();
#endif
                exit(0);
            }

            // convert the input subfiles to the output subfiles
            for (int i=0; i<n_files; i++)
            {
                try
                {
#ifdef PARALLEL
                    if ((i % n_proc) == rank)
#endif
                    convert(in_filenames[i], out_filenames[i]);
                }
                catch (ExodusException &e)
                {
                    cerr << "Exodus error in converting file "<<in_filenames[i]<<": " << e.what() << endl;
                    cerr << "recovery -- ignoring file" << endl;
                }
                catch (SiloException &e)
                {
                    cerr << "Silo error in converting file "<<in_filenames[i]<<": " << e.what() << endl;
                    cerr << "recovery -- ignoring file" << endl;
                }
            }

            write_root(out_rootname);
        }
    }
    catch (ExodusException &e)
    {
        cerr << "Exodus error: " << e.what() << endl;
        exit(-1);
    }
    catch (SiloException &e)
    {
        cerr << "Silo error: " << e.what() << endl;
        exit(-1);
    }
    catch (LogicException &e)
    {
        cerr << "logic error: " << e.what() << endl;
        exit(-1);
    }

#ifdef PARALLEL
    MPI_Finalize();
#endif

}


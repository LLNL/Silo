extern int db_perror(char const *s, int errorno, char const *fname);

int DBOverwriteMultiObject(DBfile *dbfile, char const *objname, int nblocks,
    char const * const *block_names, char const *block_names_comp_name,
    int const *block_types, char const *block_types_comp_name, DBoptlist const *ol)
{
    static char const *me = "DBOverwriteMultiObject";
    char bnames_ds_name[256], btypes_ds_name[256];
    int i;
    int bnames_ds_len=-1;
    char msg[256];
    int oldow = DBSetAllowOverwrites(1);
    DBobject *raw_obj;

    msg[0] = '\0';
    raw_obj = DBGetObject(dbfile, objname);
    if (!raw_obj)
    {
        snprintf(msg, sizeof(msg),
            "unable to get raw object \"%s\" in preparation for overwrite", objname);
        goto done;
    }

    for (i = 0; i < raw_obj->ncomponents; i++)
    {
        if (!strcmp(raw_obj->comp_names[i], block_names_comp_name))
        {
            char *bnames_list=0;
            int bnames_list_len;
            int ndims=2, dims[2]={-1,-1};
            DBStringArrayToStringList(block_names, nblocks, &bnames_list, &bnames_list_len);
            strncpy(bnames_ds_name, &(raw_obj->pdb_names[i])[4], strlen(raw_obj->pdb_names[i])-5);
            DBGetVarDims(dbfile, bnames_ds_name, ndims, dims);
            if (dims[0] > bnames_list_len || dims[1] != -1)
            {
                snprintf(msg, sizeof(msg), "total names list len=%d > \"%s.%s\"=%d",
                    bnames_list_len, objname, block_names_comp_name, dims[0]);
                free(bnames_list);
                goto done;
            }
            DBWrite(dbfile, bnames_ds_name, bnames_list, &bnames_list_len, 1, DB_CHAR);
            free(bnames_list);
        }
        else if (block_types_comp_name && !strcmp(raw_obj->comp_names[i], block_types_comp_name))
        {
            int ndims=2, dims[2]={-1,-1};
            strncpy(btypes_ds_name, &(raw_obj->pdb_names[i])[4], strlen(raw_obj->pdb_names[i])-5);
            DBGetVarDims(dbfile, btypes_ds_name, ndims, dims);
            if (dims[0] != nblocks || dims[1] != -1)
            {
                snprintf(msg, sizeof(msg), "nblocks=%d != \"%s.%s.dims[0]\"=%d",
                    nblocks, objname, block_types_comp_name, dims[0]);
                goto done;
            }
            DBWrite(dbfile, btypes_ds_name, block_types, &nblocks, 1, DB_INT);
        }
    }

done:
    DBSetAllowOverwrites(oldow);
    if (raw_obj)
        DBFreeObject(raw_obj);
    if (msg[0])
        return db_perror(msg, E_NOOVERWRITE, me);
    return 1;
}

int DBPutMultimeshWithOverwrite(DBfile *dbfile, char const *oname,
    int nblocks, char const * const *block_names,
    int const *block_types, DBoptlist const *ol)
{
    if (!DBInqVarExists(dbfile, oname))
        return DBPutMultimesh(dbfile, oname, nblocks, block_names, block_types, ol);
    return DBOverwriteMultiObject(dbfile, oname, nblocks,
               block_names, "meshnames", block_types, "meshtypes", ol);
}

int DBPutMultivarWithOverwrite(DBfile *dbfile, char const *oname,
    int nblocks, char const * const *block_names,
    int const *block_types, DBoptlist const *ol)
{
    if (!DBInqVarExists(dbfile, oname))
        return DBPutMultivar(dbfile, oname, nblocks, block_names, block_types, ol);
    return DBOverwriteMultiObject(dbfile, oname, nblocks,
               block_names, "varnames", block_types, "vartypes", ol);
}

int DBPutMultimatWithOverwrite(DBfile *dbfile, char const *oname,
    int nblocks, char const * const *block_names, DBoptlist const *ol)
{
    if (!DBInqVarExists(dbfile, oname))
        return DBPutMultimat(dbfile, oname, nblocks, block_names, ol);
    return DBOverwriteMultiObject(dbfile, oname, nblocks,
               block_names, "matnames", 0, 0, ol);
}

int DBPutMultimatspeciesWithOverwrite(DBfile *dbfile, char const *oname,
    int nblocks, char const * const *block_names, DBoptlist const *ol)
{
    if (!DBInqVarExists(dbfile, oname))
        return DBPutMultimatspecies(dbfile, oname, nblocks, block_names, ol);
    return DBOverwriteMultiObject(dbfile, oname, nblocks,
               block_names, "specnames", 0, 0, ol);
}


#include <pdb.h>

int main()
{
    int foo[10] = {0,1,2,3,4,5,6,7,8,9};
    PDBfile *f = lite_PD_open("not_a_silo_file.pdb", "w");
    lite_PD_mkdir(f, "dir1");
    lite_PD_mkdir(f, "dir2");
    lite_PD_cd(f, "dir2");
    lite_PD_write(f, "foo(10)", "integer", foo);
    lite_PD_close(f);
    return 0;
}

#include <pdb.h>

int main()
{
    PDBfile *f = lite_PD_open("not_a_silo_file.pdb", "w");
    lite_PD_close(f);
    return 0;
}

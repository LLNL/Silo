#include <hdf5.h>

int main()
{
    hid_t f = H5Fcreate("not_a_silo_file.h5",H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
    hid_t g1 = H5Gcreate(f, "group1", 0);
    hid_t g2 = H5Gcreate(g1, "group2", 0);
    H5Fclose(f);
    return 0;
}

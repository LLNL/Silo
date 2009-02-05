/*

                           Copyright 1991 - 1995
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#include <stdio.h>

main()
{
#define LEN 50

    int            iarr[LEN], iarr2[LEN], i;
    float          farr[LEN], farr2[LEN];

    for (i = 0; i < LEN; i++) {
        iarr[i] = i;
        farr[i] = (float)i;
    }

    for (i = 0; i < 20; i++) {
        iarr2[i] = 12;
        farr2[i] = 12.34567;
    }
    for (i = 20; i < 50; i++) {
        iarr2[i] = 8734198;
        farr2[i] = 67890.123;
    }
    iarr2[47] = 33333;
    farr2[47] = 333.33;

    iarr[20] = 998877;
    farr[20] = 0.0012345;

    /*
     * Print integer array
     */
    printf("\n\n---------sw_prtarray---------------------\n\n");
    SW_prtarray(iarr, LEN, 16);
    printf("\n\n---------SWPrintData---------------------\n\n");
    SWPrintData(stdout, "somearray", iarr, 20, 16, 0, 1, 70);

    printf("\n\n---------sw_prtarray---------------------\n\n");
    SW_prtarray(iarr2, LEN, 16);
    printf("\n\n---------SWPrintData---------------------\n\n");
    SWPrintData(stdout, "somearray", iarr2, 50, 16, 0, 1, 70);

    /*
     * Print float array
     */
    printf("\n\n---------sw_prtarray---------------------\n\n");
    SW_prtarray(farr, LEN, 19);
    printf("\n\n---------SWPrintData---------------------\n\n");
    SWPrintData(stdout, "somearray", farr, 20, 19, 0, 1, 70);

    printf("\n\n---------sw_prtarray---------------------\n\n");
    SW_prtarray(farr2, LEN, 19);
    printf("\n\n---------SWPrintData---------------------\n\n");
    SWPrintData(stdout, "somearray", farr2, 50, 19, 0, 1, 70);

}

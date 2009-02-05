/*

                           Copyright 1991 - 1999
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

#include "swat.h"
#include "string.h"

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                  sw_data.c

  Purpose

       This module contains miscellaneous datatype functions used
       either by the SILO system or by anyone.

  Programmer

       Jeffery Long, NSSD/B

  Contents

       (char *)    SW_GetDatatypeString (itype)
       (int)       SW_GetDatatypeID (char *datatype)
       (int)       SW_GetMachineDataSize (int datatype)

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*----------------------------------------------------------------------
 *  Routine                                         SW_GetDatatypeString
 *
 *  Function
 *
 *      Return the string representation of the given SWAT data type.
 *---------------------------------------------------------------------*/
char *
SW_GetDatatypeString(int type)
{
    char          *str;

    str = ALLOC_N(char, 8);

    switch (type) {
        case SWAT_INT:
            strcpy(str, "integer");
            break;
        case SWAT_SHORT:
            strcpy(str, "short");
            break;
        case SWAT_LONG:
            strcpy(str, "long");
            break;
        case SWAT_FLOAT:
            strcpy(str, "float");
            break;
        case SWAT_DOUBLE:
            strcpy(str, "double");
            break;
        case SWAT_CHAR:
            strcpy(str, "char");
            break;
        default:
            sprintf(str, "%4d", type);
            break;
    }

    return (str);
}

/*----------------------------------------------------------------------
 *  Routine                                         SW_GetDatatypeID
 *
 *  Purpose
 *
 *      Return the SILO integer definition for the provided ascii datatype
 *      description. That is, convert "float" to SWAT_FLOAT (i.e., 19).
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
int
SW_GetDatatypeID(char *dataname)
{
    int            size;

    if (STR_BEGINSWITH(dataname, "integer"))
        size = SWAT_INT;
    else if (STR_BEGINSWITH(dataname, "short"))
        size = SWAT_SHORT;
    else if (STR_BEGINSWITH(dataname, "long"))
        size = SWAT_LONG;
    else if (STR_BEGINSWITH(dataname, "float"))
        size = SWAT_FLOAT;
    else if (STR_BEGINSWITH(dataname, "double"))
        size = SWAT_DOUBLE;
    else if (STR_BEGINSWITH(dataname, "char"))
        size = SWAT_CHAR;
    else
        size = 0;

    return (size);
}

/*----------------------------------------------------------------------
 *  Routine                                       SW_GetMachineDataSize
 *
 *  Purpose
 *
 *      Return the byte length of the given data type ON THE CURRENT
 *      MACHINE.
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
int
SW_GetMachineDataSize(int datatype)
{
    int            size;

    switch (datatype) {
        case SWAT_INT:
            size = sizeof(int);

            break;
        case SWAT_SHORT:
            size = sizeof(short);

            break;
        case SWAT_LONG:
            size = sizeof(long);

            break;
        case SWAT_FLOAT:
            size = sizeof(float);

            break;
        case SWAT_DOUBLE:
            size = sizeof(double);

            break;
        case SWAT_CHAR:
            size = sizeof(char);

            break;
        default:
            size = 0;
            break;
    }

    return (size);
}

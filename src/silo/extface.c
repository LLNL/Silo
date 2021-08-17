/*
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/

#include "silo_private.h"

/*
 * Maximum size of the hash table.  This should be a prime number.  This
 * was pulled off the web from
 *    http://www.utm.edu/research/primes/notes/10000.txti 
 */
#define HASH_MAX 100003

#define MALLOC_N(T,N)            ((T*)malloc((size_t)((N)*sizeof(T))))

typedef struct Face
{
    int       nNodes;           /* The number of nodes in the face. */
    int       *nodes;           /* The nodes making up the face. */
    int       zoneNo;           /* The zone number associated with the face. */
    struct Face *next;          /* Pointer to another face. */
} Face;

typedef struct FaceHash
{
    Face      **table;          /* The hash table. */
    int       size;             /* The size of the hash table. */
} FaceHash;

typedef struct CalcExternalFacesState
{
    int       *zoneList;
    int       nNodes;
    int       lowOffset;
    int       highOffset;
    int       origin;
    int       *shapeType;
    int       *shapeSize;
    int       *shapeCnt;
    int       nShapes;
    int       *matList;
    int       bndMethod;
    FaceHash  faceHash;
} CalcExternalFacesState;

PRIVATE Face *AllocFace(void);
PRIVATE DBfacelist *CalcExternalFaces(int *zoneList, int nNodes,
    int lowOffset, int highOffset, int origin, int *shapeType, int *shapeSize,
    int *shapeCnt, int nShapes, int *matList, int bndMethod);
PRIVATE DBfacelist *FormFaceList(CalcExternalFacesState *st);
PRIVATE void FreeFace(Face *face);
PRIVATE void InsertFace(CalcExternalFacesState *st, int *nodes, int nNodes,
    int zoneNo);

/***********************************************************************
 *
 * Purpose:  Given a zonelist, calculate a facelist describing all of
 *           the external faces.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    nodeList  : List of nodes making up the zones in the mesh.
 *    nNodes    : Number of nodes in mesh.
 *    origin    : Origin for nodelist (0 or 1).
 *    shapeSize : For each shape, the number of nodes per zone.
 *    shapeCnt  : For each shape, the number of zones of that size.
 *    nShapes   : Number of zone shapes.
 *    matList   : Zonal array giving material numbers (else NULL).
 *    bndMethod : Method to use regarding boundaries.
 *
 * Output arguments:
 *    fl        : The resulting facelist.
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 **********************************************************************/

PUBLIC DBfacelist *
DBCalcExternalFacelist(int *nodeList, int nNodes, int origin,
                       int *shapeSize, int *shapeCnt, int nShapes,
                       int *matList, int bndMethod)
{
    int         i;
    int         *shapeType=NULL;
    DBfacelist  *fl=NULL;

    shapeType = MALLOC_N(int, nShapes);
    for (i = 0; i < nShapes; i++)
    {
        switch (shapeSize[i])
        {
            case 4:
                shapeType[i] = DB_ZONETYPE_TET;
                break;
            case 5:
                shapeType[i] = DB_ZONETYPE_PYRAMID;
                break;
            case 6:
                shapeType[i] = DB_ZONETYPE_PRISM;
                break;
            case 8:
                shapeType[i] = DB_ZONETYPE_HEX;
                break;
        }
    }
     
    fl = CalcExternalFaces(nodeList, nNodes, 0, 0, origin, shapeType,
                           shapeSize, shapeCnt, nShapes, matList, bndMethod);

    FREE(shapeType);

    return fl;
}

/***********************************************************************
 *
 * Purpose:  Given a zonelist, calculate a facelist describing all of
 *           the external faces.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    nodeList  : List of nodes making up the zones in the mesh.
 *    nNodes    : Number of nodes in mesh.
 *    lowOffset : The number of ghost zones at the beginning of
 *                the zonelist.
 *    highOffset : The number of ghost zones at the end of the
 *                zonelist.
 *    origin    : Origin for nodelist (0 or 1).
 *    shapeType : For each shape, the type of the shape.
 *    shapeSize : For each shape, the number of nodes per zone.
 *    shapeCnt  : For each shape, the number of zones of that size.
 *    nShapes   : Number of zone shapes.
 *    matList   : Zonal array giving material numbers (else NULL).
 *    bndMethod : Method to use regarding boundaries.
 *
 * Output arguments:
 *    fl        : The resulting facelist.
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 **********************************************************************/

PUBLIC DBfacelist *
DBCalcExternalFacelist2(int *nodeList, int nNodes, int lowOffset,
                        int highOffset, int origin, int *shapeType,
                        int *shapeSize, int *shapeCnt, int nShapes,
                        int *matList, int bndMethod)
{
    DBfacelist *fl=NULL;

    fl = CalcExternalFaces(nodeList, nNodes, lowOffset, highOffset,
                           origin, shapeType, shapeSize, shapeCnt,
                           nShapes, matList, bndMethod);

    return fl;
}

/***********************************************************************
 *
 * Purpose:  Given a zonelist, calculate a facelist describing all of
 *           the external faces.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    zoneList  : List of nodes making up the zones in the mesh.
 *    nNodes    : Number of nodes in mesh.
 *    lowOffset : The number of ghost zones at the beginning of
 *                the zonelist.
 *    highOffset : The number of ghost zones at the end of the
 *                zonelist.
 *    origin    : Origin for nodelist (0 or 1).
 *    shapeType : For each shape, the type of the shape.
 *    shapeSize : For each shape, the number of nodes per zone.
 *    shapeCnt  : For each shape, the number of zones of that size.
 *    nShapes   : Number of zone shapes.
 *    matList   : Zonal array giving material numbers (else NULL).
 *    bndMethod : Method to use regarding boundaries.
 *
 * Output arguments:
 *    faceList : The resulting facelist.
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 *    Jeremy Meredith, Thu Mar 23 09:44:40 PST 2000
 *    Fixed the tetrahedron node ordering.
 *
 *    Jeremy Meredith, Fri Apr 28 13:58:05 PDT 2000
 *    Allowed 2D zone types.  Simply copy the shape as a polygon.
 *
 *    Jeremy Meredith, Fri Jun 16 14:31:36 PDT 2000
 *    Allowed 1D zone type (BEAM).  Simply copy the shape as a line segment.
 *
 *    Hank Childs, Wed Feb 16 16:14:52 PST 2005
 *    Reverse hexahedrons faces, since we are now defining hexes to have
 *    the opposite orientations.
 *
 **********************************************************************/

PRIVATE DBfacelist *
CalcExternalFaces(int *zoneList, int nNodes, int lowOffset, int highOffset,
                  int origin, int *shapeType, int *shapeSize, int *shapeCnt,
                  int nShapes, int *matList, int bndMethod)
{
    int       i, j, k;
    CalcExternalFacesState st;
    int       nFaces;
    int       nEdges;
    int       iZone, iZoneList;
    int       nodes[4];
    DBfacelist *faceList=NULL;

    /*
     * Copy relevant global information to a structure for easy
     * passing among routines.
     */
    st.zoneList   = zoneList;
    st.nNodes     = nNodes;
    st.lowOffset  = lowOffset;
    st.highOffset = highOffset;
    st.origin     = origin;
    st.shapeType  = shapeType;
    st.shapeSize  = shapeSize;
    st.shapeCnt   = shapeCnt;
    st.nShapes    = nShapes;
    st.matList    = matList;
    st.bndMethod  = bndMethod;

    st.faceHash.size  = MIN(nNodes, HASH_MAX);
    st.faceHash.table = MALLOC_N(Face *, st.faceHash.size);
    memset(st.faceHash.table, 0, st.faceHash.size * sizeof(Face *));

    /*
     * Loop over all the shapes, adding the faces for each shape,
     * removing duplicates as they are encountered.
     */
    iZone = 0;
    iZoneList = 0;
    for (i = 0; i < nShapes; i++)
    {
        switch (shapeType[i])
        {
            case DB_ZONETYPE_TET:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+1];
                    nodes[2] = zoneList[iZoneList+2];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+2];
                    nodes[2] = zoneList[iZoneList+3];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+3];
                    nodes[2] = zoneList[iZoneList+1];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+1];
                    nodes[1] = zoneList[iZoneList+3];
                    nodes[2] = zoneList[iZoneList+2];
                    InsertFace(&st, nodes, 3, iZone);
                    iZone++;
                    iZoneList += 4;
                }
                break;
            case DB_ZONETYPE_PYRAMID:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+1];
                    nodes[2] = zoneList[iZoneList+2];
                    nodes[3] = zoneList[iZoneList+3];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+4];
                    nodes[2] = zoneList[iZoneList+1];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+1];
                    nodes[1] = zoneList[iZoneList+4];
                    nodes[2] = zoneList[iZoneList+2];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+2];
                    nodes[1] = zoneList[iZoneList+4];
                    nodes[2] = zoneList[iZoneList+3];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+3];
                    nodes[1] = zoneList[iZoneList+4];
                    nodes[2] = zoneList[iZoneList+0];
                    InsertFace(&st, nodes, 3, iZone);
                    iZone++;
                    iZoneList += 5;
                }
                break;
            case DB_ZONETYPE_PRISM:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+1];
                    nodes[2] = zoneList[iZoneList+2];
                    nodes[3] = zoneList[iZoneList+3];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+3];
                    nodes[1] = zoneList[iZoneList+2];
                    nodes[2] = zoneList[iZoneList+5];
                    nodes[3] = zoneList[iZoneList+4];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+4];
                    nodes[1] = zoneList[iZoneList+5];
                    nodes[2] = zoneList[iZoneList+1];
                    nodes[3] = zoneList[iZoneList+0];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+3];
                    nodes[1] = zoneList[iZoneList+4];
                    nodes[2] = zoneList[iZoneList+0];
                    InsertFace(&st, nodes, 3, iZone);
                    nodes[0] = zoneList[iZoneList+1];
                    nodes[1] = zoneList[iZoneList+5];
                    nodes[2] = zoneList[iZoneList+2];
                    InsertFace(&st, nodes, 3, iZone);
                    iZone++;
                    iZoneList += 6;
                }
                break;
            case DB_ZONETYPE_HEX:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+3];
                    nodes[2] = zoneList[iZoneList+2];
                    nodes[3] = zoneList[iZoneList+1];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+1];
                    nodes[1] = zoneList[iZoneList+2];
                    nodes[2] = zoneList[iZoneList+6];
                    nodes[3] = zoneList[iZoneList+5];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+5];
                    nodes[1] = zoneList[iZoneList+6];
                    nodes[2] = zoneList[iZoneList+7];
                    nodes[3] = zoneList[iZoneList+4];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+4];
                    nodes[1] = zoneList[iZoneList+7];
                    nodes[2] = zoneList[iZoneList+3];
                    nodes[3] = zoneList[iZoneList+0];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+0];
                    nodes[1] = zoneList[iZoneList+1];
                    nodes[2] = zoneList[iZoneList+5];
                    nodes[3] = zoneList[iZoneList+4];
                    InsertFace(&st, nodes, 4, iZone);
                    nodes[0] = zoneList[iZoneList+3];
                    nodes[1] = zoneList[iZoneList+7];
                    nodes[2] = zoneList[iZoneList+6];
                    nodes[3] = zoneList[iZoneList+2];
                    InsertFace(&st, nodes, 4, iZone);
                    iZone++;
                    iZoneList += 8;
                }
                break;
            case DB_ZONETYPE_POLYHEDRON:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    nFaces = zoneList[iZoneList++];
                    for (k = 0; k < nFaces; k++)
                    {
                        nEdges = zoneList[iZoneList++];
                        InsertFace(&st, &zoneList[iZoneList], nEdges, iZone);
                        iZoneList += nEdges;
                    }
                    iZone++;
                }
                break;
            case DB_ZONETYPE_BEAM:
            case DB_ZONETYPE_TRIANGLE:
            case DB_ZONETYPE_QUAD:
            case DB_ZONETYPE_POLYGON:
                for (j = 0; j < shapeCnt[i]; j++)
                {
                    InsertFace(&st, &zoneList[iZoneList], shapeSize[i], iZone);
                    iZoneList += shapeSize[i];
                    iZone++;
                }
                break;
            default:
                iZone += shapeCnt[i];
                iZoneList += shapeSize[i] * shapeCnt[i];
                break;
        }
    }

    /*
     * Form a DBfacelist structure from the remaining faces.
     */
    faceList = FormFaceList(&st);

    return faceList;
}

/***********************************************************************
 *
 * Purpose:  Form a DBfacelist structure from the remaining faces in
 *           the face hash table.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    st       : The external facelist state.
 *
 * Output arguments:
 *    fl       : The resulting facelist.
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *    Eric Brugger, Fri Sep 24 08:53:32 PDT 1999
 *    Modify the routine to handle the case where there are no
 *    external faces.
 *
 *    Jeremy Meredith, Thu Jul  6 11:05:12 PDT 2000
 *    Added the origin to the final zoneno[] array.  It was always
 *    creating it as a zero-origin array.
 *
 *    Eric Brugger, Fri Dec 29 08:34:38 PST 2000
 *    Replaced a comparison to NULL with a comparison to 0 since the
 *    value was an integer not a pointer.
 *
 **********************************************************************/

PRIVATE DBfacelist *
FormFaceList(CalcExternalFacesState *st)
{
    int       i, j;
    FaceHash  *faceHash=NULL;
    int       origin;
    int       minIndex, maxIndex;
    int       nZones;
    int       iFaceList;
    int       lFaceList;
    int       iFace;
    int       nFaces;
    Face      firstFace;
    Face      *prevFace=NULL, *curFace=NULL, *tmpFace=NULL;
    int       zoneNoNext;
    int       *faceList=NULL, *zoneNo=NULL;
    int       curSize;
    int       nShapes;
    int       lShapeList;
    int       *shapeCnt=NULL, *shapeSize=NULL;
    DBfacelist *fl=NULL;

    faceHash = &(st->faceHash);
    origin = st->origin;

    /*
     * Calculate the number of zones in the zonelist.
     */
    nZones = 0;
    for (i = 0; i < st->nShapes; i++) nZones += st->shapeCnt[i];

    minIndex = st->lowOffset;
    maxIndex = nZones - st->highOffset - 1;

    /*
     * Link all the faces into a list and determine the total number
     * faces and the total number of nodes in the resulting facelist.
     * Faces that are associated with ghost elements are eliminated
     * during the chaining process.  The first element in the list is
     * an extra face that is allocated from the stack.  This eliminates
     * special cases in the coding.  Once the faces have been linked
     * together the hash table is no longer needed so it is freed.
     */
    firstFace.nNodes = 0;
    firstFace.nodes  = NULL;
    firstFace.zoneNo = 0;
    firstFace.next   = NULL;
    curFace = &firstFace;
    nFaces = 0;
    lFaceList = 0;
    for (i = 0; i < faceHash->size; i++)
    {
        if (faceHash->table[i] != NULL)
        {
            curFace->next = faceHash->table[i];
            while (curFace->next != NULL)
            {
                /*
                 * If the face came from a real zone, add it to
                 * the list, otherwise delete it.
                 */
                zoneNoNext = curFace->next->zoneNo;
                if (zoneNoNext >= minIndex && zoneNoNext <= maxIndex)
                {
                    nFaces++;
                    lFaceList += curFace->nNodes;
                    curFace = curFace->next;
                }
                else
                {
                    tmpFace = curFace->next;
                    curFace->next = tmpFace->next;
                    FreeFace(tmpFace);
                }
            }
        }
    }
    lFaceList += curFace->nNodes;
    FREE(faceHash->table);
    faceHash->size = 0;

    /*
     * Build the arrays necessary for the DBfacelist structure.  Loop
     * over all the faces, extracting all the faces of a particular type
     * each time until all the faces have been extracted.
     */
    nShapes    = 0;
    lShapeList = 10;
    if (nFaces != 0)
    {
        shapeSize  = MALLOC_N(int, lShapeList);
        shapeCnt   = MALLOC_N(int, lShapeList);
        faceList   = MALLOC_N(int, lFaceList);
        zoneNo     = MALLOC_N(int, nFaces);
    }
    iFace      = 0;
    iFaceList  = 0;

    while (firstFace.next != NULL)
    {
        /*
         * Allocate more space for the shape structures if necessary.
         */
        if (nShapes >= lShapeList)
        {
            lShapeList += 10;
            shapeSize = REALLOC_N(shapeSize, int, lShapeList);
            shapeCnt  = REALLOC_N(shapeCnt, int, lShapeList);
        }

        /*
         * Add an entry in the shape structure for the current shape
         * type.
         */
        prevFace = &firstFace;
        curFace  = firstFace.next;
        curSize  = curFace->nNodes;
        shapeSize[nShapes] = curSize;
        shapeCnt[nShapes]  = 0;

        /*
         * Loop over all the faces, adding all the faces with the current
         * size to the output face list and removing them from the input
         * list of faces.
         */
        while (curFace != NULL)
        {
            if (curFace->nNodes == curSize)
            {
                shapeCnt[nShapes] += 1;
                zoneNo[iFace] = curFace->zoneNo + origin;
                iFace++;
                for (j = 0; j < curSize; j++)
                {
                    faceList[iFaceList+j] = curFace->nodes[j];
                }
                iFaceList += curSize;
                prevFace->next = curFace->next;
                FreeFace(curFace);
                curFace = prevFace;
            }
            prevFace = curFace;
            curFace = curFace->next;
        }

        nShapes++;
    }

    /*
     * Put all the pieces together into the DBfacelist structure.
     */
    fl = DBAllocFacelist();

    fl->ndims     = 3;
    fl->nfaces    = nFaces;
    fl->origin    = origin;
    fl->nodelist  = faceList;
    fl->lnodelist = lFaceList;
    fl->nshapes   = nShapes;
    fl->shapecnt  = shapeCnt;
    fl->shapesize = shapeSize;
    fl->zoneno    = zoneNo;

    return fl;
}

/***********************************************************************
 *
 * Purpose:  Insert a face into the face hash table.  If the face is
 *           not in the hash table it gets added.  If it already exists
 *           then the existing one is deleted.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    st       : The external facelist state.
 *    nodes    : The nodes making up the face.
 *    nNodes   : The number of nodes in the face.
 *    zoneNo   : The zone number associated with the face.
 *
 * Output arguments:
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 **********************************************************************/

PRIVATE void
InsertFace(CalcExternalFacesState *st, int *nodes, int nNodes, int zoneNo)
{
    int       i, j;
    FaceHash  *faceHash=NULL;
    int       bndMethod;
    int       *matList;
    int       iMin;
    int       match;
    int       deleteIt;
    Face      *prevFace=NULL, *curFace=NULL;

    faceHash  = &(st->faceHash);
    bndMethod = st->bndMethod;
    matList   = st->matList;

    /*
     * Find index of the minimum node number in the node list
     * for the face.  Since it is used to index into the hash
     * table and as the starting point for performing a match.
     */
    iMin = 0;
    for (i = 1; i < nNodes; i++)
    {
        if (nodes[i] < nodes[iMin]) iMin = i;
    }

    /*
     * Search the list of faces in the linked list of possible
     * matches for a match.
     */
    curFace = faceHash->table[nodes[iMin] % faceHash->size];
    match = 0;
    while (match == 0 && curFace != NULL)
    {
        if (curFace->nNodes == nNodes)
        {
            match = 1;
            j = (iMin + nNodes - 1) % nNodes;
            for (i = 1; i < nNodes && match == 1; i++)
            {
                match = (curFace->nodes[i] == nodes[j]);
                j = (j + nNodes - 1) % nNodes;
            }
        }

        if (match == 0)
        {
            prevFace = curFace;
            curFace = curFace->next;
        }
    }

    /*
     * Determine if the face should be deleted based on getting
     * a match and the bndMethod.
     */
    deleteIt = 0;
    if (match == 1)
    {
        if (bndMethod == 0)
        {
            deleteIt = 1;
        }
        else if (matList[curFace->zoneNo] == matList[zoneNo])
        {
            deleteIt = 1;
        }
    }
        
    /*
     * Either delete the matching face or add the new one based on
     * the deleteIt flag.
     */
    if (deleteIt == 1)
    {
        /*
         * Delete the face.
         */
        if (prevFace == NULL)
        {
            /*
             * The previous element is the beginning of the list.
             */
            faceHash->table[nodes[iMin] % faceHash->size] = curFace->next;
        }
        else
        {
            /*
             * The previous element is in the middle of the list.
             */
            prevFace->next = curFace->next;
        } 
        FreeFace(curFace);
    }
    else
    {
        /*
         * Add the face.
         */
        curFace = AllocFace();
        curFace->nNodes = nNodes;
        curFace->nodes = MALLOC_N(int, nNodes);
        for (i = 0, j = iMin; i < nNodes; i++, j = (j + 1) % nNodes)
        {
            curFace->nodes[i] = nodes[j];
        }
        curFace->zoneNo = zoneNo;
        curFace->next = faceHash->table[nodes[iMin] % faceHash->size];
        faceHash->table[nodes[iMin] % faceHash->size] = curFace;
    }
}

/***********************************************************************
 *
 * Purpose:  Free a Face structure.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *    face     : The face to free.
 *
 * Output arguments:
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 **********************************************************************/

PRIVATE void
FreeFace(Face *face)
{

    FREE(face->nodes);
    FREE(face);
}

/***********************************************************************
 *
 * Purpose:  Allocate a Face structure.
 *
 * Programmer: Eric Brugger
 * Date:       March 12, 1999
 *
 * Input arguments:
 *
 * Output arguments:
 *    face     : The newly allocated face.
 *
 * Input/Output arguments:
 *
 * Notes
 *
 * Modifications:
 *
 **********************************************************************/

PRIVATE Face *
AllocFace(void)
{
    Face      *face=NULL;

    face = MALLOC_N(Face, 1);

    return face;
}

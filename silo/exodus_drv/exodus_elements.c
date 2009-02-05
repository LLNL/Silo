#include "exodus_elements.h"
#include <silo.h>
#include <exodusII.h>
#include <string.h>

elem_name_to_type_struct elem_name_to_type_map[] =
{
    {"CIRCLE",      ET_CIRCLE},
    {"SPHERE",      ET_SPHERE},
    {"TRUSS",       ET_TRUSS},
    {"BEAM",        ET_BEAM},
    {"SHELL",       ET_SHELL},
    {"TRIANGLE",    ET_TRIANGLE},
    {"TRIAN",       ET_TRIANGLE},
    {"QUAD",        ET_QUAD},
    {"TETRA",       ET_TETRA},
    {"TETRA10",     ET_TETRA},
    {"WEDGE",       ET_WEDGE},
    {"HEX",         ET_HEX},
    {"HEX8",        ET_HEX},
    {NULL,          ET_NULL}
};

elem_type
get_elem_type(const char *name)
{
    int i=0;
    for (i=0; elem_name_to_type_map[i].name != NULL; i++)
    {
        if (strcasecmp(name, elem_name_to_type_map[i].name) == 0)
            return elem_name_to_type_map[i].type;
    }
    return ET_NULL;
}

/* ------------------------------------------------------------------------- */
/*                             conversion tables                             */
/* ------------------------------------------------------------------------- */

/* ---- circle (unsupported) ---- */
/*
int node_map_circle[]    = {1};
int side_map_circle_1[]  = ??;
int side_map_circle_c[]  = ??;
int side_map_circle_t[]  = ??;
int side_map_circle[]    = ??;
element_map_struct map_circle = ?? 
*/

/* ---- sphere (unsupported) ---- */
/*
int node_map_sphere[]    = {1};
int side_map_sphere_1[]  = ??;
int side_map_sphere_c[]  = ??;
int side_map_sphere_t[]  = ??;
int side_map_sphere[]    = ??;
element_map_struct map_sphere = ?? 
*/

/* ---- truss (unsupported) ---- */
/*
int node_map_truss[]     = {1, 2};
int side_map_truss_1[]   = ??;
int side_map_truss_c[]   = ??;
int side_map_truss_t[]   = ??;
int side_map_truss[]     = ??;
element_map_struct map_truss = ?? 
*/

/* ---- beam (unsupported) ---- */
/*
int node_map_beam[]      = {1, 2};
int side_map_beam_1[]    = ??;
int side_map_beam_c[]    = ??;
int side_map_beam_t[]    = ??;
int side_map_beam[]      = ??;
element_map_struct map_beam = ?? 
*/

/* ---- quad ---- */

int node_map_quad[]       = {1, 2, 3, 4};

int side_map_quad_1[]     = {1, 2};
int side_map_quad_2[]     = {2, 3};
int side_map_quad_3[]     = {3, 4};
int side_map_quad_4[]     = {4, 1};

int side_map_quad_c[]     = {2, 2, 2, 2};
int side_map_quad_t[]     = {DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
int* side_map_quad[]      = {side_map_quad_1, side_map_quad_2, side_map_quad_3, side_map_quad_4};

element_map_struct map_quad =
{
    DB_ZONETYPE_QUAD,
    {4, node_map_quad},
    {4, side_map_quad_c, side_map_quad_t, side_map_quad}
};

/* ---- shell-2D (unsupported) ---- */
/*
int node_map_shell2[]    = {1, 2};
int side_map_shell2_1[]  = ??;
int side_map_shell2_c[]  = ??;
int side_map_shell2_t[]  = ??;
int side_map_shell2[]    = ??;
element_map_struct map_shell2 = ?? 
*/

/* ---- shell-3D ---- */

int node_map_shell3[]     = {1, 2, 3, 4};

int side_map_shell3_1[]   = {1, 2, 3, 4};
int side_map_shell3_2[]   = {4, 3, 2, 1};
int side_map_shell3_3[]   = {1, 2};
int side_map_shell3_4[]   = {2, 3};
int side_map_shell3_5[]   = {3, 4};
int side_map_shell3_6[]   = {4, 1};

int side_map_shell3_c[]   = {4, 4, 2, 2, 2, 2};
int side_map_shell3_t[]   = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
int *side_map_shell3[]     = {side_map_shell3_1, side_map_shell3_2, side_map_shell3_3, side_map_shell3_4, side_map_shell3_5, side_map_shell3_6};

element_map_struct map_shell3 =
{
    DB_ZONETYPE_QUAD,
    {4, node_map_shell3},
    {6, side_map_shell3_c, side_map_shell3_t, side_map_shell3}
};

/* ---- triangle ---- */

int node_map_triangle[]   = {1, 2, 3};

int side_map_triangle_1[] = {1, 2};
int side_map_triangle_2[] = {2, 3};
int side_map_triangle_3[] = {3, 1};

int side_map_triangle_c[] = {2, 2, 2};
int side_map_triangle_t[] = {DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM, DB_ZONETYPE_BEAM};
int *side_map_triangle[]   = {side_map_triangle_1, side_map_triangle_2, side_map_triangle_3};

element_map_struct map_triangle =
{
    DB_ZONETYPE_TRIANGLE,
    {3, node_map_triangle},
    {3, side_map_triangle_c, side_map_triangle_t, side_map_triangle}
};

/* ---- tetrahedron ---- */

int node_map_tetra[]      = {1, 2, 4, 3};

int side_map_tetra_1[]    = {1, 2, 4};
int side_map_tetra_2[]    = {2, 3, 4};
int side_map_tetra_3[]    = {3, 1, 4};
int side_map_tetra_4[]    = {1, 3, 2};

int side_map_tetra_c[]    = {3, 3, 3, 3};
int side_map_tetra_t[]    = {DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE};
int *side_map_tetra[]      = {side_map_tetra_1, side_map_tetra_2, side_map_tetra_3, side_map_tetra_4};

element_map_struct map_tetra =
{
    DB_ZONETYPE_TET,
    {4, node_map_tetra},
    {4, side_map_tetra_c, side_map_tetra_t, side_map_tetra}
};

/* ---- wedge ---- */

int node_map_wedge[]      = {4, 1, 2, 5, 6, 3};
 
int side_map_wedge_1[]    = {1, 2, 5, 4};
int side_map_wedge_2[]    = {2, 3, 6, 5};
int side_map_wedge_3[]    = {3, 1, 4, 6};
int side_map_wedge_4[]    = {3, 2, 1};
int side_map_wedge_5[]    = {4, 5, 6};

int side_map_wedge_c[]    = {4, 4, 4, 3, 3};
int side_map_wedge_t[]    = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_TRIANGLE, DB_ZONETYPE_TRIANGLE};
int *side_map_wedge[]      = {side_map_wedge_1, side_map_wedge_2, side_map_wedge_3, side_map_wedge_4, side_map_wedge_5};

element_map_struct map_wedge =
{
    DB_ZONETYPE_PRISM,
    {6, node_map_wedge},
    {5, side_map_wedge_c, side_map_wedge_t, side_map_wedge}
};

/* ---- hexahedron ---- */

int node_map_hex[]        = {4, 3, 2, 1, 8, 7, 6, 5};

int side_map_hex_1[]      = {1, 2, 6, 5};
int side_map_hex_2[]      = {2, 3, 7, 6};
int side_map_hex_3[]      = {3, 4, 8, 7};
int side_map_hex_4[]      = {4, 1, 5, 8};
int side_map_hex_5[]      = {4, 3, 2, 1};
int side_map_hex_6[]      = {5, 6, 7, 8};

int side_map_hex_c[]      = {4, 4, 4, 4, 4, 4};
int side_map_hex_t[]      = {DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD, DB_ZONETYPE_QUAD};
int *side_map_hex[]       = {side_map_hex_1, side_map_hex_2, side_map_hex_3, side_map_hex_4, side_map_hex_5, side_map_hex_6};

element_map_struct map_hex =
{
    DB_ZONETYPE_HEX,
    {8, node_map_hex},
    {6, side_map_hex_c, side_map_hex_t, side_map_hex}
};


/* ------------------------------------------------------------------------- */
/*                       full element conversion table                       */
/* ------------------------------------------------------------------------- */

element_map_struct *map_2d[ET_NUM_ELEM_TYPE] =
{
    NULL,     /* null            */
    NULL,     /* unsup. circle   */
    NULL,     /* unsup. sphere   */
    NULL,     /* unsup. truss    */
    NULL,     /* unsup. beam     */
    NULL,     /* unsup. 2d shell */
    &map_triangle,
    &map_quad,
    &map_tetra,
    &map_wedge,
    &map_hex
};

element_map_struct *map_3d[ET_NUM_ELEM_TYPE] =
{
    NULL,     /* null            */
    NULL,     /* unsup. circle   */
    NULL,     /* unsup. sphere   */
    NULL,     /* unsup. truss    */
    NULL,     /* unsup. beam     */
    &map_shell3,
    &map_triangle,
    &map_quad,
    &map_tetra,
    &map_wedge,
    &map_hex
};

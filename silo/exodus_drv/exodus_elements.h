#ifndef EXODUS_ELEMENTS_H
#define EXODUS_ELEMENTS_H


typedef enum 
{
    ET_NULL   = 0,
    ET_CIRCLE,
    ET_SPHERE,
    ET_TRUSS,
    ET_BEAM,
    ET_SHELL,
    ET_TRIANGLE,
    ET_QUAD,
    ET_TETRA,
    ET_WEDGE,
    ET_HEX,
    ET_NUM_ELEM_TYPE
} elem_type;

typedef struct
{
    char       *name;
    elem_type   type;
} elem_name_to_type_struct;

typedef struct
{
    int  n_nodes;
    int *nodes;
} node_map_struct;

typedef struct
{
    int   n_sides;
    int  *side_n_nodes;
    int  *side_types;
    int **side_nodes;
} side_map_struct;

typedef struct
{
    int             silo_type;
    node_map_struct node_map;
    side_map_struct side_map;
} element_map_struct;



extern element_map_struct *map_2d[];
extern element_map_struct *map_3d[];

elem_type   get_elem_type(const char*);

#endif


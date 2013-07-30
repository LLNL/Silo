#include <stdio.h>

extern void PrintFileComponentTypes(char const *filename, FILE* outf, int show_all_errors, int test_fic_vfd);

/*********************************************************************
 *
 * Purpose: Main function for listtypes.c. This function iterates
 *          over the command line arguments and supplies them to
 *          a function that prints the component types for a file.
 *          This program tests the DBGetComponentType function.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:05:37 PST 2000
 *
 * Input Arguments:
 *     argc : The number of command line arguments.
 *     argv : An array containing the command line arguments.
 *
 * Modifications:
 *     Thomas R. Treadway, Thu Jul  5 16:33:38 PDT 2007
 *     Chaneged main's return type to int, to stop gcc-4.x whining.
 *
 ********************************************************************/

int
main(int argc, char *argv[])
{
    int i;
    int test_fic_vfd = 0;
    int show_all_errors = 0;

    if (argc < 2)
    {
        printf("Usage: listtypes filename [filename ...]\n");
        return 0;
    }

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "show-all-errors"))
            show_all_errors = 1;
        else if (!strcmp(argv[i], "test-fic-vfd"))
            test_fic_vfd = 1;
        else
            PrintFileComponentTypes(argv[i], stdout, show_all_errors, test_fic_vfd);
    }
    
    return 0;
}

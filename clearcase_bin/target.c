#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

int main(int argc, char **argv, char **env)
{
    char *target = NULL;
    char *e = NULL;
    int i, total;
    char **newenv = NULL;

    for(total=0; env[total] != NULL; total++)
        ; /* Do nothing */

    newenv = (char**)malloc(sizeof(char*)*(total+2));
    for(i=0;i<total;i++)
        newenv[i] = strdup(env[i]);

    target = (char*)malloc(strlen(argv[0])+8);
    sprintf(target,"TARGET=%s",argv[0]);
    newenv[total] = strdup(target);
    free(target);
    newenv[total+1] = NULL;

    execve("/home/meshtv/clearcase_bin/_target",argv,newenv);

    fprintf(stderr,"Unable to execute /home/meshtv/clearcase_bin/_target.\n");
    /* We should never get here, but let's be safe. */
    for(i=0;i<total;i++)
        free(newenv[i]);
    free(newenv[i+1]);
    free(newenv);
    return(0);
}

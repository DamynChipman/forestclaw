#ifndef CUDACLAW_ALLOCATE_H
#define CUDACLAW_ALLOCATE_H


/* Only include headers needed to get this file to compile;  all other
   headers should go in c files */

// #include "../fc2d_cudaclaw.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct fclaw2d_patch;
struct fclaw2d_global;

typedef struct cudaclaw_fluxes
{
    size_t num_bytes;   /* All members have the same size */
    size_t num_bytes_aux;  
    size_t num_bytes_waves;  
    size_t num_bytes_speeds;  

    double *qold_dev;
    double *aux_dev;
    
    double *fp_dev;
    double *fm_dev;
    double *gp_dev;
    double *gm_dev;

    double *waves_dev;
    double *speeds_dev;
    
} cudaclaw_fluxes_t;

void cudaclaw_allocate_fluxes(struct fclaw2d_global *glob,
                               struct fclaw2d_patch *patch);

void cudaclaw_deallocate_fluxes(struct fclaw2d_global *glob,
                                 struct fclaw2d_patch *patch);


#ifdef __cplusplus
}
#endif
#endif

#include "cudaclaw5_allocate.h"

#include <fclaw2d_global.h>
#include <fclaw2d_patch.h>
#include <fclaw2d_clawpatch.h>
#include <fclaw_timer.h>



void cudaclaw5_allocate_fluxes(struct fclaw2d_global *glob,
                               struct fclaw2d_patch *patch)
{
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    float milliseconds = 0;

    size_t size = fclaw2d_clawpatch_size(glob);

    cudaclaw5_fluxes_t *fluxes = FCLAW_ALLOC(cudaclaw5_fluxes,1);
    fluxes->num_bytes = size*sizeof(double);

    /* CPU memory allocation */
    fluxes->fp = new double[size];
    fluxes->fm = new double[size];
    fluxes->gp = new double[size];
    fluxes->gm = new double[size];


    cudaEventRecord(start);
    cudaMalloc((void**)&fluxes->qold_dev, fluxes->num_bytes);
    cudaMalloc((void**)&fluxes->fm_dev,   fluxes->num_bytes);
    cudaMalloc((void**)&fluxes->fp_dev,   fluxes->num_bytes);
    cudaMalloc((void**)&fluxes->gm_dev,   fluxes->num_bytes);
    cudaMalloc((void**)&fluxes->gp_dev,   fluxes->num_bytes);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&milliseconds, start, stop);
    glob->timers[FCLAW2D_TIMER_CUDA_ALLOCATE].cumulative += milliseconds*1e-3;

    fclaw2d_patch_set_user_data(glob,patch,fluxes);

    cudaEventDestroy(start);
    cudaEventDestroy(stop);

}

void cudaclaw5_deallocate_fluxes(fclaw2d_global_t *glob,
                                 fclaw2d_patch_t *patch)
{
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    float milliseconds = 0;

    cudaclaw5_fluxes_t *fluxes = (cudaclaw5_fluxes_t*) 
               fclaw2d_patch_get_user_data(glob,patch);

    FCLAW_ASSERT(fluxes != NULL);

    cudaEventRecord(start);
    cudaFree(fluxes->qold_dev);
    cudaFree(fluxes->fm_dev);
    cudaFree(fluxes->fp_dev);
    cudaFree(fluxes->gm_dev);
    cudaFree(fluxes->gp_dev);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&milliseconds, start, stop);
    glob->timers[FCLAW2D_TIMER_CUDA_ALLOCATE].cumulative += milliseconds*1e-3;

    delete [] fluxes->fp;
    delete [] fluxes->fm;
    delete [] fluxes->gp;
    delete [] fluxes->gm;

    FCLAW_FREE((void*) fluxes);

    cudaEventDestroy(start);
    cudaEventDestroy(stop);

}


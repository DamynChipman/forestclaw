/*
Copyright (c) 2012-2022 Carsten Burstedde, Donna Calhoun, Scott Aiton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef REFINE_DIM
#define REFINE_DIM 2
#endif

#ifndef PATCH_DIM
#define PATCH_DIM 2
#endif


#if PATCH_DIM == 2

#define METRIC_VTABLE_NAME "fclaw2d_clawpatch"

#include "fclaw2d_metric.h"
#include "fclaw2d_metric.hpp"

#elif PATCH_DIM == 3

#define METRIC_VTABLE_NAME "fclaw3d_clawpatch"

#include "fclaw3d_metric.h"
#include "fclaw3d_metric.hpp"
#include <_fclaw2d_to_fclaw3d.h>

#endif


#include <fclaw_pointer_map.h>

#include <fclaw2d_global.h>
#include <fclaw2d_patch.h>  

static
fclaw2d_metric_patch_t* get_metric_patch(fclaw2d_global_t* glob,
                                         fclaw2d_patch_t *patch)
{
    return (fclaw2d_metric_patch_t*) fclaw2d_patch_metric_patch(glob, patch);
}


/* ----------------------------- Creating/deleting patches ---------------------------- */

fclaw2d_metric_patch_t* fclaw2d_metric_patch_new()
{
    fclaw2d_metric_patch_t *mp = new fclaw2d_metric_patch_t;
    return mp;
}

void fclaw2d_metric_patch_delete(fclaw2d_metric_patch_t **mp)
{
    FCLAW_ASSERT(mp != NULL);
    delete *mp;
    *mp = NULL;
}


#if PATCH_DIM == 2
void fclaw2d_metric_patch_define(fclaw2d_global_t* glob,
                                 fclaw2d_patch_t* patch, 
                                 int mx, int my, int mbc, 
                                 double dx, double dy,
                                 double xlower, double ylower, 
                                 double xupper, double yupper,
                                 int blockno, int patchno, 
                                 fclaw2d_build_mode_t build_mode)
#elif PATCH_DIM == 3
void fclaw3d_metric_patch_define(fclaw2d_global_t* glob,
                                 fclaw2d_patch_t* patch, 
                                 int mx, int my, int mz, int mbc, 
                                 double dx, double dy,double dz,
                                 double xlower, double ylower, double zlower,
                                 double xupper, double yupper, double zupper,
                                 int blockno, int patchno, 
                                 fclaw2d_build_mode_t build_mode)
#endif
    
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);

    mp->mx   = mx;
    mp->my   = my;
    mp->mbc  = mbc;
    mp->blockno = blockno;

    mp->dx = dx;
    mp->dy = dy;
    mp->xlower = xlower;
    mp->ylower = ylower;
    mp->xupper = xupper;
    mp->yupper = yupper;

#if PATCH_DIM == 3
    mp->mz = mz;
    mp->zlower = zlower;
    mp->zupper = xupper;
#endif    

    /* Set up area for storage - this is needed for ghost patches, 
    and updated patches */
    {
        /* Create box for primary grid (cell-centered) */
        int ll_p[PATCH_DIM];
        int ur_p[PATCH_DIM];
        for (int idir = 0; idir < PATCH_DIM; idir++)
        {
            ll_p[idir] = -mbc;
        }
        ur_p[0] = mp->mx + mbc + 1;
        ur_p[1] = mp->my + mbc + 1;
#if PATCH_DIM == 3
        ur_p[2] = mp->mz + mbc + 1;
#endif

        Box box_p(ll_p,ur_p,PATCH_DIM);

#if PATCH_DIM == 2        
        mp->area.define(box_p,1);
#elif PATCH_DIM == 3
        mp->volume.define(box_p,1);
#endif

    }


    if (build_mode != FCLAW2D_BUILD_FOR_GHOST_AREA_PACKED
        && build_mode == FCLAW2D_BUILD_FOR_UPDATE)
    {
        /* 
        From here on out, we build for patches that get updated 

        In 2d manifold : 
        24 additional field variables needed for all metric terms on a 
        manifold
            xp,yp,zp           : 3
            xd,yd,zd           : 3
            surf_normals       : 3    (3x1 vector)
            curvature          : 1
            area               : 1
            <xy>face normals   : 6    (2 3x1 vectors)
            <xy>face tangents  : 6    (2 3x1 vectors)
            edge lengths       : 2
            -----------------------
            Total              : 25

        In 3d : 
        36 additional field variables needed for all metric terms
            xp,yp,zp           : 3
            xd,yd,zd           : 3
            face areas         : 3
            volume             : 1
            rotation matrix    : 27 (3 3x3 matrices)
            -----------------------
            Total              : 37

            We should come up with a way to store only what is needed 
        */

        int ll_p[PATCH_DIM];
        int ur_p[PATCH_DIM];

        /* Metric  patches have an extra layer of ghost cells */
        for (int idir = 0; idir < PATCH_DIM; idir++)
        {
            ll_p[idir] = -mbc;
        }
        ur_p[0] = mp->mx + mbc + 1;
        ur_p[1] = mp->my + mbc + 1;

#if PATCH_DIM == 3
        ur_p[2] = mp->mz + mbc + 1;
#endif

        Box box_p(ll_p,ur_p,PATCH_DIM);   /* Store cell centered values here */

        /* Mesh cell centers of physical mesh */
        mp->xp.define(box_p,1);
        mp->yp.define(box_p,1);
        mp->zp.define(box_p,1);
#if PATCH_DIM == 2
        mp->surf_normals.define(box_p,3);
        mp->curvature.define(box_p,1);
#endif

        int ll_d[PATCH_DIM];
        int ur_d[PATCH_DIM];

        /* Create node centered box */
        for (int idir = 0; idir < PATCH_DIM; idir++)
        {
            ll_d[idir] = -mbc;
        }
        ur_d[0] = mp->mx + mbc + 2;
        ur_d[1] = mp->my + mbc + 2;

#if PATCH_DIM == 3
        ur_d[2] = mp->mz + mbc + 2;
#endif

        Box box_d(ll_d,ur_d,PATCH_DIM);

        mp->xd.define(box_d,1);
        mp->yd.define(box_d,1);
        mp->zd.define(box_d,1);

#if PATCH_DIM == 2        
        /* Store length of left and bottom edge of box */
        /* Face centered values */
        /* DAC : Why didn't I set this up as a cell-centered box? */
        mp->xface_normals.define(box_d,3);
        mp->yface_normals.define(box_d,3);
        mp->xface_tangents.define(box_d,3);
        mp->yface_tangents.define(box_d,3);

        mp->edge_lengths.define(box_d,2);
#elif PATCH_DIM == 3        
        /* Store face areas of left, front, bottom edge of box */
        mp->face_area.define(box_d,3);

        /* Store 3x3 rotation matrix for each of three faces */
        mp->xrot.define(box_d,9);
        mp->yrot.define(box_d,9);
        mp->zrot.define(box_d,9);
#endif

    }
}



/* For 3d extruded mesh, this averages cell volumes */
static
#if PATCH_DIM == 2
void metric_average_area_from_fine(fclaw2d_global_t *glob,
                                   fclaw2d_patch_t *fine_patches,
                                   fclaw2d_patch_t *coarse_patch,
                                   int blockno, 
                                   int coarse_patchno,
                                   int fine0_patchno)
#elif PATCH_DIM == 3
void metric_average_volume_from_fine(fclaw2d_global_t *glob,
                                     fclaw2d_patch_t *fine_patches,
                                     fclaw2d_patch_t *coarse_patch,
                                     int blockno, 
                                     int coarse_patchno,
                                     int fine0_patchno)
#endif

{
    fclaw2d_metric_vtable_t *metric_vt = fclaw2d_metric_vt(glob);
    int mx,my, mbc;
    double xlower,ylower,dx,dy;

#if PATCH_DIM == 2
    fclaw2d_metric_patch_grid_data(glob,coarse_patch,&mx,&my,&mbc,
                                   &xlower,&ylower,&dx,&dy);
#elif PATCH_DIM == 3
    int mz;
    double zlower, dz;
    fclaw3d_metric_patch_grid_data(glob,coarse_patch,&mx,&my,&mz,&mbc,
                                   &xlower,&ylower,&zlower,&dx,&dy,&dz);    
#endif

#if PATCH_DIM == 2
    double *areacoarse = fclaw2d_metric_patch_get_area(glob, coarse_patch);

    for(int igrid = 0; igrid < 4; igrid++)
    {
        double* areafine = fclaw2d_metric_patch_get_area(glob, &fine_patches[igrid]);

        FCLAW2D_FORT_AVERAGE_AREA(&mx,&my,&mbc,areacoarse,areafine,&igrid);
    }
    metric_vt->compute_area_ghost(glob,coarse_patch,blockno,coarse_patchno);
#elif PATCH_DIM == 3
    double *volcoarse, *fa_coarse;
    fclaw3d_metric_patch_scalar(glob,coarse_patch,&volcoarse, &fa_coarse);

#if REFINE_DIM == 2
    for(int igrid = 0; igrid < 4; igrid++)
    {
        double *volfine, *fa_fine;
        fclaw3d_metric_patch_scalar(glob,&fine_patches[igrid],&volfine, &fa_fine);

        /* Average from fine to coarse when creating coarse grid from a fine grid. 
           This will be exact, since both volume and face areas are computing at 
           finest level resolution.
        */
        FCLAW3DX_METRIC_FORT_AVERAGE_VOLUME(&mx,&my,&mz, &mbc,volcoarse,volfine, &igrid);
        FCLAW3DX_METRIC_FORT_AVERAGE_FACEAREA(&mx,&my,&mz, &mbc,fa_coarse,fa_fine, &igrid);
    }
    metric_vt->compute_volume_ghost(glob,coarse_patch,blockno,coarse_patchno);
#else
    fclaw_global_essential("Average area/vol from fine not implemented for full octree\n");
    exit(0);
#endif
#endif
}

/* --------------------------------- Public interface  -------------------------------- */

#if PATCH_DIM == 2
void fclaw2d_metric_patch_compute_area (fclaw2d_global_t *glob,
                                       fclaw2d_patch_t* patch,
                                       int blockno, int patchno)
{
    fclaw2d_metric_vtable_t *metric_vt = fclaw2d_metric_vt(glob);
    FCLAW_ASSERT(metric_vt->compute_area);
    metric_vt->compute_area(glob,patch,blockno,patchno);
}
#elif PATCH_DIM == 3
void fclaw3d_metric_patch_compute_volume (fclaw2d_global_t *glob,
                                       fclaw2d_patch_t* patch,
                                       int blockno, int patchno)
{
    fclaw2d_metric_vtable_t *metric_vt = fclaw2d_metric_vt(glob);
    FCLAW_ASSERT(metric_vt->compute_volume);
    metric_vt->compute_volume(glob,patch,blockno,patchno);
}
#endif

void fclaw2d_metric_patch_setup(fclaw2d_global_t* glob,
                                fclaw2d_patch_t* patch,
                                int blockno,
                                int patchno)
{
    fclaw2d_metric_vtable_t *metric_vt = fclaw2d_metric_vt(glob);

    /* Setup (xp,yp,zp) and (xd,yd,zd) */
    metric_vt->compute_mesh(glob,patch,blockno,patchno);

    /* In 2d : Surface normals and tangents at each face
       In 3d : Volume, face areas, and rotation matrix for each face. 
    */
    metric_vt->compute_basis(glob,patch,blockno,patchno);
}



void fclaw2d_metric_patch_setup_from_fine(fclaw2d_global_t *glob,
                                          fclaw2d_patch_t *fine_patches,
                                          fclaw2d_patch_t *coarse_patch,
                                          int blockno,
                                          int coarse_patchno,
                                          int fine0_patchno)
{
    fclaw2d_metric_vtable_t *metric_vt = fclaw2d_metric_vt(glob);

#if PATCH_DIM == 2
    metric_average_area_from_fine(glob,fine_patches,coarse_patch,
                                  blockno, coarse_patchno, 
                                  fine0_patchno);
#elif PATCH_DIM == 3
    metric_average_volume_from_fine(glob,fine_patches,coarse_patch,
                                    blockno, coarse_patchno, 
                                    fine0_patchno);
#endif

    metric_vt->compute_mesh(glob,coarse_patch,blockno,coarse_patchno);
    metric_vt->compute_basis(glob,coarse_patch,blockno,coarse_patchno);
}



/* ------------------------------------ Virtual table  -------------------------------- */

static
fclaw2d_metric_vtable_t* metric_vt_new()
{
    return (fclaw2d_metric_vtable_t*) FCLAW_ALLOC_ZERO (fclaw2d_metric_vtable_t, 1);
}

static
void metric_vt_destroy(void* vt)
{
    FCLAW_FREE (vt);
}

fclaw2d_metric_vtable_t* fclaw2d_metric_vt(fclaw2d_global_t* glob)
{
	fclaw2d_metric_vtable_t* metric_vt = (fclaw2d_metric_vtable_t*) 
	   							fclaw_pointer_map_get(glob->vtables, "fclaw2d_metric");
	FCLAW_ASSERT(metric_vt != NULL);
	FCLAW_ASSERT(metric_vt->is_set != 0);
	return metric_vt;
}


void fclaw2d_metric_vtable_initialize(fclaw2d_global_t* glob)  
{
    fclaw2d_metric_vtable_t *metric_vt = metric_vt_new();


    /* Fortran files */
#if PATCH_DIM == 2    
    metric_vt->compute_mesh          = fclaw2d_metric_compute_mesh_default;
    metric_vt->compute_area          = fclaw2d_metric_compute_area_default;
    metric_vt->compute_area_ghost    = fclaw2d_metric_compute_area_ghost_default;
    metric_vt->compute_basis       = fclaw2d_metric_compute_basis_default;

    metric_vt->fort_compute_mesh     = &FCLAW2D_FORT_COMPUTE_MESH;
    metric_vt->fort_compute_normals       = &FCLAW2D_FORT_COMPUTE_NORMALS;
    metric_vt->fort_compute_tangents      = &FCLAW2D_FORT_COMPUTE_TANGENTS;
    metric_vt->fort_compute_surf_normals  = &FCLAW2D_FORT_COMPUTE_SURF_NORMALS;
#elif PATCH_DIM == 3
    metric_vt->compute_mesh          = fclaw3d_metric_compute_mesh_default;
    metric_vt->compute_volume        = fclaw3d_metric_compute_volume_default;
    metric_vt->compute_volume_ghost  = fclaw3d_metric_compute_volume_ghost_default;
    metric_vt->compute_basis         = fclaw3d_metric_compute_basis_default;
#endif

    metric_vt->is_set = 1;

	FCLAW_ASSERT(fclaw_pointer_map_get(glob->vtables,METRIC_VTABLE_NAME) == NULL);
	fclaw_pointer_map_insert(glob->vtables,METRIC_VTABLE_NAME, metric_vt, metric_vt_destroy);
}


/* --------------------------------- Misc access functions ---------------------------- */

/* These functions are not virtualized and are not defined by the 
   patch interface */

fclaw2d_metric_patch_t* fclaw2d_metric_get_metric_patch(fclaw2d_global_t* glob,
                                                        fclaw2d_patch_t* patch)

{
    return get_metric_patch(glob, patch);
}

double* fclaw2d_metric_patch_get_area(fclaw2d_global_t* glob,
                                      fclaw2d_patch_t* patch)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
#if PATCH_DIM == 2
    return mp->area.dataPtr();
#elif PATCH_DIM == 3
    return mp->volume.dataPtr();
#endif
}


/* ----------- See fclaw3d_metric.cpp for 3d versions of functions below -------------- */

#if PATCH_DIM == 2
void fclaw2d_metric_patch_scalar(fclaw2d_global_t* glob,
                                 fclaw2d_patch_t* patch,
                                 double **area, double** edgelengths,
                                 double **curvature)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *area = mp->area.dataPtr();
    *edgelengths =  mp->edge_lengths.dataPtr();
    *curvature = mp->curvature.dataPtr();
}


void fclaw2d_metric_patch_vector(struct fclaw2d_global* glob,
                                 struct fclaw2d_patch* patch,
                                 double **xnormals, double **ynormals,
                                 double **xtangents, double **ytangents,
                                 double **surfnormals)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *xnormals = mp->xface_normals.dataPtr();
    *ynormals = mp->yface_normals.dataPtr();
    *xtangents = mp->xface_tangents.dataPtr();
    *ytangents = mp->yface_tangents.dataPtr();
    *surfnormals = mp->surf_normals.dataPtr();
}


void fclaw2d_metric_patch_grid_data(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t* patch,
                                    int* mx, int* my, int* mbc,
                                    double* xlower, double* ylower,
                                    double* dx, double* dy)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *mx     = mp->mx;
    *my     = mp->my;
    *mbc    = mp->mbc;
    *xlower = mp->xlower;
    *ylower = mp->ylower;
    *dx     = mp->dx;
    *dy     = mp->dy;
}


void fclaw2d_metric_patch_mesh_data(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t* patch,
                                    double **xp, double **yp, double **zp,
                                    double **xd, double **yd, double **zd,
                                    double **area)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *xp = mp->xp.dataPtr();
    *yp = mp->yp.dataPtr();
    *zp = mp->zp.dataPtr();
    *xd = mp->xd.dataPtr();
    *yd = mp->yd.dataPtr();
    *zd = mp->zd.dataPtr();
    *area = mp->area.dataPtr();
}

void fclaw2d_metric_patch_mesh_data2(fclaw2d_global_t* glob,
                                     fclaw2d_patch_t* patch,
                                     double **xnormals, double **ynormals,
                                     double **xtangents, double **ytangents,
                                     double **surfnormals,
                                     double **edgelengths, double **curvature)
{
    fclaw2d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *xnormals    = mp->xface_normals.dataPtr();
    *ynormals    = mp->yface_normals.dataPtr();
    *xtangents   = mp->xface_tangents.dataPtr();
    *ytangents   = mp->yface_tangents.dataPtr();
    *surfnormals = mp->surf_normals.dataPtr();
    *curvature   = mp->curvature.dataPtr();
    *edgelengths = mp->edge_lengths.dataPtr();
}

#elif PATCH_DIM == 3

void fclaw3d_metric_patch_scalar(fclaw2d_global_t* glob,
                                 fclaw2d_patch_t* patch,
                                 double **volume, double** faceareas)
{
    fclaw3d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *volume = mp->volume.dataPtr();
    *faceareas =  mp->face_area.dataPtr();
}


void fclaw3d_metric_patch_basis(fclaw2d_global_t* glob,
                                fclaw2d_patch_t* patch,
                                double **xrot, double **yrot, double **zrot)
{
    fclaw3d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *xrot = mp->xrot.dataPtr();
    *yrot = mp->yrot.dataPtr();
    *zrot = mp->zrot.dataPtr();
}


void fclaw3d_metric_patch_grid_data(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t* patch,
                                    int* mx, int* my, int* mz, 
                                    int* mbc,
                                    double* xlower, double* ylower, double *zlower,
                                    double* dx, double* dy, double* dz)
{
    fclaw3d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *mx     = mp->mx;
    *my     = mp->my;
    *mz     = mp->mz;
    *mbc    = mp->mbc;
    *xlower = mp->xlower;
    *ylower = mp->ylower;
    *zlower = mp->zlower;
    *dx     = mp->dx;
    *dy     = mp->dy;
    *dz     = mp->dz;
}


void fclaw3d_metric_patch_mesh_data(fclaw2d_global_t* glob,
                                    fclaw2d_patch_t* patch,
                                    double **xp, double **yp, double **zp,
                                    double **xd, double **yd, double **zd,
                                    double **volume, double** faceareas)
{
    fclaw3d_metric_patch_t* mp = get_metric_patch(glob, patch);
    *xp = mp->xp.dataPtr();
    *yp = mp->yp.dataPtr();
    *zp = mp->zp.dataPtr();
    *xd = mp->xd.dataPtr();
    *yd = mp->yd.dataPtr();
    *zd = mp->zd.dataPtr();
    *volume = mp->volume.dataPtr();
    *faceareas = mp->face_area.dataPtr();
}

#endif







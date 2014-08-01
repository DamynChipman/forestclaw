/*
Copyright (c) 2012 Carsten Burstedde, Donna Calhoun
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

#ifndef FORESTCLAW2D_H
#define FORESTCLAW2D_H

#include <fclaw2d_base.h>
#include <sc_keyvalue.h>

#ifdef __cplusplus
extern "C"
{
#if 0
}                               /* need this because indent is dumb */
#endif
#endif

/*************************** DATA TYPES ***********************************/

typedef struct fclaw2d_domain fclaw2d_domain_t;
typedef struct fclaw2d_block fclaw2d_block_t;
typedef struct fclaw2d_patch fclaw2d_patch_t;

typedef enum
{
    FCLAW2D_PATCH_CHILDID = 0x7,
    FCLAW2D_PATCH_FIRST_SIBLING = 0x8,
    FCLAW2D_PATCH_ON_PARALLEL_BOUNDARY = 0x10,
    FCLAW2D_PATCH_IS_GHOST = 0x20
}
fclaw2d_patch_flags_t;

/*
 * The domain structure gives a processor local view of the grid hierarchy.
 * Unless explicitly noted otherwise, all variables are processor local,
 * i.e., they are generally different on each processor.
 * Variables that are synchronized and shared between processors
 * are denoted *global*.
 */

struct fclaw2d_patch
{
    int level;                  /* 0 is root, increases if refined */
    int flags;                  /* flags that encode tree information */
    double xlower, xupper;
    double ylower, yupper;
    union
    {
        fclaw2d_patch_t *next;  /* local: next patch same level same block */
        int blockno;            /* off-proc: this patch's block number */
    }
    u;
    void *user;
};

struct fclaw2d_block
{
    double xlower, xupper;
    double ylower, yupper;
    int is_boundary[4];         /* physical boundary flag */
    int num_patches;            /* local patches in this block */
    int num_patches_before;     /* in all previous blocks */
    int minlevel, maxlevel;     /* local over this block.  If this proc doesn't
                                   store any patches in this block, we set
                                   maxlevel < 0 <= minlevel. */
    fclaw2d_patch_t *patches;   /* allocated storage */
    fclaw2d_patch_t **patchbylevel;     /* array of pointers */
    void *user;
};

struct fclaw2d_domain
{
    sc_MPI_Comm mpicomm;        /* MPI communicator */
    int mpisize, mpirank;       /* MPI variables */
    int possible_maxlevel;      /* theoretical maximum that can be reached */

    int local_num_patches;      /* sum of patches over all blocks on this proc */
    int local_minlevel, local_maxlevel; /* proc local.  If this proc doesn't
                                           store any patches at all, we set
                                           local_maxlevel < 0 <= local_minlevel. */
    int64_t global_num_patches; /* sum of local_num_patches over all procs */
    int64_t global_num_patches_before;  /* Number of patches on lower procs */
    int global_minlevel, global_maxlevel;       /* global, well-defined */

    int num_blocks;
    fclaw2d_block_t *blocks;    /* allocated storage */
    int num_exchange_patches;   /* # my patches relevant to other procs.
                                   Identified by this expression to be true:
                                   (patch->flags &
                                   FCLAW2D_PATCH_ON_PARALLEL_BOUNDARY) */
    int num_ghost_patches;      /* # off-proc patches relevant to this proc */
    fclaw2d_patch_t *ghost_patches;     /* array of off-proc patches */

    void *pp;                   /* opaque backend data */
    int pp_owned;               /* The pp member is owned by this domain */
    sc_keyvalue_t *attributes;  /* Reserved to store domain attributes */

    void *user;
};

/***************************** DOMAIN ATTRIBUTES **************************/

/** Add a named attribute to the domain.
 * Attribute names starting with 'fclaw' are reserved.
 * \param [in] domain   This domain will get a new attribute.
 * \param [in] name     This name must not yet be used for another attribute.
 * \param [in] attribute        Arbitrary data stored under \a name.
 */
void fclaw2d_domain_attribute_add (fclaw2d_domain_t * domain,
                                   const char *name, void *attribute);

/** Access a named attribute of the domain.
 * \param [in] domain   The domain may or may not have the queried attribute.
 * \param [in] name     The attribute by this \a name is retrieved.
 * \param [in] default_attr     Returned if the attribute does not exist.
 * \return              The data that was previously stored under \a name,
 *                      or \a default_attr if the attribute does not exist.
 */
void *fclaw2d_domain_attribute_access (fclaw2d_domain_t * domain,
                                       const char *name, void *default_attr);

/** Remove a named attribute from the domain.
 * It is NOT necessary to call this function before domain destruction.
 * \param [in] domain   The domain must have the attribute \a name.
 * \param [in] name     An attribute of this name must exist.
 */
void fclaw2d_domain_attribute_remove (fclaw2d_domain_t * domain,
                                      const char *name);

/*************************** TOPOLOGICAL PROPERTIES ***********************/

/** Return the space dimension. */
int fclaw2d_domain_dimension (const fclaw2d_domain_t * domain);

/** Return the number of faces of a cube: 4 in 2D, 6 in 3D. */
int fclaw2d_domain_num_faces (const fclaw2d_domain_t * domain);

/** Return the number of corners of a cube: 4 in 2D, 8 in 3D.
 * This is the same as the number of siblings in a refined tree. */
int fclaw2d_domain_num_corners (const fclaw2d_domain_t * domain);

/** Return the number of corners of a cube face: 2 in 2D, 4 in 3D.
 * This is the same as the number of refined (smaller) face neighbors. */
int fclaw2d_domain_num_face_corners (const fclaw2d_domain_t * domain);

/** Return the number of possible orientations of a cube face.
 * This is mostly used for internal encodings.
 */
int fclaw2d_domain_num_orientations (const fclaw2d_domain_t * domain);

/** Find the numbers of faces adjacent to a cube corner: 2 in 2D, 3 in 3D. */
void fclaw2d_domain_corner_faces (const fclaw2d_domain_t * domain,
                                  int icorner, int faces[2]);

/*************************** PATCH FUNCTIONS ******************************/

/** Return the dimension of a corner.
 * \param [in] patch    A patch with properly set member variables.
 * \param [in] cornerno A corner number in 0..3.
 * \return              0 if the corner is always at a fourfold intersection,
 *                      1 if the corner would end up in the middle of a face
 *                      when there is a coarser neighbor.
 */
int fclaw2d_patch_corner_dimension (const fclaw2d_patch_t * patch,
                                    int cornerno);

/** Return the number of a patch with respect to its parent in the tree.
 * \param [in] patch    A patch with properly set member variables.
 * \return              The child id is a number in 0..3.
 */
int fclaw2d_patch_childid (const fclaw2d_patch_t * patch);

/** Check if a patch is the first in a family of four siblings.
 * \param [in] patch    A patch with properly set member variables.
 * \return              True if patch is the first sibling.
 */
int fclaw2d_patch_is_first_sibling (const fclaw2d_patch_t * patch);

/** Check if a patch is a parallel ghost patch.
 * \param [in] patch    A patch with properly set member variables.
 * \return              True if patch is off-processor, false if local.
 */
int fclaw2d_patch_is_ghost (const fclaw2d_patch_t * patch);

/************************* ALLOCATION *************************************/

void *fclaw2d_alloc (size_t size);
void *fclaw2d_calloc (size_t nmemb, size_t size);
void *fclaw2d_realloc (void *ptr, size_t size);
void fclaw2d_free (void *ptr);
#define FCLAW2D_ALLOC(t,n)      (t *) fclaw2d_alloc ((n) * sizeof (t))
#define FCLAW2D_ALLOC_ZERO(t,n) (t *) fclaw2d_calloc ((n), sizeof (t))
#define FCLAW2D_REALLOC(p,t,n)  (t *) fclaw2d_realloc ((p), (n) * sizeof (t))
#define FCLAW2D_FREE(p)         fclaw2d_free (p)

/***************************** PATCH ITERATORS ****************************/

/** Callback prototype for the patch iterators.
 * \param [in] domain	General domain structure.
 * \param [in] patch	The patch currently processed by the iterator.
 * \param [in] blockno  Block number of processed patch.
 * \param [in] patchno  Patch number within block of processed patch.
 * \param [in,out] user	Data that was passed into the iterator functions.
 */
typedef void (*fclaw2d_patch_callback_t)
    (fclaw2d_domain_t * domain, fclaw2d_patch_t * patch,
     int blockno, int patchno, void *user);

/** Iterate over all patches on a given level.
 * \param [in] domain	General domain structure.
 * \param [in] level	Level to iterate.  Ignore patches of other levels.
 * \param [in] pcb	Function called for each patch of matching level.
 * \param [in,out] user	Data is passed to the pcb callback.
 */
void fclaw2d_domain_iterate_level (fclaw2d_domain_t * domain, int level,
                                   fclaw2d_patch_callback_t pcb, void *user);

/** Iterate over all patches of all levels.
 * \param [in] domain	General domain structure.
 * \param [in] pcb	Function called for each patch in the domain.
 * \param [in,out] user	Data is passed to the pcb callback.
 */
void fclaw2d_domain_iterate_patches (fclaw2d_domain_t * domain,
                                     fclaw2d_patch_callback_t pcb,
                                     void *user);

/** Iterate over all families of sibling patches.
 * \param [in] domain	General domain structure.
 * \param [in] pcb	Function called for each family in the domain.
 *                      Its patch argument points to an array of four
 *                      valid patches that constitute a family of siblings.
 *                      Their patchnos are consecutive, blockno is the same.
 * \param [in,out] user	Data is passed to the pcb callback.
 */
void fclaw2d_domain_iterate_families (fclaw2d_domain_t * domain,
                                      fclaw2d_patch_callback_t pcb,
                                      void *user);

/************************ PATCH NEIGHBORS *********************************/

/** Determine physical boundary status as 1, or 0 for neighbor patches.
 * \param [in] domain	Valid domain structure.
 * \param [in] blockno	Number of the block within the domain.
 * \param [in] patchno	Number of the patch within the block.
 * \param [in,out] boundaries	Domain boundary boolean flags.
 *			The order is left, right, bottom, top.
 * \return		True if at least one patch face is on a boundary.
 */
int fclaw2d_patch_boundary_type (fclaw2d_domain_t * domain,
                                 int blockno, int patchno, int boundaries[4]);

typedef enum fclaw2d_face_neighbor
{
    FCLAW2D_PATCH_BOUNDARY,
    FCLAW2D_PATCH_HALFSIZE,
    FCLAW2D_PATCH_SAMESIZE,
    FCLAW2D_PATCH_DOUBLESIZE
}
fclaw2d_patch_relation_t;

/** Determine neighbor patch(es) and orientation across a given face.
 * \param [in] domain   Valid domain structure.
 * \param [in] blockno  Number of the block within the domain.
 * \param [in] patchno  Number of the patch within the block.
 * \param [in] faceno   Number of the patch face: left, right, bottom, top.
 * \param [out] rproc   Processor number of neighbor patches.  Exception:
 *                      If the neighbor is a bigger patch, rproc[1] contains
 *                      the number of the small patch as one of two half faces.
 * \param [out] rblockno        Neighbor block number.
 * \param [out] rpatchno        Neighbor patch numbers for up to 2 neighbors.
 *                              The patch number is relative to its block.
 *                              If the neighbor is off-processor, this is not
 *                              a patch number but in [0, num_ghost_patches[.
 * \param [out] rfaceno Neighbor face number and orientation.
 * \return              The relative patch size of the face neighbor.
 */
fclaw2d_patch_relation_t fclaw2d_patch_face_neighbors (fclaw2d_domain_t *
                                                       domain, int blockno,
                                                       int patchno,
                                                       int faceno,
                                                       int rproc[2],
                                                       int *rblockno,
                                                       int rpatchno[2],
                                                       int *rfaceno);

/** Change perspective across a face neighbor situation.
 * \param [in,out] faceno   On input, valid face number for a patch.
 *                          On output, valid face number seen from
 *                          faceno's neighbor patch.
 * \param [in,out] rfaceno  On input, encoded neighbor face number as returned
 *                          by fclaw2d_patch_face_neighbors.
 *                          On output, encoded neighbor face number seen from
 *                          faceno's neighbor patch.
 */
void fclaw2d_patch_face_swap (int *faceno, int *rfaceno);

/** Fill an array with the axis combination of a face neighbor transform.
 * \param [in]  faceno      The number of the originating face.
 * \param [in]  rfaceno     Encoded as rfaceno = r * 4 + nf, where nf = 0..3 is
 *                          the neigbbor's connecting face number and r = 0..1
 *                          is the relative orientation to the neighbor's face.
 * \param [out] ftransform  This array holds 9 integers.
 *              [0,2]       The coordinate axis sequence of the origin face,
 *                          the first referring to the tangential and the second
 *                          to the normal.  A permutation of (0, 1).
 *              [3,5]       The coordinate axis sequence of the target face.
 *              [6,8]       Edge reversal flag for tangential axis (boolean);
 *                          face code in [0, 3] for the normal coordinate q:
 *                          0: q' = -q
 *                          1: q' = q + 1
 *                          2: q' = q - 1
 *                          3: q' = 2 - q
 *              [1,4,7]     0 (unused for compatibility with 3D).
 */
void fclaw2d_patch_face_transformation (int faceno, int rfaceno,
                                        int ftransform[]);

/** Transform a patch coordinate into a neighbor patch's coordinate system.
 * This function assumes that the two patches are of the SAME size.
 * The neighbor patch may be in the same block, encoded by ftransform[8] == 4.
 * Else we have an input patch in one block and on output patch across a face.
 * \param [in] ipatch       The patch that the input coordinates are relative to.
 * \param [in] opatch       The patch that the output coordinates are relative to.
 * \param [in] ftransform   It must have room for NINE (9) integers and be
 *                          computed by \a fclaw2d_patch_face_transformation.
 *                          If \a ipatch and \a opatch are in the same block,
 *                          we require \a ftransform[8] |= 4.
 * \param [in] mx           Number of cells along x direction of patch.
 * \param [in] my           Number of cells along y direction of patch.
 *                          This function assumes \a mx == \a my.
 * \param [in] based        Indices are 0-based for corners and 1-based for cells.
 * \param [in,out] i        Integer coordinate along x-axis in \a based .. \a mx.
 * \param [in,out] j        Integer coordinate along y-axis in \a based .. \a my.
 */
void fclaw2d_patch_transform_face (fclaw2d_patch_t * ipatch,
                                   fclaw2d_patch_t * opatch,
                                   const int ftransform[],
                                   int mx, int my, int based, int *i, int *j);

/** Transform a patch coordinate into a neighbor patch's coordinate system.
 * This function assumes that the neighbor patch is smaller (HALF size).
 * The neighbor patch may be in the same block, encoded by ftransform[8] == 4.
 * Else we have an input patch in one block and on output patch across a face.
 * \param [in] ipatch       The patch that the input coordinates are relative to.
 * \param [in] opatch       The patch that the output coordinates are relative to.
 * \param [in] ftransform   It must have room for NINE (9) integers and be
 *                          computed by \a fclaw2d_patch_face_transformation.
 *                          If \a ipatch and \a opatch are in the same block,
 *                          we require \a ftransform[8] |= 4.
 * \param [in] position     0 or 1 depending on the position of small neighbor.
 * \param [in] mx           Number of cells along x direction of patch.
 * \param [in] my           Number of cells along y direction of patch.
 *                          This function assumes \a mx == \a my.
 * \param [in] based        Indices are 0-based for corners and 1-based for cells.
 * \param [in,out] i        FOUR (4) integer coordinates along x-axis in
 *                          \a based .. \a mx.  On input, only the first is used.
 *                          On output, they are relative to the fine patch and
 *                          stored in order of the children of the coarse patch.
 * \param [in,out] j        FOUR (4) integer coordinates along y-axis in
 *                          \a based .. \a mx.  On input, only the first is used.
 *                          On output, they are relative to the fine patch and
 *                          stored in order of the children of the coarse patch.
 */
void fclaw2d_patch_transform_face2 (fclaw2d_patch_t * ipatch,
                                    fclaw2d_patch_t * opatch,
                                    const int ftransform[], int position,
                                    int mx, int my, int based, int i[],
                                    int j[]);

/** Determine neighbor patch(es) and orientation across a given corner.
 * The current version only supports one neighbor, i.e., no true multi-block.
 * A query across a corner in the middle of a longer face returns the boundary.
 * \param [in] domain   Valid domain structure.
 * \param [in] blockno  Number of the block within the domain.
 * \param [in] patchno  Number of the patch within the block.
 * \param [in] cornerno	Number of the patch corner: 0=bl, 1=br, 2=tl, 3=tr.
 * \param [out] rproc   Processor number of neighbor patch.
 * \param [out] rblockno        Neighbor block number.
 * \param [out] rpatchno        Neighbor patch number relative to the block.
 *                              If the neighbor is off-processor, this is not
 *                              a patch number but in [0, num_ghosts_patches[.
 * \param [out] neighbor_size   The relative patch size of the neighbor.
 * \return                      True if at least one corner neighbor exists
 *                              that is not already a face neighbor.
 */
int fclaw2d_patch_corner_neighbors (fclaw2d_domain_t * domain,
                                    int blockno, int patchno, int cornerno,
                                    int *rproc, int *rblockno, int *rpatchno,
                                    fclaw2d_patch_relation_t * neighbor_size);

/** Transform a patch coordinate into a neighbor patch's coordinate system.
 * This function assumes that the two patches are of the SAME size.
 * This function assumes that the two patches are in the SAME block.
 * \param [in] ipatch       The patch that the input coordinates are relative to.
 * \param [in] opatch       The patch that the output coordinates are relative to.
 * \param [in] icorner      Corner number of this patch to transform across.
 * \param [in] mx           Number of cells along x direction of patch.
 * \param [in] my           Number of cells along y direction of patch.
 *                          This function assumes \a mx == \a my.
 * \param [in] based        Indices are 0-based for corners and 1-based for cells.
 * \param [in,out] i        Integer coordinate along x-axis in \a based .. \a mx.
 * \param [in,out] j        Integer coordinate along y-axis in \a based .. \a my.
 */
void fclaw2d_patch_transform_corner (fclaw2d_patch_t * ipatch,
                                    fclaw2d_patch_t * opatch,
                                    int icorner,
                                    int mx, int my, int based, int *i, int *j);

/** Transform a patch coordinate into a neighbor patch's coordinate system.
 * This function assumes that the neighbor patch is smaller (HALF size).
 * This function assumes that the two patches are in the SAME block.
 * \param [in] ipatch       The patch that the input coordinates are relative to.
 * \param [in] opatch       The patch that the output coordinates are relative to.
 * \param [in] icorner      Corner number of this patch to transform across.
 * \param [in] mx           Number of cells along x direction of patch.
 * \param [in] my           Number of cells along y direction of patch.
 *                          This function assumes \a mx == \a my.
 * \param [in] based        Indices are 0-based for corners and 1-based for cells.
 * \param [in,out] i        FOUR (4) integer coordinates along x-axis in
 *                          \a based .. \a mx.  On input, only the first is used.
 *                          On output, they are relative to the fine patch and
 *                          stored in order of the children of the coarse patch.
 * \param [in,out] j        FOUR (4) integer coordinates along y-axis in
 *                          \a based .. \a mx.  On input, only the first is used.
 *                          On output, they are relative to the fine patch and
 *                          stored in order of the children of the coarse patch.
 */
void fclaw2d_patch_transform_corner2 (fclaw2d_patch_t * ipatch,
                                      fclaw2d_patch_t * opatch,
                                      int icorner,
                                      int mx, int my, int based,
                                      int i[], int j[]);

/************************** ADAPT *****************************************/

/** Mark a patch for refinement.
 * It is safe to call this function from an iterator callback except
 * fclaw2d_domain_iterate_adapted.
 */
void fclaw2d_patch_mark_refine (fclaw2d_domain_t * domain,
                                int blockno, int patchno);

/** Mark a patch for coarsening.
 * Coarsening will only happen if the patch family is not further refined
 * and all sibling patches are marked as well.
 * It is safe to call this function from an iterator callback except
 * fclaw2d_domain_iterate_adapted.
 */
void fclaw2d_patch_mark_coarsen (fclaw2d_domain_t * domain,
                                 int blockno, int patchno);

/** Callback prototype used in fclaw2d_domain_iterate_adapted.
 * The newsize value informs on refine/coarsen/noop status.
 * If refined (new patch is HALFSIZE), the old patch is old_patch[0] and the
 * new patches are given by new_patch[0] through new_patch[3]. The new_patchno
 * numbers are consecutive as well.
 * If noop (new patch is SAMESIZE), only old_patch[0] and new_patch[0] matter.
 * If coarsened (new patch is DOUBLESIZE), situation is the reverse of refine.
 */
typedef void (*fclaw2d_match_callback_t) (fclaw2d_domain_t * old_domain,
                                          fclaw2d_patch_t * old_patch,
                                          fclaw2d_domain_t * new_domain,
                                          fclaw2d_patch_t * new_patch,
                                          fclaw2d_patch_relation_t newsize,
                                          int blockno,
                                          int old_patchno, int new_patchno,
                                          void *user);

/** Iterate over the previous and the adapted domain simultaneously.
 * \param [in,out] old_domain   Domain before adaptation.
 * \param [in,out] new_domain   Domain after adaptation.
 * \param [in] mcb              Callback
 */
void fclaw2d_domain_iterate_adapted (fclaw2d_domain_t * old_domain,
                                     fclaw2d_domain_t * new_domain,
                                     fclaw2d_match_callback_t mcb,
                                     void *user);

/*************************** PARTITION ************************************/

/** Allocate data buffer for parallel transfer of all patches.
 * \param [in,out] domain       The memory lives inside this domain.
 * \param [in] data_size        Number of bytes per patch to transfer.
 * \param [in,out] patch_data   Address of an array of void pointers.
 *                              Data is allocated by this function.  After the
 *                              call, *patch_data holds one pointer per patch
 *                              that points to exactly data_size bytes of
 *                              memory that can be written to by forestclaw.
 *                              *patch_data must be NULL before the call.
 */
void fclaw2d_domain_allocate_before_partition (fclaw2d_domain_t * domain,
                                               size_t data_size,
                                               void ***patch_data);

/** Reallocate data buffer to reflect patch data after partition.
 * \param [in,out] domain       The memory lives inside this domain.
 * \param [in,out] patch_data   Address of an array of void pointers.
 *                              Data is reallocated by this function.  After the
 *                              call, *patch_data holds one pointer per patch
 *                              that points to exactly data_size bytes of
 *                              memory that can be read from by forestclaw.
 */
void fclaw2d_domain_retrieve_after_partition (fclaw2d_domain_t * domain,
                                              void ***patch_data);

/** Free buffers that were used in transfering data during partition.
 * \param [in,out] domain       The memory lives inside this domain.
 * \param [in,out] patch_data   Address of an array of void pointers to free.
 *                              *patch_data will be NULL after the call.
 */
void fclaw2d_domain_free_after_partition (fclaw2d_domain_t * domain,
                                          void ***patch_data);

/**************************** EXCHANGE ************************************/

/** Data structure for storing allocated data for parallel exchange. */
typedef struct fclaw2d_domain_exchange
{
    size_t data_size;
    /* These two members are for consistency checking */
    int num_exchange_patches;
    int num_ghost_patches;
    /*
       One pointer per processor-local exchange patch in order, for a total
       count of domain->num_exchange_patches.  This applies precisely to local
       patches that touch the parallel boundary from the inside, i.e., if
       (flags & FCLAW2D_PATCH_ON_PARALLEL_BOUNDARY).
     */
    void **patch_data;
    /*
       Array of domain->num_ghost_patches many void pointers, each pointing to
       exactly data_size bytes of memory that can be read from by forestclaw
       after each fclaw2d_domain_parallel_exchange.
     */
    void **ghost_data;
    /*
       Memory where the ghost patch data actually lives.
       The above array ghost_data points into this memory.
       It will not be necessary to dereference this memory explicitly.
     */
    char *ghost_contiguous_memory;
}
fclaw2d_domain_exchange_t;

/** Allocate buffer to hold the data from off-processor patches.
 * Free this by fclaw2d_domain_free_after_exchange before regridding.
 * \param [in] domain           The domain is not modified.
 * \param [in] data_size        Number of bytes per patch to exchange.
 * \return                      Allocated data structure.
 *                              The pointers in patch_data[i] need to be set
 *                              after this call by forestclaw.
 */
fclaw2d_domain_exchange_t
    * fclaw2d_domain_allocate_before_exchange (fclaw2d_domain_t * domain,
                                               size_t data_size);

/** Exchange data for parallel ghost neighbors.
 * This function receives data from parallel neighbor (ghost) patches.
 * It can be called multiple times on the same allocated buffers.
 * We assume that the data size for all patches is the same.
 * \param [in] domain           Used to access forest and ghost metadata.
 *                              #(sent patches) is domain->num_exchange_patches.
 *                              #(received patches) is domain->num_ghost_patches.
 * \param [in] e                Allocated buffers whose e->patch_data[i] pointers
 *                              must have been set properly by forestclaw.
 * \param [in] exchange_minlevel The minimum quadrant level that is exchanged.
 * \param [in] exchange_maxlevel The maximum quadrant level that is exchanged.
 */
void fclaw2d_domain_ghost_exchange (fclaw2d_domain_t * domain,
                                    fclaw2d_domain_exchange_t * e,
                                    int exchange_minlevel,
                                    int exchange_maxlevel);

/** Free buffers used in exchanging off-processor data during time stepping.
 * This should be done just before regridding.
 * \param [in] domain           The domain is not modified.
 * \param [in] e                Allocated buffers.
 */
void fclaw2d_domain_free_after_exchange (fclaw2d_domain_t * domain,
                                         fclaw2d_domain_exchange_t * e);

/************************ COMMUNICATION ***********************************/

/** Compute and return the maximum over all processors of a double value.
 * The minimum can be computed by using this function on the negative value.
 */
double fclaw2d_domain_global_maximum (fclaw2d_domain_t * domain, double d);

/** Compute and return the sum over all processors of a double value.
 */
double fclaw2d_domain_global_sum (fclaw2d_domain_t * domain, double d);

/** Synchronize all processes.  Avoid using if at all possible.
 */
void fclaw2d_domain_barrier (fclaw2d_domain_t * domain);

/** Serialize a section of code.
 * THIS IS NOT SCALABLE.
 * WILL BE HORRIBLY SLOW FOR LARGE NUMBERS OF PROCESSORS.
 * A processor returns from this function only after all lower-numbered
 * processors have called fclaw2d_domain_serialization_leave.
 * No collective communication routines must be called between the calls
 * to this function and fclaw2d_domain_serialization_leave.
 * \param [in] domain           The domain is not modified.
 */
void fclaw2d_domain_serialization_enter (fclaw2d_domain_t * domain);

/** Serialize a section of code.
 * THIS IS NOT SCALABLE.
 * WILL BE HORRIBLY SLOW FOR LARGE NUMBERS OF PROCESSORS.
 * A processor must call this function to allow all higher-numbered
 * processors to return from fclaw2d_domain_serialization_enter.
 * \param [in] domain           The domain is not modified.
 */
void fclaw2d_domain_serialization_leave (fclaw2d_domain_t * domain);

#ifdef __cplusplus
#if 0
{                               /* need this because indent is dumb */
#endif
}
#endif

#endif /* !FORESTCLAW2D_H */

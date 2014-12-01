/* Cartesian grid, tranformed to Ax + b */

#include <fclaw2d_map.h>
#include <p4est_connectivity.h>
#include <fclaw2d_convenience.h>

#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

/* This is used for communicating with Matlab about the mapping */
void write_brick_data_(int* n,
                       int* mi,
                       int* mj,
                       double xv[],
                       double yv[]);

typedef struct fclaw2d_block_ll
{
    int nb;
    int mi, mj;
    double *xv;
    double *yv;
} fclaw2d_block_ll_t;

static int
fclaw2d_map_query_brick (fclaw2d_map_context_t * cont, int query_identifier)
{
    switch (query_identifier)
    {
    case FCLAW2D_MAP_QUERY_IS_USED:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_SCALEDSHIFT:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_AFFINE:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_NONLINEAR:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_GRAPH:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PLANAR:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_ALIGNED:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_FLAT:
        return 1;
    case FCLAW2D_MAP_QUERY_IS_DISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_SPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PILLOWDISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_SQUAREDDISK:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_PILLOWSPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_CUBEDSPHERE:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_FIVEPATCH:
        return 0;
    case FCLAW2D_MAP_QUERY_IS_CART:
        return 1;
    default:
        printf("\n");
        printf("fclaw2d_map_query_brick (fclaw2d_map_brick.h) : "\
               "Query id not identified;  Maybe the query is not up to "\
               "date?\nSee fclaw2d_map_brick.c.\n");
        printf("Requested query id : %d\n",query_identifier);
        SC_ABORT_NOT_REACHED ();
    }
    return 0;
}


static void
fclaw2d_map_c2m_brick(fclaw2d_map_context_t * cont, int blockno,
                     double xc, double yc,
                     double *xp, double *yp, double *zp)
{
    /* Map the brick coordinates to global [0,1] coordinates. */
    fclaw2d_block_ll_t *bv = (fclaw2d_block_ll_t *) cont->user_data;
    *xp = (double) (bv->xv[blockno] + xc)/bv->mi;
    *yp = (double) (bv->yv[blockno] + yc)/bv->mj;
    *zp = 0;
}

void fclaw2d_map_destroy_brick(fclaw2d_map_context_t *cont)
{
    fclaw2d_block_ll_t *bv = (fclaw2d_block_ll_t *) cont->user_data;
    FCLAW_FREE(bv->xv);
    FCLAW_FREE(bv->yv);

    FCLAW_FREE (cont->user_data);
    FCLAW_FREE (cont);
}


fclaw2d_map_context_t* fclaw2d_map_new_brick(p4est_connectivity_t *conn,
                                             int mi,
                                             int mj)
{
    fclaw2d_map_context_t *cont;
    fclaw2d_block_ll_t *bv;
    int i,nb,vnum;

    cont = FCLAW_ALLOC_ZERO (fclaw2d_map_context_t, 1);
    cont->query = fclaw2d_map_query_brick;
    cont->mapc2m = fclaw2d_map_c2m_brick;
    cont->destroy = fclaw2d_map_destroy_brick;

    nb = (int) conn->num_trees;
    bv = FCLAW_ALLOC_ZERO(fclaw2d_block_ll_t,1);

    /* We don't store this in user_double[], since that only has limited
       storage (16 doubles) */
    bv->xv = FCLAW_ALLOC_ZERO(double,nb);
    bv->yv = FCLAW_ALLOC_ZERO(double,nb);

    bv->nb = nb;  /* These integer values could also be stored in user_int[] data */
    bv->mi = mi;
    bv->mj = mj;

    for (i = 0; i < nb; i++)
    {
        vnum = conn->tree_to_vertex[4 * i];
        /* (x,y) coordinates of lower-left corner */
        bv->xv[i] = conn->vertices[3*vnum];
        bv->yv[i] = conn->vertices[3*vnum+1];
    }
    cont->user_data = (void*) bv;

    /* Write data for Matlab plotting */
    write_brick_data_(&nb,&mi,&mj,bv->xv,bv->yv);
    return cont;
}

#ifdef __cplusplus
#if 0
{
#endif
}
#endif

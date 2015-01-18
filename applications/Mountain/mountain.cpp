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

#include <amr_single_step.h>
#include <fclaw2d_clawpack.H>
#include <fclaw2d_map.h>
#include <p4est_connectivity.h>

#include <amr_forestclaw.H>
#include <amr_utils.H>
#include <fclaw2d_map_query.h>



#include "mountain_user.H"

int
main (int argc, char **argv)
{
  int		        lp;
  int                   example;
  sc_MPI_Comm           mpicomm;
  sc_options_t          *options;
  p4est_connectivity_t  *conn = NULL;
  fclaw2d_map_context_t *cont = NULL, *brick=NULL;
  fclaw2d_domain_t	*domain;
  amr_options_t         samr_options, *gparms = &samr_options;
  fclaw2d_clawpack_parms_t  *clawpack_parms;

  int mi,mj,a,b;
  double rotate[2];
  double shift[3];
  double scale[3];

  lp = SC_LP_PRODUCTION;
  mpicomm = sc_MPI_COMM_WORLD;
  fclaw_mpi_init (&argc, &argv, mpicomm, lp);

#ifdef MPI_DEBUG
  /* this has to go after MPI has been initialized */
  fclaw2d_mpi_debug();
#endif



  /* ---------------------------------------------------------------
     Read parameters from .ini file, parse command line, and
     do parameter checking.
     -------------------------------------------------------------- */
  options = sc_options_new(argv[0]);

  sc_options_add_int (options, 0, "example", &example, 0,
                      "1 for identity mapping [0,1]x[0,1]" \
                      "2 for Cartesian brick mapping" \
                      "3 for five patch square");

  sc_options_add_int (options, 0, "mi", &mi, 5,
                         "mi : Number of bricks in the x direction [5]");

  sc_options_add_int (options, 0, "mj", &mj, 2,
                         "mj : Number of bricks in the y direction [2]");

  /* Register default parameters and any solver parameters */
  gparms = amr_options_new(options);
  clawpack_parms = fclaw2d_clawpack_parms_new(options);

  /* Parse any command line arguments.  Argument gparms is no longer needed
     as an input since the array conversion is now done in 'postprocess' */

  amr_options_parse(options,argc,argv,lp);

  /* Any arrays are converted here */
  amr_postprocess_parms(gparms);
  fclaw2d_clawpack_postprocess_parms(clawpack_parms);

  /* Check final state of parameters */
  amr_checkparms(gparms);
  fclaw2d_clawpack_checkparms(clawpack_parms,gparms);

  /* ---------------------------------------------------------------
     Floating point traps
     -------------------------------------------------------------- */
  if (gparms->trapfpe == 1)
  {
      printf("Enabling floating point traps\n");
      feenableexcept(FE_INVALID);
  }

  /* ---------------------------------------------------------------
     Domain geometry
     -------------------------------------------------------------- */

  scale[0] = 5000;
  scale[1] = 2000;
  scale[2] = 1;

  shift[0] = 0;
  shift[1] = 0;
  shift[2]  = 0;

  rotate[0] = 0;
  rotate[1] = 0;

  a = 0;   /* periodic in x */
  b = 0;   /* periodic in y */

  conn = p4est_connectivity_new_brick(mi,mj,a,b);
  brick = fclaw2d_map_new_brick(conn,mi,mj);

  switch (example) {
  case 1:
      /* A cut cell mesh */
      cont = fclaw2d_map_new_identity (brick,scale,shift,rotate);
      break;
  case 2:
      /* A terrain following grid */
      cont = fclaw2d_map_new_mountain (brick,scale,shift,rotate);
      break;
  default:
      sc_abort_collective ("Parameter example must be 1");
  }

  domain = fclaw2d_domain_new_conn_map (mpicomm, gparms->minlevel, conn, cont);

  /* ---------------------------------------------------------- */
  if (gparms->verbosity > 0)
  {
      fclaw2d_domain_list_levels(domain, lp);
      fclaw2d_domain_list_neighbors(domain, lp);
  }

  /* ---------------------------------------------------------------
     Set domain data.
     --------------------------------------------------------------- */
  init_domain_data(domain);

  set_domain_parms(domain,gparms);
  set_clawpack_parms(domain,clawpack_parms);

  /* ---------------------------------------------------------------
     Define the solver and link in other problem/user specific
     routines
     --------------------------------------------------------------- */

  /* Using user defined functions just to demonstrate how one might setup
     something that depends on more than one solver (although only one is used
     here) */
  link_problem_setup(domain,mountain_problem_setup);

  mountain_link_solvers(domain);

  link_regrid_functions(domain,mountain_patch_tag4refinement,mountain_patch_tag4coarsening);

  /* ---------------------------------------------------------------
     Run
     --------------------------------------------------------------- */
  amrinit(&domain);
  amrrun(&domain);
  amrreset(&domain);

  fclaw2d_map_destroy(cont);    /* This destroys the brick as well */
  sc_options_destroy(options);         /* this could be moved up */
  amr_options_destroy(gparms);
  fclaw2d_clawpack_parms_delete(clawpack_parms);

  fclaw_mpi_finalize ();

  return 0;
}

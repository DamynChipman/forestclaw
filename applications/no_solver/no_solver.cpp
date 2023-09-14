/*
Copyright (c) 2012-2023 Carsten Burstedde, Donna Calhoun, Scott Aiton
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

#include "no_solver_user.h"

#include <fclaw_include_all.h>

#include <fclaw_clawpatch_options.h>
#include <fclaw_clawpatch.h>

static
void create_domain_map (fclaw_global_t *glob,
                        fclaw_options_t* fclaw_opt)
{
    fclaw_domain_t         *domain = NULL;

    domain = fclaw_domain_new_unitsquare (mpicomm, fclaw_opt->minlevel);
    fclaw_domain_list_levels (domain, FCLAW_VERBOSITY_ESSENTIAL);
    fclaw_domain_list_neighbors (domain, FCLAW_VERBOSITY_DEBUG);
    fclaw_global_store_domain (glob, domain);
    fclaw2d_map_store (glob, fclaw2d_map_new_nomap ());
}

static
void run_program(fclaw_global_t* glob)
{
    /* ---------------------------------------------------------------
       Set domain data.
       --------------------------------------------------------------- */
    const user_options_t *user = no_solver_get_options(glob);

    
    /* Initialize virtual table for ForestClaw */
    fclaw_vtable_initialize(glob);
    fclaw_diagnostics_vtable_initialize();

    fclaw_clawpatch_vtable_initialize(user->clawpatch_version);

    no_solver_link_solvers(glob);

    /* ---------------------------------------------------------------
       Run
       --------------------------------------------------------------- */
    fclaw_initialize(glob);
    fclaw_run(glob);
    fclaw_finalize(glob);
}

int
main (int argc, char **argv)
{
    fclaw_app_t *app;
    int first_arg;
    fclaw_exit_type_t vexit;

    /* Options */
    user_options_t              *user_opt;
    fclaw_options_t             *fclaw_opt;
    fclaw_clawpatch_options_t *clawpatch_opt;

    /* Initialize application */
    app = fclaw_app_new (&argc, &argv, NULL);

    /* Create new options packages */
    fclaw_opt =                   fclaw_options_register(app,  NULL,       "fclaw_options.ini");
    clawpatch_opt =   fclaw_clawpatch_2d_options_register(app, "clawpatch", "fclaw_options.ini");
    user_opt =                no_solver_options_register(app,              "fclaw_options.ini");

    /* Read configuration file(s) and command line, and process options */
    vexit =  fclaw_app_options_parse (app, &first_arg,"fclaw_options.ini.used");

    /* Run the program */
    if (!vexit)
    {
        /* Create global structure which stores the domain, timers, etc */
        int size, rank;
        sc_MPI_Comm mpicomm = fclaw_app_get_mpi_size_rank (app, &size, &rank);
        fclaw_global_t *glob = fclaw_global_new_comm (glob->mpicomm, size, rank);
        create_domain_map (glob, fclaw_opt);

        /* Store option packages in glob */
        fclaw_options_store           (glob, fclaw_opt);
        fclaw_clawpatch_options_store (glob, clawpatch_opt);
        no_solver_options_store         (glob, user_opt);

        run_program(glob);

        fclaw_global_destroy(glob);
    }

    fclaw_app_destroy (app);

    return 0;
}

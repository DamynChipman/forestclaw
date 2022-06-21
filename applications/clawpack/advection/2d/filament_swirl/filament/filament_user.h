/*
Copyright (c) 2012-2021 Carsten Burstedde, Donna Calhoun
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

#ifndef FILAMENT_USER_H
#define FILAMENT_USER_H

#include "../../all/advection_user.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 0
/* Fix syntax highlighting */
#endif

typedef struct filament_options
{
    int example;
    double alpha;
    int claw_version;

    int is_registered;

} filament_options_t;


filament_options_t* filament_options_register (fclaw_app_t * app,
                                               const char *section,
                                               const char *configfile);

void filament_options_store (fclaw2d_global_t* glob, filament_options_t* user);

const filament_options_t* filament_get_options(fclaw2d_global_t* glob);

void filament_link_solvers(fclaw2d_global_t *glob);

fclaw2d_domain_t* filament_create_domain(sc_MPI_Comm mpicomm, 
                                         fclaw_options_t* fclaw_opt, 
                                         filament_options_t* user,
                                         fclaw2d_clawpatch_options_t* clawpatch_opt);

void filament_run_program(fclaw2d_global_t* glob);

#ifdef __cplusplus
}
#endif

#endif

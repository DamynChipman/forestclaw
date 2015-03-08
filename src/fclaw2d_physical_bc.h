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

#ifndef FCLAW2D_PHYSICAL_BC_H
#define FCLAW2D_PHYSICAL_BC_H

#include "fclaw2d_defs.H"

#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

/* This is needed by other routines, so we don't set it to static. */
void fclaw2d_get_physical_bc(fclaw2d_domain_t *domain,
                             int this_block_idx,
                             int this_patch_idx,
                             fclaw_bool *intersects_bdry);

void fclaw2d_set_physical_bc(fclaw2d_domain_t *domain, int a_level,
                             double a_level_time, fclaw_bool time_interp);

void fclaw2d_physical_bc_default(fclaw2d_domain *domain,
                                 fclaw2d_patch_t *this_patch,
                                 int this_block_idx,
                                 int this_patch_idx,
                                 double t,
                                 double dt,
                                 fclaw_bool intersects_phys_bdry[],
                                 fclaw_bool time_interp);



#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif

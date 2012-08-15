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

#include "amr_utils.H"
#include "forestclaw2d.h"


global_parms::global_parms()
{
    m_maxmwaves = 20;
    m_mthlim = new int[m_maxmwaves];
    m_mthbc = new int[2*SpaceDim];
    m_refratio = 2;  // hardwired for now.
}

global_parms::~global_parms()
{
    delete [] m_mthbc;
    delete [] m_mthlim;
}


void global_parms::get_inputParams()
{
    // read data using Fortran file
    inputparms_(m_mx_leaf,
                m_my_leaf,
                m_initial_dt,
                m_tfinal,
                m_max_cfl,
                m_desired_cfl,
                m_nout,
                m_src_term,
                m_verbose,
                m_mcapa,
                m_maux,
                m_meqn,
                m_mwaves,
                m_maxmwaves,
                m_mthlim,
                m_mbc,
                m_mthbc,
                m_order,
                m_minlevel,
                m_maxlevel);

    // Check maxlevel :
    if (m_maxlevel > fclaw2d_possible_maxlevel)
    {
        cout
	  << "get_inputParms (amr_utils.f) : User 'maxlevel' > p4est 'maxlevel'"
          << endl;
        exit(1);
    }

    // Set up arrays needed by clawpack.
    m_method[0] = 0; // not used in forestclaw

    m_method[1] = m_order[0];
    if (SpaceDim == 2)
    {
        m_method[2] = m_order[1];
    }
    else
    {
        m_method[2] = 10*m_order[1] + m_order[2];
    }
    m_method[3] = m_verbose;
    m_method[4] = m_src_term;
    m_method[5] = m_mcapa;
    m_method[6] = m_maux;
}


void global_parms::print_inputParams()
{
  cout << endl;
  cout << "CLAWPACK PARAMETERS : " << endl;
  cout << "Initial dt " << m_initial_dt << endl;
  cout << "maximum cfl " << m_max_cfl << endl;
  cout << "desired cfl " << m_desired_cfl << endl;
  cout << "method(2:3) (order of integration) = " << m_method[1] << " " << m_method[2] << endl;
  cout << "method(4) (verbosity) = " << m_method[3] << endl;
  cout << "method(5) (source term splitting) = " << m_method[4] << endl;
  cout << "method(6) (mcapa) = " << m_method[5] << endl;
  cout << "method(7) (maux) = " << m_method[6] << endl;
  cout << endl;

  cout << "refratio (fixed) = " << m_refratio << endl;
  cout << endl;

  cout << "meqn (number of equations) = " << m_meqn << endl;
  cout << "maux (number of auxiliary variables) = " << m_maux << endl;
  cout << "mcapa (location of capacity function) = " << m_mcapa << endl;
  cout << "mwaves (number of waves) = " << m_mwaves << endl;

  cout << "mthlim(mwaves) (limiters) = ";
  for(int i = 0; i < m_mwaves; i++)
    {
      cout << m_mthlim[i] << " ";
    }
  cout << endl << endl;

  cout << "mbc (number of ghost cells) = " << m_mbc << endl;

  /*
  cout << "Auxiliary array type : " << endl;
  for (int i = 0; i < m_maux; i++)
    {
      cout << "  " << m_auxtype[i] << endl;
    }
  cout << endl;
  */


  cout << "mthbc(2*dim) (boundary conditions) = ";
  for(int i = 0; i < 2*SpaceDim; i++)
    {
      cout << m_mthbc[i] << " ";
    }
  cout << endl << endl;

  cout << "Min level = " << m_minlevel << endl;
  cout << "Max level = " << m_maxlevel << endl;

}


FArrayBox::FArrayBox()
{
    m_data = NULL;
}

FArrayBox::~FArrayBox()
{
    if (m_data != NULL) {delete [] m_data; m_data = NULL;}
}

void FArrayBox::define(int a_size,const Box& a_box)
{
    m_data = new double[a_size];
    m_box = a_box;
}

Real* FArrayBox::dataPtr() const
{
    return m_data;
}

Box FArrayBox::box()
{
    return m_box;
}

Box::Box()
{
}

Box::Box(const Box& a_box)
{
    for(int idir = 0; idir < 2; idir++)
    {
        m_ll[idir] = a_box.m_ll[idir];
        m_ur[idir] = a_box.m_ur[idir];
    }
}

Box::Box(const int ll[], const int ur[])
{
    for(int idir = 0; idir < 2; idir++)
    {
        m_ll[idir] = ll[idir];
        m_ur[idir] = ur[idir];
    }
}

int Box::smallEnd(int idir) const
{
    return m_ll[idir];
}

int Box::bigEnd(int idir) const
{
    return m_ur[idir];
}

subcycle_manager::subcycle_manager() {}
subcycle_manager::~subcycle_manager() {}

void subcycle_manager::define(fclaw2d_domain_t *domain,
                              const Real& a_t_curr)
{
    global_parms *gparms = get_domain_data(domain);
    m_t_minlevel = a_t_curr;
    m_refratio = gparms->m_refratio;

    // Finest level we can ever have.  For any particular run, however
    // the finest level may not be this fine.  This is stored in this->m_maxlevel,
    // computed below.
    int maxlevel_fixed = gparms->m_maxlevel;

    // Too many elements if m_minlevel > 0, but we want to be
    // able to index using full range of indices.
    m_levels.resize(maxlevel_fixed + 1);

    m_minlevel = 0;
    for(int level = 0; level <= maxlevel_fixed; level++)
    {
        int np = num_patches(domain,level);
        if (level == m_minlevel && np == 0)
            m_minlevel++;
    }

    m_maxlevel = maxlevel_fixed;
    for (int level = maxlevel_fixed; level >= 0; level--)
    {
        int np = num_patches(domain,level);
        if (level == m_maxlevel && np == 0)
            m_maxlevel--;
    }

    for (int level = m_minlevel; level <= m_maxlevel; level++)
    {
        int np = num_patches(domain,level);
        m_levels[level].define(level,m_refratio,np);
    }
}

void subcycle_manager::set_dt_minlevel(const Real& a_dt_minlevel)
{
    // Time step for coarsest level that has grids
    m_dt_minlevel = a_dt_minlevel;

    Real dt_level = a_dt_minlevel;
    m_levels[m_minlevel].set_dt_level(dt_level);
    for (int level = m_minlevel+1; level <= m_maxlevel; level++)
    {
        dt_level /= m_refratio;
        m_levels[level].set_dt_level(dt_level);
    }
}

Real subcycle_manager::reduce_to_minlevel(const Real& a_dt)
{
    Real dt_minlevel = a_dt;
    for (int level = 1; level <= m_minlevel; level++)
    {
        dt_minlevel/= m_refratio;
    }
    return dt_minlevel;
}

int subcycle_manager::reduce_to_minlevel_factor()
{
    int factor = 1;
    for (int level = 1; level <= m_minlevel; level++)
    {
        factor *= m_refratio;
    }
    return factor;
}


int subcycle_manager::get_minlevel()
{
    return m_minlevel;
}

int subcycle_manager::get_maxlevel()
{
    return m_maxlevel;
}

int subcycle_manager::get_last_step(const int& a_level)
{
    return m_levels[a_level].m_last_time_step;
}

void subcycle_manager::increment_time_step(const int& a_level)
{
    m_levels[a_level].m_last_time_step += time_step_inc(a_level);
}

bool subcycle_manager::is_coarsest(const int& a_level)
{
    return a_level == m_minlevel;
}

bool subcycle_manager::is_finest(const int& a_level)
{
    return a_level == m_maxlevel;
}

Real subcycle_manager::get_dt(const int& a_level)
{
    if (a_level >= m_minlevel && a_level <= m_maxlevel)
    {
        return m_levels[a_level].m_dt_level;
    }
    else
    {
        printf("Error (get_dt) : level not defined\n");
        exit(1);
    }

}

bool subcycle_manager::can_advance(const int& a_level, const int& a_from_step)
{
    bool b1 = level_exchange_done(a_level, a_from_step);
    bool b2 = coarse_exchange_done(a_level,a_from_step);
    bool b3 = fine_exchange_done(a_level,a_from_step);
    bool b4 = time_step_done(a_level,a_from_step);
    return b1 && b2 && b3 && b4;
}

Real subcycle_manager::current_time(const int& a_level)
{
    int rfactor = 1;
    for(int level = m_minlevel+1; level <= m_maxlevel-a_level; level++)
    {
        rfactor *= m_refratio;
    }
    Real dt_level = m_levels[a_level].m_dt_level;
    int last_step = m_levels[a_level].m_last_time_step;
    return m_t_minlevel + dt_level*last_step/rfactor;
}

bool subcycle_manager::time_step_done(const int& a_level, const int& a_time_step)
{
    return m_levels[a_level].m_last_time_step >= a_time_step;
}

bool subcycle_manager::level_exchange_done(const int& a_level, const int& a_time_step)
{
    return m_levels[a_level].m_last_level_exchange == a_time_step;
}

bool subcycle_manager::coarse_exchange_done(const int& a_level, const int& a_time_step)
{
    if (is_coarsest(a_level))
        return true;
    else
        return m_levels[a_level].m_last_coarse_exchange == a_time_step;
}

bool subcycle_manager::fine_exchange_done(const int& a_level, const int& a_time_step)
{
    if (is_finest(a_level))
        return true;
    else
        return m_levels[a_level].m_last_fine_exchange == a_time_step;
}

void subcycle_manager::set_level_exchange(const int& a_level, const int& a_time_step)
{
    m_levels[a_level].m_last_level_exchange = a_time_step;
}
void subcycle_manager::set_coarse_exchange(const int& a_level, const int& a_time_step)
{
    if (!is_coarsest(a_level))
        m_levels[a_level].m_last_coarse_exchange = a_time_step;
}

void subcycle_manager::set_fine_exchange(const int& a_level, const int& a_time_step)
{
    if (!is_finest(a_level))
        m_levels[a_level].m_last_fine_exchange = a_time_step;
}

int subcycle_manager::time_step_inc(const int& a_level)
{
    int inc = 1;
    for(int level = 0; level < m_maxlevel-a_level; level++)
    {
        inc *= m_refratio;
    }
    return inc;
}



level_data::level_data() { }
level_data::~level_data() {}

void level_data::define(const int& a_level,
                        const int& a_refratio,
                        const int& a_patches_at_level)
{
    m_level = a_level;
    m_last_time_step = 0;
    m_last_level_exchange = 0;   // Assume that the level exchange has been done at subcycled time
                                 // step 0.
    m_last_coarse_exchange = 0;
    m_last_fine_exchange = 0;

    m_num_patches = a_patches_at_level;
}

void level_data::set_dt_level(const Real& a_dt_level)
{
    m_dt_level = a_dt_level;
}

void cb_num_patches(fclaw2d_domain_t *domain,
	fclaw2d_patch_t *patch, int block_no, int patch_no, void *user)
{
  (*(int *) user)++;
}

int num_patches(fclaw2d_domain_t *domain, int level)
{
  int count = 0;
  fclaw2d_domain_iterate_level(domain, level,
                               (fclaw2d_patch_callback_t) cb_num_patches,
                               &count);
  return count;
}

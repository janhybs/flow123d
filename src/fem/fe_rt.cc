/*!
 *
﻿ * Copyright (C) 2015 Technical University of Liberec.  All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3 as published by the
 * Free Software Foundation. (http://www.gnu.org/licenses/gpl-3.0.en.html)
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * 
 * @file    fe_rt.cc
 * @brief   Definitions of Raviart-Thomas finite elements.
 * @author  Jan Stebel
 */

#include "fem/fe_rt.hh"
#include "fem/fe_values.hh"
#include "mesh/ref_element.hh"
#include "quadrature/quadrature_lib.hh"


RT0_space::RT0_space(unsigned int dim)
    : FunctionSpace(dim, dim)
{}


const double RT0_space::basis_value(unsigned int basis_index,
                                    const arma::vec &point,
                                    unsigned int comp_index) const
{
    OLD_ASSERT(basis_index < this->dim(), "Index of basis function is out of range.");
    OLD_ASSERT(comp_index < this->n_components_, "Index of component is out of range.");

    if (basis_index>0 && comp_index==basis_index-1)
        return point[comp_index]-1;
    else
        return point[comp_index];
}


const arma::vec RT0_space::basis_grad(unsigned int basis_index,
                                      const arma::vec &point,
                                      unsigned int comp_index) const
{
    OLD_ASSERT(basis_index < this->dim(), "Index of basis function is out of range.");
    OLD_ASSERT(comp_index < this->n_components_, "Index of component is out of range.");
  
    arma::vec g(this->space_dim_);
    g.zeros();
    g[comp_index] = 1;

    return g;
}





template<unsigned int dim>
FE_RT0<dim>::FE_RT0()
{
    arma::vec::fixed<dim> sp;

    this->init(false, FEVectorPiola);
    this->function_space_ = make_shared<RT0_space>(dim);
    
    for (unsigned int sid=0; sid<RefElement<dim>::n_sides; ++sid)
    {
        sp.fill(0);
        for (unsigned int i=0; i<RefElement<dim>::n_nodes_per_side; ++i)
            sp += RefElement<dim>::node_coords(RefElement<dim>::interact(Interaction<0,dim-1>(sid))[i]);
        sp /= RefElement<dim>::n_nodes_per_side;
        // barycentric coordinates
        arma::vec::fixed<dim+1> bsp;
        bsp.subvec(0,dim-1) = sp;
        bsp[dim] = 1. - arma::sum(sp);
        // The dof (flux through side) is computed as scalar product of the value with normal vector times side measure.
        this->dofs_.push_back(Dof(dim, 0, bsp, RefElement<dim>::normal_vector(sid)*RefElement<dim>::side_measure(sid), Value));
    }
    this->component_indices_.clear();
    this->nonzero_components_.resize(this->dofs_.size(), std::vector<bool>(this->n_components(), true));

    this->compute_node_matrix();
}





template class FE_RT0<1>;
template class FE_RT0<2>;
template class FE_RT0<3>;


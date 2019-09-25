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
 * @file    composed_quadrature.cc
 * @brief
 * @author  David Flanderka
 */

#include "fields/composed_quadrature.hh"
#include "fields/point_sets.hh"
#include "quadrature/quadrature.hh"
#include <memory>


template <unsigned int dim>
EvalPoints<dim>::EvalPoints()
{}

template <unsigned int dim>
BulkSubQuad<dim> EvalPoints<dim>::add_bulk(const Quadrature<dim> &quad)
{
    ASSERT(bulk_set_.c_quad_==nullptr).error("Multiple initialization of bulk point set!\n");

    bulk_set_.c_quad_ = this;
    for (auto p : quad.get_points()) bulk_set_.point_indices_.push_back( this->add_local_point(p) );
    return bulk_set_;
}

template <unsigned int dim>
SideSubQuad<dim> EvalPoints<dim>::add_side(const Quadrature<dim-1> &quad)
{
    ASSERT(side_set_.c_quad_==nullptr).error("Multiple initialization of side point set!\n");

    side_set_.c_quad_ = this;

    for (unsigned int j=0; j<RefElement<dim>::n_side_permutations; ++j) {
        for (unsigned int i=0; i<dim+1; ++i) {
            Quadrature<dim> high_dim_q(quad, i, j);
            for (auto p : high_dim_q.get_points()) {
                side_set_.point_indices_[j].push_back( this->add_local_point(p) );
            }
        }
    }

    return side_set_;
}

template <unsigned int dim>
unsigned int EvalPoints<dim>::add_local_point(arma::vec::fixed<dim> coords) {
    // Check if point exists in local points vector.
	for (unsigned int i=0; i<local_points_.size(); ++i) {
        if ( arma::norm(coords-local_points_[i], 2) < 4*std::numeric_limits<double>::epsilon() ) return i;
    }
	// Add new point if doesn't exist
	local_points_.push_back(coords);
    return local_points_.size()-1;
}


template class EvalPoints<1>;
template class EvalPoints<2>;
template class EvalPoints<3>;
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
 * @file    surface_depth.hh
 * @brief
 */

#ifndef SURFACE_DEPTH_HH_
#define SURFACE_DEPTH_HH_

#include <string>
#include <vector>
#include <armadillo>
#include "mesh/bih_tree.hh"
#include "input/input_exception.hh"

class Mesh;


class SurfaceDepth
{
public:
	/// Declaration of exceptions
	TYPEDEF_ERR_INFO( EI_Message, const std::string);
    DECLARE_INPUT_EXCEPTION(ExcSurfaceProjection, << EI_Message::val );

    /**
	 * Constructor
	 */
	SurfaceDepth(const Mesh *mesh, std::string surface_region, std::string surface_direction);

	/// Compute distance of point from given surface region (was set in constructor)
	double compute_distance(arma::vec3 point);

protected:
	/// Create projection matrix \p m_
	void create_projection_matrix(arma::vec3 surface_vec);

	/// Construct BIH tree above surface region of given name.
	void construct_bih_tree(Mesh *mesh, std::string surface_region);

	/// Tree of mesh elements
	BIHTree bih_tree_;

	/// normal vector of surface plane
	arma::vec3 surface_norm_vec_;

	/// projection matrix
	arma::mat m_;

	/// vector of nodes coordinates of elements above which BIH tree is created
	std::vector<arma::mat> nodes_coords_;
};

#endif /* SURFACE_DEPTH_HH_ */
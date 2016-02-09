/*
 * inspectelements.h
 *
 *  Created on: 13.4.2014
 *      Author: viktor
 */
#ifndef INSPECT_ELEMENTS_H_
#define INSPECT_ELEMENTS_H_

#include "system/system.hh"

#include "mesh/bounding_box.hh"
#include "mesh/mesh_types.hh"

#include "simplex.h"

#include <queue>

class Mesh; // forward declare

namespace computeintersection {

// forward declare
struct ProlongationLine;
struct ProlongationPoint;

template<unsigned int N, unsigned int M> class IntersectionPoint;
class IntersectionLine;
class IntersectionPolygon;
    
/**
* Main class, which takes mesh and you can call method for computing intersections for different dimensions of elements
* It can compute whole polygon area.
* It can create a mesh file with intersections
*/
class InspectElements {

    // For case 1D-2D - list of intersection points
    std::vector<std::vector<IntersectionPoint<1,2>>> intersection_point_list;
    
	// For case 2D-3D - list of intersectionpolygon
	std::vector<std::vector<IntersectionPolygon>> intersection_list;
	// For case 1D-3D - list of intersectionline
	std::vector<std::vector<IntersectionLine>> intersection_line_list;
	// Array of flags, which elements are computed
	std::vector<bool> closed_elements;
	std::vector<int> flag_for_3D_elements;

    // Used only for 2d-3d
	std::queue<ProlongationLine> prolongation_line_queue_2D;
	std::queue<ProlongationLine> prolongation_line_queue_3D;

	std::queue<ProlongationPoint> prolongation_point_queue; // Used only for 1d-3d

	int element_2D_index;   // Used only for 2d-3d

	Simplex<1> abscissa;    // Used only for 1d-3d
	Simplex<2> triangle;    // Used only for 2d-3d
	Simplex<3> tetrahedron;

	Mesh *mesh;
	std::vector<BoundingBox> elements_bb;
	BoundingBox mesh_3D_bb;

	void update_abscissa(const ElementFullIter &el);
	void update_triangle(const ElementFullIter &el);
	void update_tetrahedron(const ElementFullIter &el);

    /** @brief Prolongates the intersection.
     * According to properties of IPs of intersection line it prolongates:
     * A] to neghboring 1D element, if the IP is the end point of 1D element,
     * B] to neghboring 3D element, if the IP is on the side of 3D element.
     */
	void prolongate_1D_decide(const IntersectionLine &il, const ElementFullIter &ele_1d, const ElementFullIter &ele_3d);
    
    /** @brief Computes intersection of given 1d (\p ele_1d) and 3d (\p ele_3d) elements and prolongates.
     * Attention: abscissa and tetrahedron must be already updated!
     * Can be called recusively in prolongation process from prolongate_elements().
     */
	void prolongate_1D_element(const ElementFullIter &ele_1d, const ElementFullIter &ele_3d);
    
    // Used only for 2d-3d
    void computeIntersections2d3d();
    void computeIntersections2d3dProlongation(const ProlongationLine &pl);
    void computeIntersections2d3dUseProlongationTable(std::vector<unsigned int> &prolongation_table, 
                                                      const ElementFullIter &elm, 
                                                      const ElementFullIter &ele);

    bool intersectionExists(unsigned int elm_2D_idx, unsigned int elm_3D_idx);

public:

	InspectElements(Mesh *_mesh);
	~InspectElements();

	/**
	 * Every method needs to be implemented for different type of mesh intersection
	 */
	template<unsigned int subdim, unsigned int dim> 
	void compute_intersections();

	template<unsigned int subdim, unsigned int dim>
	void compute_intersections_init();

    const std::vector<IntersectionPoint<1,2>> & list_intersection_points(unsigned int ele_idx);
    const std::vector<IntersectionLine> & list_intersection_lines(unsigned int idx_component_1D);
    
	void print_mesh_to_file(std::string name);
	void print_mesh_to_file_1D(std::string name);

    /** @brief Computes the area of 2d-3d polygonal intersections (sum over all polygons).
     * @return the area of intersection polygon
     */
	double polygon_area();

    /** @brief Computes the length of 1d-3d line intersection (sum over all lines).
     * @return the line length
     */
    double line_length();
};


inline const std::vector< IntersectionLine >& InspectElements::list_intersection_lines(unsigned int ele_idx)
{
    ASSERT_LESS(ele_idx, intersection_line_list.size());
    return intersection_line_list[ele_idx];
}

inline const vector< IntersectionPoint< 1, 2 > >& InspectElements::list_intersection_points(unsigned int ele_idx)
{
    ASSERT_LESS(ele_idx, intersection_point_list.size());
    //cout << intersection_point_list.size();
    return intersection_point_list[ele_idx];
}


// Declaration of specializations implemented in cpp:
template<> void InspectElements::compute_intersections_init<1,2>();
template<> void InspectElements::compute_intersections_init<1,3>();
template<> void InspectElements::compute_intersections_init<2,3>();
template<> void InspectElements::compute_intersections<1,2>();
template<> void InspectElements::compute_intersections<1,3>();
template<> void InspectElements::compute_intersections<2,3>();

} // END namespace


#endif // INSPECT_ELEMENTS_H_
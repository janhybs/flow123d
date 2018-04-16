/*
 * mortar_assembly.hh
 *
 *  Created on: Feb 22, 2017
 *      Author: jb
 */

#ifndef SRC_FLOW_MORTAR_ASSEMBLY_HH_
#define SRC_FLOW_MORTAR_ASSEMBLY_HH_

#include "mesh/side_impl.hh"
#include "mesh/mesh.h"
#include "quadrature/intersection_quadrature.hh"
#include "flow/darcy_flow_mh.hh"
#include "la/local_system.hh"
#include <vector>


typedef std::shared_ptr<DarcyMH::EqData> AssemblyDataPtr;


class MortarAssemblyBase {
public:
    typedef vector<unsigned int> IsecList;

    MortarAssemblyBase(AssemblyDataPtr data)
    : data_(data),
      mixed_mesh_(data->mesh->mixed_intersections()),
      fix_velocity_flag(false)
    {

    }

    virtual ~MortarAssemblyBase() {};

    // Default assembly is empty to allow dummy implementation for dimensions without coupling.
    virtual void assembly(LocalElementAccessorBase<3> ele_ac) {};

    void fix_velocity(LocalElementAccessorBase<3> ele_ac) {
        fix_velocity_flag = true;
        this->assembly(ele_ac);
        fix_velocity_flag = false;
    }

protected:
    AssemblyDataPtr data_;
    MixedMeshIntersections &mixed_mesh_;
    LocalSystem loc_system_;
    bool fix_velocity_flag;

};


struct IsecData {
    arma::uvec vel_dofs;
    arma::uvec dofs;
    unsigned int dim;
    double delta;
    double ele_z_coord_;
    arma::uvec dirichlet_dofs;
    arma::vec dirichlet_sol;
    unsigned int n_dirichlet;
};


class P0_CouplingAssembler :public MortarAssemblyBase {
public:
    P0_CouplingAssembler(AssemblyDataPtr data);
    void assembly(LocalElementAccessorBase<3> ele_ac);
    void pressure_diff(LocalElementAccessorBase<3> ele_ac, double delta);
    void fix_velocity_local(const IsecData & row_ele, const IsecData &col_ele);
private:
    inline arma::mat & tensor_average(unsigned int row_dim, unsigned int col_dim) {
        return tensor_average_[4*row_dim + col_dim];
    }

    void add_to_linsys(double scale);

    vector<IsecData> isec_data_list;

    /// Row matrices to compute element pressure as average of boundary pressures
    std::vector< arma::mat > tensor_average_;
    std::vector< arma::vec > col_average_;
    IntersectionQuadratureP0 quadrature_;
    arma::mat product_;
    LocalElementAccessorBase<3> slave_ac_;

};



class P1_CouplingAssembler :public MortarAssemblyBase {
public:
    P1_CouplingAssembler(AssemblyDataPtr data)
: MortarAssemblyBase(data),
      rhs(5),
      dofs(5),
      dirichlet(5)
    {
        rhs.zeros();
    }

    void assembly(LocalElementAccessorBase<3> ele_ac);
    void add_sides(LocalElementAccessorBase<3> ele_ac, unsigned int shift, vector<int> &dofs, vector<double> &dirichlet);
private:

    arma::vec rhs;
    vector<int> dofs;
    vector<double> dirichlet;
};



#endif /* SRC_FLOW_MORTAR_ASSEMBLY_HH_ */

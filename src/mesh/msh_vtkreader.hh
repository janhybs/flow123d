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
 * @file    msh_vtkreader.hh
 * @brief
 * @author  dalibor
 */

#ifndef MSH_VTK_READER_HH
#define	MSH_VTK_READER_HH

#include <string>
#include <istream>
#include <pugixml.hpp>

#include "mesh/msh_basereader.hh"
#include "system/file_path.hh"

class VtkMeshReader : public BaseMeshReader {
public:
	/// Possible data sections in UnstructuredGrid - Piece node.
	enum DataSections {
	    points, cells, cell_data
	};

	/// Type of data formats - ascii, binary or compressed with zLib.
	enum DataFormat {
	    ascii, binary_uncompressed, binary_zlib
	};

	/**
	 * Map of DataArray sections in VTK file.
	 *
	 * For each field_name contains MeshDataHeader.
	 */
	typedef typename std::map< std::string, MeshDataHeader > HeaderTable;

	/**
     * Construct the VTK format reader from given filename.
     * This opens the file for reading.
     */
	VtkMeshReader(const FilePath &file_name);

	/// Destructor
	~VtkMeshReader();

	/**
	 * Check if nodes and elements of VTK mesh is compatible with \p mesh.
	 *
	 *  - to all nodes of VTK mesh must exists one and only one nodes in second mesh
	 *  - the same must occur for elements
	 *  - method fill vector \p vtk_to_gmsh_element_map_
	 *  - it is necessary to call this method before calling \p get_element_data
	 */
	void check_compatible_mesh(Mesh &mesh);

protected:
    /**
	 * Find header of DataArray section of VTK file given by field_name.
	 *
	 * Note: \p time has no effect (it is only for continuity with GMSH reader).
	 */
	MeshDataHeader & find_header(double time, std::string field_name) override;

    /// Reads table of DataArray headers through pugixml interface
    void make_header_table() override;

    /// Helper method that create DataArray header of given xml node (used from \p make_header_table)
    MeshDataHeader create_header(pugi::xml_node node, unsigned int n_entities, Tokenizer::Position pos);

    /// Get DataType by value of string
	DataType get_data_type(std::string type_str);

	/// Return size of value of data_type.
	unsigned int type_value_size(DataType data_type);

	/// Parse ascii data to data cache
	void parse_ascii_data(ElementDataCacheBase &data_cache, unsigned int size_of_cache, unsigned int n_components,
			unsigned int n_entities, Tokenizer::Position pos);

	/// Parse binary data to data cache
	void parse_binary_data(ElementDataCacheBase &data_cache, unsigned int size_of_cache, unsigned int n_components,
			unsigned int n_entities, Tokenizer::Position pos, DataType value_type);

	/// Uncompress and parse binary compressed data to data cache
	void parse_compressed_data(ElementDataCacheBase &data_cache, unsigned int size_of_cache, unsigned int n_components,
			unsigned int n_entities, Tokenizer::Position pos, DataType value_type);

	/// Set base attributes of VTK and get count of nodes and elements.
	void read_base_vtk_attributes(pugi::xml_node vtk_node, unsigned int &n_nodes, unsigned int &n_elements);

	/// Get position of AppendedData tag in VTK file
	Tokenizer::Position get_appended_position();

	/// Override @p BaseMeshReader::check_test_compatible_mesh.
	inline void check_test_compatible_mesh() override {
		ASSERT_GT(vtk_to_gmsh_element_map_.size(), 0)
				.error("Vector of mapping VTK to GMSH element is not initialized. Did you call check_compatible_mesh?");
	}

    /**
     * Implements @p BaseMeshReader::read_element_data.
     */
    void read_element_data(ElementDataCacheBase &data_cache, MeshDataHeader actual_header, unsigned int size_of_cache,
    		unsigned int n_components, std::vector<int> const & el_ids) override;

    /// header type of VTK file (only for appended data)
    DataType header_type_;

    /// variants of data format (ascii, appended, compressed appended)
    DataFormat data_format_;

    /// Table with data of DataArray headers
    HeaderTable header_table_;

    /// input stream allow read appended data, used only if this tag exists
    std::istream *data_stream_;

    /// store count of read entities
    unsigned int n_read_;

    /// get ids of elements in GMSH source mesh
    std::vector<int> vtk_to_gmsh_element_map_;

};

#endif	/* MSH_VTK_READER_HH */


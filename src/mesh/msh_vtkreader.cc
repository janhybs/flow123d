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
 * @file    msh_vtkreader.cc
 * @brief
 * @author  dalibor
 */


#include <iostream>
#include <vector>
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

#include "msh_vtkreader.hh"
#include "msh_gmshreader.h" // TODO move exception to base class and remove
#include "system/tokenizer.hh"
#include "system/system.hh"

#include "config.h"
#include <zlib.h>


/*******************************************************************
 * Helper methods
 */
template<typename T>
T read_binary_value(std::istream &data_stream)
{
	T val;
	data_stream.read(reinterpret_cast<char *>(&val), sizeof(val));
	return val;
}


uint64_t read_header_type(VtkMeshReader::DataType data_header_type, std::istream &data_stream)
{
	if (data_header_type == VtkMeshReader::DataType::uint64)
		return read_binary_value<uint64_t>(data_stream);
	else if (data_header_type == VtkMeshReader::DataType::uint32)
		return (uint64_t)read_binary_value<unsigned int>(data_stream);
	else {
		ASSERT(false).error("Unsupported header_type!\n");
		return 0;
	}
}


/*******************************************************************
 * implementation of VtkMeshReader
 */
VtkMeshReader::VtkMeshReader(const FilePath &file_name)
: BaseMeshReader()
{
	current_cache_ = new ElementDataCacheBase();
	parse_result_ = doc_.load_file( ((std::string)file_name).c_str() );
	read_base_vtk_attributes();
	// data of appended tag
	if (header_type_==DataType::undefined) { // no AppendedData tag
		appended_pos_ = 0;
	} else {
		set_appended_stream(file_name);
	}
}



VtkMeshReader::~VtkMeshReader()
{
	if (appended_pos_>0) delete appended_stream_;
}



void VtkMeshReader::read_base_vtk_attributes()
{
	pugi::xml_node node = doc_.child("VTKFile");
	// header type of appended data
	header_type_ = this->get_data_type( node.attribute("header_type").as_string() );
	// data format
	if (header_type_ == DataType::undefined) {
		data_format_ = DataFormat::ascii;
	} else {
		std::string compressor = node.attribute("compressor").as_string();
		if (compressor == "vtkZLibDataCompressor")
			data_format_ = DataFormat::binary_zlib;
		else
			data_format_ = DataFormat::binary_uncompressed;
	}
	// size of node and element vectors
	pugi::xml_node piece_node = node.child("UnstructuredGrid").child("Piece");
	n_nodes_ = piece_node.attribute("NumberOfPoints").as_uint();
	n_elements_ = piece_node.attribute("NumberOfCells").as_uint();
}



void VtkMeshReader::set_appended_stream(const FilePath &file_name) {
	// open ifstream for find position
	f_name_ = (std::string)file_name;
	appended_stream_ = new std::ifstream( f_name_);
	{
		// find line by tokenizer
		Tokenizer tok(file_name);
		if (! tok.skip_to("AppendedData"))
			THROW(GmshMeshReader::ExcMissingSection() << GmshMeshReader::EI_Section("AppendedData") << GmshMeshReader::EI_GMSHFile(tok.f_name()) );
		else {
			appended_pos_ = tok.get_position().file_position_;
		}
	}

	// find exact position of appended data (starts with '_')
	char c;
	appended_stream_->seekg(appended_pos_);
	do {
		appended_stream_->get(c);
	} while (c!='_');
	appended_pos_ = appended_stream_->tellg();
	delete appended_stream_; // close stream

	// open stream in binary mode
	appended_stream_ = new std::ifstream( f_name_, std::ios_base::in | std::ios_base::binary );
}



VtkMeshReader::DataArrayAttributes VtkMeshReader::get_data_array_attr(DataSections data_section, std::string data_array_name)
{
    static std::vector<std::string> section_names = {"Points", "Cells", "CellData"};

    pugi::xml_node node = doc_.child("VTKFile").child("UnstructuredGrid").child("Piece");
    if (data_section == DataSections::points) {
    	node = node.child("Points").child("DataArray");
    } else {
    	ASSERT(data_array_name!="").error("Empty Name attribute of DataArray!\n");
    	node = node.child( section_names[data_section].c_str() ).find_child_by_attribute("DataArray", "Name", data_array_name.c_str());
    }

    DataArrayAttributes attributes;
    attributes.name_ = node.attribute("name").as_string();
    attributes.type_ = this->get_data_type( node.attribute("type").as_string() );
    attributes.n_components_ = node.attribute("NumberOfComponents").as_uint(1);
    std::string format = node.attribute("format").as_string();
    if (format=="appended") {
        ASSERT(data_format_ != DataFormat::ascii)(data_array_name).error("Invalid format of DataArray!");
        attributes.offset_ = node.attribute("offset").as_uint();
    } else if (format=="ascii") {
    	ASSERT(data_format_ == DataFormat::ascii)(data_array_name).error("Invalid format of DataArray!");
        attributes.tag_value_ = node.child_value();
        boost::algorithm::trim(attributes.tag_value_);
    } else {
        ASSERT(false).error("Unsupported or missing VTK format.");
    }
    return attributes;
}



VtkMeshReader::DataType VtkMeshReader::get_data_type(std::string type_str) {
    // names of types in DataArray section
	static const std::map<std::string, DataType> types = {
			{"Int8",    DataType::int8},
			{"UInt8",   DataType::uint8},
			{"Int16",   DataType::int16},
			{"UInt16",  DataType::uint16},
			{"Int32",   DataType::int32},
			{"UInt32",  DataType::uint32},
			{"Int64",   DataType::int64},
			{"UInt64",  DataType::uint64},
			{"Float32", DataType::float32},
			{"Float64", DataType::float64},
			{"",        DataType::undefined}
	};

	std::map<std::string, DataType>::const_iterator it = types.find(type_str);
	if (it != types.end()) {
	    return it->second;
    } else {
        ASSERT(false).error("Unsupported VTK data type.");
        return DataType::uint32;
    }

}



unsigned int VtkMeshReader::type_value_size(VtkMeshReader::DataType data_type)
{
	static const std::vector<unsigned int> sizes = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0 };

	return sizes[data_type];
}



template<typename T>
typename ElementDataCache<T>::ComponentDataPtr VtkMeshReader::get_element_data( std::string field_name, double time,
		unsigned int n_entities, unsigned int n_components, bool &actual, std::vector<int> const & el_ids, unsigned int component_idx)
{
	DataArrayAttributes data_attr = this->get_data_array_attr(DataSections::cell_data, field_name);
	if ( !current_cache_->is_actual(time, field_name) ) {
    	unsigned int size_of_cache; // count of vectors stored in cache

	    // check that the header is valid, try to correct
	    if (this->n_elements_ != n_entities) {
	    	WarningOut().fmt("In file '{}', '$ElementData' section for field '{}'.\nWrong number of entities: {}, using {} instead.\n",
	                f_name_, field_name, this->n_elements_, n_entities);
	        // actual_header.n_entities=n_entities;
	    }

	    if (n_components == 1) {
	    	// read for MultiField to 'n_comp' vectors
	    	// or for Field if ElementData contains only one value
	    	size_of_cache = data_attr.n_components_;
	    }
	    else {
	    	// read for Field if more values is stored to one vector
	    	size_of_cache = 1;
	    	if (data_attr.n_components_ != n_components) {
	    		WarningOut().fmt("In file '{}', '$ElementData' section for field '{}'.\nWrong number of components: {}, using {} instead.\n",
	    				f_name_, field_name, data_attr.n_components_, n_components);
	    		data_attr.n_components_=n_components;
	    	}
	    }

	    // create vector of shared_ptr for cache
	    typename ElementDataCache<T>::CacheData data_cache;
		switch (data_format_) {
			case DataFormat::ascii: {
				data_cache = parse_ascii_data<T>( size_of_cache, n_components, this->n_elements_, data_attr.tag_value_ );
				break;
			}
			case DataFormat::binary_uncompressed: {
				ASSERT_PTR(appended_stream_).error();
				data_cache = parse_binary_data<T>( size_of_cache, n_components, this->n_elements_, appended_pos_+data_attr.offset_,
						data_attr.type_ );
				break;
			}
			case DataFormat::binary_zlib: {
				ASSERT_PTR(appended_stream_).error();
				data_cache = parse_compressed_data<T>( size_of_cache, n_components, this->n_elements_, appended_pos_+data_attr.offset_,
						data_attr.type_);
				break;
			}
			default: {
				ASSERT(false).error(); // should not happen
				break;
			}
		}

	    LogOut().fmt("time: {}; {} entities of field {} read.\n",
	    		time, n_read_, field_name);

	    actual = true; // use input header to indicate modification of @p data buffer

	    // set new cache
	    delete current_cache_;
	    current_cache_ = new ElementDataCache<T>(time, field_name, data_cache);
	}

	if (component_idx == std::numeric_limits<unsigned int>::max()) component_idx = 0;
	return static_cast< ElementDataCache<T> *>(current_cache_)->get_component_data(component_idx);
}


template<typename T>
typename ElementDataCache<T>::CacheData VtkMeshReader::parse_ascii_data(unsigned int size_of_cache, unsigned int n_components,
		unsigned int n_entities, std::string data_str)
{
    unsigned int idx, i_row;
    n_read_ = 0;

    typename ElementDataCache<T>::CacheData data_cache = ElementDataCache<T>::create_data_cache(size_of_cache, n_components*n_entities);

	std::istringstream istr(data_str);
	Tokenizer tok(istr);
	tok.next_line();
	for (i_row = 0; i_row < n_entities; ++i_row) {
    	for (unsigned int i_vec=0; i_vec<size_of_cache; ++i_vec) {
    		idx = i_row * n_components;
    		std::vector<T> &vec = *( data_cache[i_vec].get() );
    		for (unsigned int i_col=0; i_col < n_components; ++i_col, ++idx) {
    			vec[idx] = boost::lexical_cast<T>(*tok);
    			++tok;
    		}
    	}
        n_read_++;
	}

	return data_cache;
}


template<typename T>
typename ElementDataCache<T>::CacheData VtkMeshReader::parse_binary_data(unsigned int size_of_cache, unsigned int n_components,
		unsigned int n_entities, unsigned int data_pos, VtkMeshReader::DataType value_type)
{
    unsigned int idx, i_row;
    n_read_ = 0;

    typename ElementDataCache<T>::CacheData data_cache = ElementDataCache<T>::create_data_cache(size_of_cache, n_components*n_entities);
	appended_stream_->seekg(data_pos);
	uint64_t data_size = read_header_type(header_type_, *appended_stream_) / type_value_size(value_type);
	ASSERT_EQ(size_of_cache*n_components*n_entities, data_size).error();

	for (i_row = 0; i_row < n_entities; ++i_row) {
    	for (unsigned int i_vec=0; i_vec<size_of_cache; ++i_vec) {
    		idx = i_row * n_components;
    		std::vector<T> &vec = *( data_cache[i_vec].get() );
    		for (unsigned int i_col=0; i_col < n_components; ++i_col, ++idx) {
    			vec[idx] = read_binary_value<T>(*appended_stream_);
    		}
    	}
        n_read_++;
	}

	return data_cache;
}


template<typename T>
typename ElementDataCache<T>::CacheData VtkMeshReader::parse_compressed_data(unsigned int size_of_cache, unsigned int n_components,
		unsigned int n_entities, unsigned int data_pos, VtkMeshReader::DataType value_type)
{
	appended_stream_->seekg(data_pos);
	uint64_t n_blocks = read_header_type(header_type_, *appended_stream_);
	uint64_t u_size = read_header_type(header_type_, *appended_stream_);
	uint64_t p_size = read_header_type(header_type_, *appended_stream_);

	std::vector<uint64_t> block_sizes;
	block_sizes.reserve(n_blocks);
	for (uint64_t i = 0; i < n_blocks; ++i) {
		block_sizes.push_back( read_header_type(header_type_, *appended_stream_) );
	}

	stringstream decompressed_data;
	uint64_t decompressed_data_size = 0;
	for (uint64_t i = 0; i < n_blocks; ++i) {
		uint64_t decompressed_block_size = (i==n_blocks-1 && p_size>0) ? p_size : u_size;
		uint64_t compressed_block_size = block_sizes[i];

		std::vector<char> data_block(compressed_block_size);
		appended_stream_->read(&data_block[0], compressed_block_size);

		std::vector<char> buffer(decompressed_block_size);

		// set zlib object
		z_stream strm;
		strm.zalloc = 0;
		strm.zfree = 0;
		strm.next_in = (Bytef *)(&data_block[0]);
		strm.avail_in = compressed_block_size;
		strm.next_out = (Bytef *)(&buffer[0]);
		strm.avail_out = decompressed_block_size;

		// decompression of data
		inflateInit(&strm);
		inflate(&strm, Z_NO_FLUSH);
		inflateEnd(&strm);

		// store decompressed data to stream
		decompressed_data << std::string(buffer.begin(), buffer.end());
		decompressed_data_size += decompressed_block_size;
	}

    unsigned int idx, i_row;
    n_read_ = 0;

    typename ElementDataCache<T>::CacheData data_cache = ElementDataCache<T>::create_data_cache(size_of_cache, n_components*n_entities);
	uint64_t data_size = decompressed_data_size / type_value_size(value_type);
	ASSERT_EQ(size_of_cache*n_components*n_entities, data_size).error();

	for (i_row = 0; i_row < n_entities; ++i_row) {
    	for (unsigned int i_vec=0; i_vec<size_of_cache; ++i_vec) {
    		idx = i_row * n_components;
    		std::vector<T> &vec = *( data_cache[i_vec].get() );
    		for (unsigned int i_col=0; i_col < n_components; ++i_col, ++idx) {
    			vec[idx] = read_binary_value<T>(decompressed_data);
    		}
    	}
        n_read_++;
	}

	return data_cache;
}



// explicit instantiation of template methods
#define VTK_READER_GET_ELEMENT_DATA(TYPE) \
template typename ElementDataCache<TYPE>::ComponentDataPtr VtkMeshReader::get_element_data<TYPE>(std::string field_name, double time, \
	unsigned int n_entities, unsigned int n_components, bool &actual, std::vector<int> const & el_ids, unsigned int component_idx); \
template typename ElementDataCache<TYPE>::CacheData VtkMeshReader::parse_ascii_data<TYPE>(unsigned int size_of_cache, \
	unsigned int n_components, unsigned int n_entities, std::string data_str); \
template typename ElementDataCache<TYPE>::CacheData VtkMeshReader::parse_binary_data<TYPE>(unsigned int size_of_cache, \
	unsigned int n_components, unsigned int n_entities, unsigned int data_pos, VtkMeshReader::DataType value_type); \
template typename ElementDataCache<TYPE>::CacheData VtkMeshReader::parse_compressed_data<TYPE>(unsigned int size_of_cache, \
	unsigned int n_components, unsigned int n_entities, unsigned int data_pos, VtkMeshReader::DataType value_type); \
template TYPE read_binary_value<TYPE>(std::istream &data_stream)

VTK_READER_GET_ELEMENT_DATA(int);
VTK_READER_GET_ELEMENT_DATA(unsigned int);
VTK_READER_GET_ELEMENT_DATA(double);

template uint64_t read_binary_value<uint64_t>(std::istream &data_stream);

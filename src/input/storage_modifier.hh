/*
 * storage_modifier.hh
 *
 *  Created on: Feb 13, 2014
 *      Author: jb
 */

#ifndef STORAGE_MODIFIER_HH_
#define STORAGE_MODIFIER_HH_

#include "input/storage.hh"
#include "input/type_base.hh"
#include "input/type_record.hh"

namespace Input {

/**
 *
 */
StorageBase * modify_storage(const Type::TypeBase *target_type, const Type::TypeBase *source_type,
		StorageBase *source_storage, unsigned int index, unsigned int vec_size);

StorageBase * modify_storage(const Type::TypeBase *target_type, const Type::Record *source_type,
		StorageBase *source_storage, unsigned int index, unsigned int vec_size);
StorageBase * modify_storage(const Type::TypeBase *target_type, const Type::AbstractRecord *source_type,
		StorageBase *source_storage, unsigned int index, unsigned int vec_size);
StorageBase * modify_storage(const Type::TypeBase *target_type, const Type::Array *source_type,
		StorageBase *source_storage, unsigned int index, unsigned int vec_size);

} /* namespace Input */

#endif /* STORAGE_MODIFIER_HH_ */

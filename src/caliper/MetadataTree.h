// Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory.
//
// This file is part of Caliper.
// Written by David Boehme, boehme3@llnl.gov.
// LLNL-CODE-678900
// All rights reserved.
//
// For details, see https://github.com/scalability-llnl/Caliper.
// Please also see the LICENSE file for our additional BSD notice.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Record.h"
#include "cali_types.h"

#include <memory>

namespace cali
{
    class  Attribute;
    class  MemoryPool;
    struct MetaAttributeIDs;
    class  Node;
    class  Variant;
    
    class MetadataTree
    {
        struct MetadataTreeImpl;
        
        std::unique_ptr<MetadataTreeImpl> mP;
        
    public:
        
        MetadataTree();

        ~MetadataTree();

        MetadataTree(const MetadataTree&) = delete;
        MetadataTree& operator = (const MetadataTree&) = delete;
        
        // --- Modifying tree operations ---

        Node*
        get_path(size_t n, const Attribute attr[], const Variant data[], Node* parent, MemoryPool* pool);

        Node*
        remove_first_in_path(Node* path, const Attribute& attr, MemoryPool* pool);
        
        Node*
        replace_first_in_path(Node* path, const Attribute& attr, const Variant& data, MemoryPool* pool);
        Node*
        replace_all_in_path(Node* path, const Attribute& attr, size_t n, const Variant data[], MemoryPool* pool);

        // --- Non-modifying tree operations

        Node*
        find_node_with_attribute(const Attribute& attr, Node* path) const;
        
        // --- Data access ---
        
        Node*
        node(cali_id_t) const;
        Node*
        root() const;

        Node*
        type_node(cali_attr_type type) const;

        const MetaAttributeIDs*
        meta_attribute_ids() const;

        // --- I/O ---

        void
        write_new_nodes(WriteRecordFn fn);
    };

} // namespace cali

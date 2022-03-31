/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/types.hpp>
#include <vector>

template<typename index_type, typename vertex_type>
class MeshBuilder final {
public:
    void clear();
    void iclear();
    void vclear();
    void index(const index_type &idx);
    void vertex(const vertex_type &vtx);
    bool empty() const;
    bool iempty() const;
    bool vempty() const;
    size_t icount() const;
    size_t vcount() const;
    size_t isize() const;
    size_t vsize() const;
    const index_type *idata() const;
    const vertex_type *vdata() const;

private:
    std::vector<index_type> ivec;
    std::vector<vertex_type> vvec;
};

template<typename index_type, typename vertex_type>
inline void MeshBuilder<index_type, vertex_type>::clear()
{
    ivec.clear();
    vvec.clear();
}

template<typename index_type, typename vertex_type>
inline void MeshBuilder<index_type, vertex_type>::iclear()
{
    ivec.clear();
}

template<typename index_type, typename vertex_type>
inline void MeshBuilder<index_type, vertex_type>::vclear()
{
    vvec.clear();
}

template<typename index_type, typename vertex_type>
inline void MeshBuilder<index_type, vertex_type>::index(const index_type &idx)
{
    ivec.push_back(idx);
}

template<typename index_type, typename vertex_type>
inline void MeshBuilder<index_type, vertex_type>::vertex(const vertex_type &vtx)
{
    vvec.push_back(vtx);
}

template<typename index_type, typename vertex_type>
inline bool MeshBuilder<index_type, vertex_type>::empty() const
{
    return ivec.empty() && vvec.empty();
}

template<typename index_type, typename vertex_type>
inline bool MeshBuilder<index_type, vertex_type>::iempty() const
{
    return ivec.empty();
}

template<typename index_type, typename vertex_type>
inline bool MeshBuilder<index_type, vertex_type>::vempty() const
{
    return vvec.empty();
}

template<typename index_type, typename vertex_type>
inline size_t MeshBuilder<index_type, vertex_type>::icount() const
{
    return ivec.size();
}

template<typename index_type, typename vertex_type>
inline size_t MeshBuilder<index_type, vertex_type>::vcount() const
{
    return vvec.size();
}

template<typename index_type, typename vertex_type>
inline size_t MeshBuilder<index_type, vertex_type>::isize() const
{
    return sizeof(index_type) * ivec.size();
}

template<typename index_type, typename vertex_type>
inline size_t MeshBuilder<index_type, vertex_type>::vsize() const
{
    return sizeof(vertex_type) * vvec.size();
}

template<typename index_type, typename vertex_type>
inline const index_type *MeshBuilder<index_type, vertex_type>::idata() const
{
    return ivec.data();
}

template<typename index_type, typename vertex_type>
inline const vertex_type *MeshBuilder<index_type, vertex_type>::vdata() const
{
    return vvec.data();
}

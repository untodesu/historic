/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <vector>

template<typename IT, typename VT>
class MeshBuilder final {
public:
    MeshBuilder();
    void clear();
    void clearIndices();
    void clearVertices();
    void push(const IT &idx);
    void push(const VT &vtx);
    const bool empty() const;
    const IT *getIndices() const;
    const VT *getVertices() const;
    const size_t numIndices() const;
    const size_t numVertices() const;
    const size_t calcIBOSize() const;
    const size_t calcVBOSize() const;

private:
    std::vector<IT> indices;
    std::vector<VT> vertices;
};

template<typename IT, typename VT>
inline MeshBuilder<IT, VT>::MeshBuilder()
    : indices(), vertices()
{

}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::clear()
{
    indices.clear();
    vertices.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::clearIndices()
{
    indices.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::clearVertices()
{
    vertices.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::push(const IT &idx)
{
    indices.push_back(idx);
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::push(const VT &vtx)
{
    vertices.push_back(vtx);
}

template<typename IT, typename VT>
inline const bool MeshBuilder<IT, VT>::empty() const
{
    return indices.empty() && vertices.empty();
}

template<typename IT, typename VT>
inline const IT *MeshBuilder<IT, VT>::getIndices() const
{
    return indices.data();
}

template<typename IT, typename VT>
inline const VT *MeshBuilder<IT, VT>::getVertices() const
{
    return vertices.data();
}

template<typename IT, typename VT>
inline const size_t MeshBuilder<IT, VT>::numIndices() const
{
    return indices.size();
}

template<typename IT, typename VT>
inline const size_t MeshBuilder<IT, VT>::numVertices() const
{
    return vertices.size();
}

template<typename IT, typename VT>
inline const size_t MeshBuilder<IT, VT>::calcIBOSize() const
{
    return sizeof(IT) * indices.size();
}

template<typename IT, typename VT>
inline const size_t MeshBuilder<IT, VT>::calcVBOSize() const
{
    return sizeof(VT) * vertices.size();
}

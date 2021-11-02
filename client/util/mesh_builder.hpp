/*
 * mesh_builder.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>
#include <vector>

template<typename IT, typename VT>
class MeshBuilder final {
public:
    void clear();
    void iclear();
    void vclear();
    void index(const IT &idx);
    void vertex(const VT &vtx);
    bool empty() const;
    bool iempty() const;
    bool vempty() const;
    size_t icount() const;
    size_t vcount() const;
    size_t isize() const;
    size_t vsize() const;
    const IT *idata() const;
    const VT *vdata() const;

private:
    std::vector<IT> ivec;
    std::vector<VT> vvec;
};

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::clear()
{
    ivec.clear();
    vvec.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::iclear()
{
    ivec.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::vclear()
{
    vvec.clear();
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::index(const IT &idx)
{
    ivec.push_back(idx);
}

template<typename IT, typename VT>
inline void MeshBuilder<IT, VT>::vertex(const VT &vtx)
{
    vvec.push_back(vtx);
}

template<typename IT, typename VT>
inline bool MeshBuilder<IT, VT>::empty() const
{
    return ivec.empty() && vvec.empty();
}

template<typename IT, typename VT>
inline bool MeshBuilder<IT, VT>::iempty() const
{
    return ivec.empty();
}

template<typename IT, typename VT>
inline bool MeshBuilder<IT, VT>::vempty() const
{
    return vvec.empty();
}

template<typename IT, typename VT>
inline size_t MeshBuilder<IT, VT>::icount() const
{
    return ivec.size();
}

template<typename IT, typename VT>
inline size_t MeshBuilder<IT, VT>::vcount() const
{
    return vvec.size();
}

template<typename IT, typename VT>
inline size_t MeshBuilder<IT, VT>::isize() const
{
    return sizeof(IT) * ivec.size();
}

template<typename IT, typename VT>
inline size_t MeshBuilder<IT, VT>::vsize() const
{
    return sizeof(VT) * vvec.size();
}

template<typename IT, typename VT>
inline const IT *MeshBuilder<IT, VT>::idata() const
{
    return ivec.data();
}

template<typename IT, typename VT>
inline const VT *MeshBuilder<IT, VT>::vdata() const
{
    return vvec.data();
}

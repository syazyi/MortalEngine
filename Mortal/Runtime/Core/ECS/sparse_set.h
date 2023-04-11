#pragma once

#include <vector>
#include <numeric>
#include "ecs.h"

namespace mortal
{
    //template<typename T>
    constexpr EntityID invaildEntityID = std::numeric_limits<EntityID>::max();
    class sparse_set{
    public:
        sparse_set() = default;
        ~sparse_set() {}

        void Add(EntityID id);
        void Remove(EntityID id);
        bool IsContain(EntityID id);
    private:
        
        std::vector<uint32_t> m_SparseSet;
        std::vector<EntityID> m_DenseSet;
    };
} // namespace mortal

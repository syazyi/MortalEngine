#include "sparse_set.h"

namespace mortal {

	void sparse_set::Add(EntityID id)
	{
		assert(id <= 2100000000U);
		if (!IsContain(id)) {
			m_DenseSet.push_back(id);
			size_t denSize = m_DenseSet.size();
			size_t spaSize = m_SparseSet.size();
			if (id >= spaSize) {
				m_SparseSet.resize((id + 1) * 2, invaildEntityID);
			}
			m_SparseSet[id] = denSize - 1;
		}
	}

	void sparse_set::Remove(EntityID id)
	{
		if (IsContain(id)) {
			m_DenseSet[m_SparseSet[id]] = invaildEntityID;
			m_SparseSet[id] = invaildEntityID;
		}
	}

	bool sparse_set::IsContain(EntityID id)
	{
		size_t spaSize = m_SparseSet.size();
		if (spaSize > id && m_SparseSet[id] != invaildEntityID) {
			return true;
		}
		return false;
	}

}

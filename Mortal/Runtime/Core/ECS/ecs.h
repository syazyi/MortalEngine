#pragma once
#include "Core.h"

#ifdef  max
#undef max
#endif //  max

namespace mortal {
	using EntityID = uint32_t;
	using ComponentID = uint32_t;

	
	struct IndexGenerator_Entity
	{
		static EntityID GetIndex() {
			return entityId++;
		}
		inline static EntityID entityId = 0;
	};

	struct IndexGetter_Component {
		template<typename T>
		static ComponentID GetIndex() {
			static ComponentID i = componentId++;
			return i;
		}
		inline static ComponentID componentId = 0;
	};

	static EntityID GenerateEntityID() {
		return IndexGenerator_Entity::GetIndex();
	}

	template<typename T>
	static ComponentID GetComponentID() {
		return IndexGetter_Component::GetIndex<T>();
	}
}
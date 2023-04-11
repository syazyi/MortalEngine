#pragma once
#include "ecs.h"
#include <map>
namespace mortal
{
    struct Position
    {
        float x;
        float y;
        float z;
    };

    //16kB
    constexpr uint32_t ChunkCapability = 16000;
    template<typename EntityType>
    class Chunk {
    public:
        Chunk() {
            m_chunk.reserve(16000);
        }
    private:
        std::vector<EntityType> m_chunk;
        uint32_t chunkSize;
        uint32_t chunkTypeSize;
    };

    class IComponent {
    public:
        IComponent() = default;
        virtual ~IComponent(){}
        virtual void Create() {};

        using GetData = std::function<void*(void)>;
        GetData data;
    };

    template<typename ComponentType>
    class ComponentPool : public IComponent{
    public:
        ComponentPool() = default;
        void Create() override;
        void Delete();

    private:
        std::vector<ComponentType> m_Components;
    };

    class ComponentInfo {
    public:
        ComponentInfo() = default;
        
    private:
        IComponent* componentPool;
    };


    class Command {
    public:
        Command() = default;

        template<class T>
        void spawn(T&& component) {
            auto id = GetComponentID<T>();
            auto poolPtr = a.find(id);
            if (poolPtr == a.end()) {
                auto* temp = new ComponentPool<T>();
                temp->data = [component](void) mutable ->void* {
                    return reinterpret_cast<void*>(&component);
                };
                a[id] = temp;
            }
            a[id]->Create();
        }
    private:
        std::map<ComponentID, IComponent*> a;

    };

    template<typename ComponentType>
    inline void ComponentPool<ComponentType>::Create()
    {
            void* dataPtr = data();
            m_Components.push_back(*reinterpret_cast<ComponentType*>(dataPtr));
    }

 } // namespace mortal

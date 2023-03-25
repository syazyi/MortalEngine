// #ifndef EVENT_H_
// #define EVENT_H_

// #include "Core/Macro.h"


// namespace mortal
// {
//     enum class EEventType :  uint8_t{
//         None = 0,
//         WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
// 		AppTick, AppUpdate, AppRender,
// 		KeyPressed, KeyReleased, KeyTyped,
// 		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
//     };

//     enum class EEventCategory : uint8_t{
//         None = 0,
//         Application = 1 << 0,
//         Input = 1 << 1,
//         Keyboard = 1 << 2,
//         Mouse = 1 << 3,
//         MouseBotton = 1 << 4
//     };

//     class MORTAL_API Event{
//     public:
//         virtual ~Event() = default;

//         bool bHandle = false;

//         virtual EEventType GetEventType() = 0;
//         virtual uint8_t GetEventCategory() = 0;

//         //EEventType GetEventTypeStatic(){}

//         //collective function
//         bool IsInCategory(EEventCategory category){
//             return this->GetEventCategory() & static_cast<uint8_t>(category);
//         }
//     };

//     class MORTAL_API EventDispatcher{
//     public:
//         EventDispatcher() = default;

//         template<typename T, typename F>
//         bool Dispatch(const F& func){

//             //must be create a static function to ue
//             if(m_event.GetEventType() == T::GetEventTypeStatic()){
//                 m_event.bHandle |= func(static_cast<T&>(m_event));
//                 return true;
//             }
//             return false;
//         }
//     private:
//         Event& m_event;
//     };

//     #define MORTAL_EVENT_GETTYPE(type)      \
//         virtual EEventType GetEventType() override{   \
//             return type;                   \
//         }                                   

//     #define MORTAL_EVENT_GETCATEGORY(category)          \
//         virtual uint8_t GetEventCategory() override {   \
//             return static_cast<uint8_t>(category);      \
//         }                                               \
//         static EEventCategory GetEventCategoryStatic(){ \
//             return category;                            \
//         }

//     class WindowResizeEvent : public Event{
//     public:
//         WindowResizeEvent() = default;
//         MORTAL_EVENT_GETTYPE(EEventType::WindowResize)
//         MORTAL_EVENT_GETCATEGORY(EEventCategory::Application)
//     private:
//         int m_x;
//         int m_y;
//     };
// } // namespace mortal


// #endif
#ifndef SINGLETON_HH
#define SINGLETON_HH

#include <type_traits>

namespace Terryn
{

/// This is a standard implementation of the Singleton.
/// Please note that theres going to be an ever growing list of
/// requirements on this type.  This should only be used on
/// device drivers that are smartly implemented to avoid memory
/// leakage.
///
/// please drink responsibly.
template <class T,
          /// must be trivially constructable
          typename = std::enable_if<std::is_trivial_v<T>>,
          /// Must NOT be copy-constructable
          typename = std::enable_if<!std::is_copy_constructible_v<T>>,
          /// must NOT be move-constructable
          typename = std::enable_if<!std::is_move_constructible_v<T>>
          >
struct Singleton
{
    static T& Instance()
    {
        static T instance;
        return instance;
    }

    Singleton() = delete;
    Singleton(Singleton const&) = delete;
    Singleton & operator=(Singleton const&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton & operator =(Singleton&&) = delete;
};
}

#endif // SINGLETON_HH

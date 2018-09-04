/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

// Std includes:
#include <typeindex>
#include <memory>

namespace safl {
namespace detail {

class TypeEraser;

/**
 * @ingroup Util
 * @brief Check if the given type is a subclass of @ref TypeEraser.
 */
template<typename tType>
using IsErasedType = typename std::is_base_of<TypeEraser, tType>;

/**
 * @ingroup Util
 * @brief The base for classes which operate on or store an erased type.
 */
class TypeEraser
{
public:
    template<typename tType>
    [[ gnu::pure ]]
    bool isType() const noexcept
    {
        static_assert(!IsErasedType<tType>::value,
                      "The argument must NOT be a subclass of TypeEraser");
        return m_typeIndex == typeid(tType);
    }

    [[ gnu::pure ]]
    bool isType(std::type_index typeIndex) const noexcept
    {
        return m_typeIndex == typeIndex;
    }

    template<typename tType>
    [[ gnu::pure ]]
    bool isOfTypeAs(const tType &other) const noexcept
    {
        static_assert(IsErasedType<tType>::value,
                      "The argument must be a subclass of TypeEraser");
        return m_typeIndex == other.m_typeIndex;
    }

    template<typename tType>
    [[ gnu::pure ]]
    bool isOfTypeAs(const std::unique_ptr<tType> &other) const noexcept
    {
        return isOfTypeAs(*other);
    }

protected:
    TypeEraser() noexcept
        : m_typeIndex(typeid(NoType))
    {
    }

    TypeEraser(std::type_index typeIndex)
        : m_typeIndex(typeIndex)
    {
    }

    void setType(std::type_index typeIndex)
    {
        m_typeIndex = typeIndex;
    }

    ~TypeEraser() = default;

private:
    class NoType {};
    std::type_index m_typeIndex;
};

} // namespace detail
} // namespace safl

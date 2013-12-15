///
/// \file combinators.hpp
///
/// This file defines pattern combinators supported by our library.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// This file is a part of Mach7 library (http://parasol.tamu.edu/mach7/).
/// Copyright (C) 2011-2012 Texas A&M University.
/// All rights reserved.
///

#pragma once

#include "primitive.hpp" // FIX: Ideally this should be common.hpp, but GCC seem to disagree: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=55460

namespace mch ///< Mach7 library namespace
{

//------------------------------------------------------------------------------

/// Conjunction pattern combinator
template <typename P1, typename P2>
struct conjunction
{
    static_assert(is_pattern<P1>::value, "Argument P1 of a conjunction-pattern must be a pattern");
    static_assert(is_pattern<P2>::value, "Argument P2 of a conjunction-pattern must be a pattern");

    conjunction(const P1& p1, const P2& p2) : m_p1(p1), m_p2(p2) {}
    conjunction(P1&& p1, P2&& p2) noexcept : m_p1(std::move(p1)), m_p2(std::move(p2)) {}
    conjunction(conjunction&& c)  noexcept : m_p1(std::move(c.m_p1)), m_p2(std::move(c.m_p2)) {}
    conjunction& operator=(const conjunction&); // No assignment

    /// Type function returning a type that will be accepted by the pattern for
    /// a given subject type S. We use type function instead of an associated 
    /// type, because there is no a single accepted type for a #wildcard pattern
    /// for example. Requirement of #Pattern concept.
    template <typename S> 
    struct accepted_type_for 
    { 
        static_assert(std::is_same<
                          typename P1::template accepted_type_for<S>::type,
                          typename P2::template accepted_type_for<S>::type
                      >::value,"The type accepted by patterns in conjunction must be the same");
        typedef typename P1::template accepted_type_for<S>::type type; 
    };

    /// We parameterize over accepted type since the actually accepted type is 
    /// a function of the subject type.
    template <typename T>
    bool operator()(const T& subject) const { return m_p1(subject) && m_p2(subject); }

    P1 m_p1; ///< The 1st pattern in conjunction
    P2 m_p2; ///< The 2nd pattern in conjunction
};

/// #is_pattern_ is a helper meta-predicate capable of distinguishing all our patterns
template <typename P1, typename P2> struct is_pattern_<conjunction<P1,P2>> { static const bool value = true /*is_pattern<P1>::value && is_pattern<P2>::value*/; };

//------------------------------------------------------------------------------

/// Disjunction pattern combinator
template <typename P1, typename P2>
struct disjunction
{
    static_assert(is_pattern<P1>::value, "Argument P1 of a disjunction-pattern must be a pattern");
    static_assert(is_pattern<P2>::value, "Argument P2 of a disjunction-pattern must be a pattern");

    disjunction(const P1& p1, const P2& p2) : m_p1(p1), m_p2(p2) {}
    disjunction(P1&& p1, P2&& p2) noexcept : m_p1(std::move(p1)), m_p2(std::move(p2)) {}
    disjunction(disjunction&& c)  noexcept : m_p1(std::move(c.m_p1)), m_p2(std::move(c.m_p2)) {}
    disjunction& operator=(const disjunction&); // No assignment

    /// Type function returning a type that will be accepted by the pattern for
    /// a given subject type S. We use type function instead of an associated 
    /// type, because there is no a single accepted type for a #wildcard pattern
    /// for example. Requirement of #Pattern concept.
    template <typename S> 
    struct accepted_type_for 
    { 
        static_assert(std::is_same<
                          typename P1::template accepted_type_for<S>::type,
                          typename P2::template accepted_type_for<S>::type
                      >::value,"The type accepted by patterns in disjunction must be the same");
        typedef typename P1::template accepted_type_for<S>::type type; 
    };

    /// We parameterize over accepted type since the actually accepted type is 
    /// a function of the subject type.
    template <typename T>
    bool operator()(const T& subject) const { return m_p1(subject) || m_p2(subject); }

    P1 m_p1; ///< The 1st pattern of disjunction combinator
    P2 m_p2; ///< The 2nd pattern of disjunction combinator
};


/// #is_pattern_ is a helper meta-predicate capable of distinguishing all our patterns
template <typename P1, typename P2> struct is_pattern_<disjunction<P1,P2>> { static const bool value = true /*is_pattern<P1>::value && is_pattern<P2>::value*/; };

//------------------------------------------------------------------------------

/// Negation pattern combinator
template <typename P1>
struct negation
{
    static_assert(is_pattern<P1>::value, "Argument P1 of a negation-pattern must be a pattern");

    negation(const P1& p1) : m_p1(p1) {}
    negation(P1&& p1) noexcept : m_p1(std::move(p1)) {}
    negation(negation&& c)  noexcept : m_p1(std::move(c.m_p1)) {}
    negation& operator=(const negation&); // No assignment

    /// Type function returning a type that will be accepted by the pattern for
    /// a given subject type S. We use type function instead of an associated 
    /// type, because there is no a single accepted type for a #wildcard pattern
    /// for example. Requirement of #Pattern concept.
    template <typename S> struct accepted_type_for : P1::template accepted_type_for<S> { typedef S type; };

    /// We parameterize over accepted type since the actually accepted type is 
    /// a function of the subject type.
    template <typename T>
    bool operator()(const T& subject) const { return !m_p1(subject); }

    P1 m_p1; ///< The argument pattern of negation combinator
};

/// #is_pattern_ is a helper meta-predicate capable of distinguishing all our patterns
template <typename P1> struct is_pattern_<negation<P1>> { static const bool value = true /*is_pattern<P1>::value*/; };

//------------------------------------------------------------------------------

} // of namespace mch

//------------------------------------------------------------------------------

/// \note This operator has to be in the global namespace as Pi is unrestricted
///       and ADL won't work to find it!
template <typename P1, typename P2>
inline auto operator&&(P1&& p1, P2&& p2) noexcept
        -> typename std::enable_if<
                        mch::either_is_pattern<P1,P2>::value,
                        mch::conjunction<
                            decltype(mch::filter(std::forward<P1>(p1))),
                            decltype(mch::filter(std::forward<P2>(p2)))
                        >
                    >::type
{
    return mch::conjunction<
                            decltype(mch::filter(std::forward<P1>(p1))),
                            decltype(mch::filter(std::forward<P2>(p2)))
                        >(
                            mch::filter(std::forward<P1>(p1)),
                            mch::filter(std::forward<P2>(p2))
                         );
}

//------------------------------------------------------------------------------

/// \note This operator has to be in the global namespace as Pi is unrestricted
///       and ADL won't work to find it!
template <typename P1, typename P2>
inline auto operator||(P1&& p1, P2&& p2) noexcept
        -> typename std::enable_if<
                        mch::either_is_pattern<P1,P2>::value,
                        mch::disjunction<
                            decltype(mch::filter(std::forward<P1>(p1))),
                            decltype(mch::filter(std::forward<P2>(p2)))
                        >
                    >::type
{
    return mch::disjunction<
                            decltype(mch::filter(std::forward<P1>(p1))),
                            decltype(mch::filter(std::forward<P2>(p2)))
                        >(
                            mch::filter(std::forward<P1>(p1)),
                            mch::filter(std::forward<P2>(p2))
                         );
}

//------------------------------------------------------------------------------

/// \note This operator has to be in the global namespace as Pi is unrestricted
///       and ADL won't work to find it!
template <typename P1>
inline auto operator!(P1&& p1) noexcept 
        -> typename std::enable_if<
                        mch::is_pattern<P1>::value, 
                        mch::negation<decltype(mch::filter(std::forward<P1>(p1)))>
                    >::type 
{
    return mch::negation<decltype(mch::filter(std::forward<P1>(p1)))>(mch::filter(std::forward<P1>(p1)));
}

//------------------------------------------------------------------------------
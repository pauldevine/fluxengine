//          Copyright Joakim Karlsson & Kim Gräsman 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SNOWHOUSE_EQUALSCONTAINERCONSTRAINT_H
#define SNOWHOUSE_EQUALSCONTAINERCONSTRAINT_H

#include "expressions/expression.h"

namespace snowhouse
{
    namespace constraint_internal
    {
        template <typename T>
        inline bool default_comparer(const T& lhs, const T& rhs)
        {
            return lhs == rhs;
        }
    }

    template <typename ExpectedType, typename BinaryPredicate>
    struct EqualsContainerConstraint :
        Expression<EqualsContainerConstraint<ExpectedType, BinaryPredicate>>
    {
        EqualsContainerConstraint(
            const ExpectedType& expected, const BinaryPredicate predicate):
            m_expected(expected),
            m_predicate(predicate)
        {
        }

        template <typename ActualType>
        bool operator()(const ActualType& actual) const
        {
            typename ActualType::const_iterator actual_it;
            typename ExpectedType::const_iterator expected_it;

            for (actual_it = actual.begin(), expected_it = m_expected.begin();
                 actual_it != actual.end() && expected_it != m_expected.end();
                 ++actual_it, ++expected_it)
            {
                if (!m_predicate(*actual_it, *expected_it))
                {
                    return false;
                }
            }

            return actual_it == actual.end() && expected_it == m_expected.end();
        }

        const ExpectedType m_expected;
        const BinaryPredicate m_predicate;
    };

    template <typename ExpectedType>
    inline EqualsContainerConstraint<ExpectedType,
        bool (*)(const typename ExpectedType::value_type&,
            const typename ExpectedType::value_type&)>
    EqualsContainer(const ExpectedType& expected)
    {
        return EqualsContainerConstraint<ExpectedType,
            bool (*)(const typename ExpectedType::value_type&,
                const typename ExpectedType::value_type&)>(
            expected, constraint_internal::default_comparer);
    }

    template <typename ExpectedType, typename BinaryPredicate>
    inline EqualsContainerConstraint<ExpectedType, BinaryPredicate>
    EqualsContainer(
        const ExpectedType& expected, const BinaryPredicate predicate)
    {
        return EqualsContainerConstraint<ExpectedType, BinaryPredicate>(
            expected, predicate);
    }

    template <typename ExpectedType, typename BinaryPredicate>
    struct Stringizer<EqualsContainerConstraint<ExpectedType, BinaryPredicate>>
    {
        static std::string ToString(
            const EqualsContainerConstraint<ExpectedType, BinaryPredicate>&
                constraint)
        {
            std::ostringstream builder;
            builder << snowhouse::Stringize(constraint.m_expected);
            return builder.str();
        }
    };
}

#endif

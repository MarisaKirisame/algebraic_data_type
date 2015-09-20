#ifndef ALGEBRAIC_DATA_TYPE_HPP
#define ALGEBRAIC_DATA_TYPE_HPP
#include "pattern_tester.hpp"
#include "core.hpp"
#include "pattern_matcher.hpp"
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, UNUSED ) \
    template< typename ... UNUSED > \
    using NAME = typename ADT::get_constructor< WHICH, UNUSED ... >::type
    typedef std::tuple<> unit; //Fuck void
#endif // ALGEBRAIC_DATA_TYPE_HPP

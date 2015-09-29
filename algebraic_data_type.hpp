#ifndef ALGEBRAIC_DATA_TYPE_HPP
#define ALGEBRAIC_DATA_TYPE_HPP
#include "pattern_tester.hpp"
#include "core.hpp"
#include "pattern_matcher.hpp"
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, UNUSED ) \
    template< typename ... UNUSED > \
    auto NAME( UNUSED && ... UNUSED ## 0 ) { return constructor< ADT, WHICH >( std::forward< UNUSED >( UNUSED ## 0 ) ... ); }
    typedef std::tuple< > unit; //Fuck void
#endif // ALGEBRAIC_DATA_TYPE_HPP

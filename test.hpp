#ifndef TEST_HPP
#define TEST_HPP
#include "algebraic_data_type.hpp"
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace algebraic_data_type;

typedef algebraic_data_type< unit, recursive_indicator > Nat;
DECLARE_CONSTRUCTOR( Nat, 0, O, T );
DECLARE_CONSTRUCTOR( Nat, 1, S, T );
BOOST_AUTO_TEST_CASE( nat_test )
{
    Nat n = S<>( )( S<>( )( O<>( )( unit( ) ) ) );
    BOOST_CHECK( (
        n.match< O< arg >, S< S< arg > >, arg >(
            common::make_expansion(
                []( const Nat & n ){ return simple_match( n, [](const auto & l, const auto &){ return l.which_constructor == 0; } ); },
                []( const unit & ){ return false; } ) ) ) );
}

typedef algebraic_data_type< unit, unit > Bool;
DECLARE_CONSTRUCTOR( Bool, 1, False, T );
DECLARE_CONSTRUCTOR( Bool, 0, True, T );
BOOST_AUTO_TEST_CASE( bool_test )
{
    Bool b = True<>( )( unit( ) );
    BOOST_CHECK( b.match_pattern< wildstar >( ) );
    BOOST_CHECK( b.match_pattern< True< wildstar > >( ) );
    BOOST_CHECK( ( b.match< False< wildstar >, True< wildstar > >( []( ) { return true; } ) ) );
}

typedef algebraic_data_type< std::tuple< bool, bool, bool > > tri_bool;
DECLARE_CONSTRUCTOR( tri_bool, 0, tb, T );
BOOST_AUTO_TEST_CASE( tri_bool_test )
{
    tri_bool p = tb<>( )( std::make_tuple( true, false, false ) );
    BOOST_CHECK( ( p.match< tb< arg, arg, wildstar > >( []( bool l, bool r ) { return l && ! r; } ) ) );
}
#endif // TEST_HPP

#ifndef TEST_HPP
#define TEST_HPP
#include "algebraic_data_type.hpp"
#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp> //#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace algebraic_data_type;

typedef algebraic_data_type< recursive_indicator, unit > Nat;
DECLARE_CONSTRUCTOR( Nat, 1, O, T )
DECLARE_CONSTRUCTOR( Nat, 0, S, T )

BOOST_AUTO_TEST_CASE( nat_test )
{

    Nat n = S( S( O( ) ) );
    BOOST_CHECK( n.match_pattern( S( arg ) ) );
    BOOST_CHECK( (
        n.match(
            with(
                S( S( arg ) ),
                O( uim ),
                common::make_expansion(
                    []( const Nat & n ) { return simple_match( n, [](const auto &, const auto &) { return true; } ); },
                    []( ) { return false; } ) ) ) ) );
}

typedef algebraic_data_type< unit , unit > Bool;
DECLARE_CONSTRUCTOR( Bool, 1, False, T )
DECLARE_CONSTRUCTOR( Bool, 0, True, T )
BOOST_AUTO_TEST_CASE( bool_test )
{
    Bool b = True( );
    BOOST_CHECK( b.match_pattern( wildstar ) );
    BOOST_CHECK( b.match_pattern( True( uim ) ) );
    BOOST_CHECK( ! ( b.match(
        with( False( uim ), []( ) { return true; } ),
        with( True( uim ), []( ) { return false; } ) ) ) );
}

typedef algebraic_data_type< std::tuple< bool, bool, bool > > tri_bool;
DECLARE_CONSTRUCTOR( tri_bool, 0, tb, T )
BOOST_AUTO_TEST_CASE( tri_bool_test )
{
    tri_bool p = tb( true, false, false );
    BOOST_CHECK( ( p.match( with( tb( arg, arg, wildstar ), []( bool l, bool r ) { return l && ! r; } ) ) ) );
}

typedef algebraic_data_type< std::tuple< bool, recursive_indicator >, unit > bl;
DECLARE_CONSTRUCTOR( bl, 1, nil, t )
DECLARE_CONSTRUCTOR( bl, 0, cons, t )
BOOST_AUTO_TEST_CASE( bl_test )
{
    bl l = cons( true, nil( ) );
    BOOST_CHECK( ( l.match( with( cons( arg, arg ), []( bool b, const bl & ){ return b; } ) ) ) );
}

typedef algebraic_data_type< std::tuple< Bool, Bool > > meow;
DECLARE_CONSTRUCTOR( meow, 0, Meow, t )
BOOST_AUTO_TEST_CASE( meow_test )
{
    meow MEOW = Meow( True( ), False( ) );
    BOOST_CHECK( ( MEOW.match( with( Meow( True( uim ), False( uim ) ), []( ) { return true; } ) ) ) );
}
#endif // TEST_HPP

#include "algebraic_data_type.hpp"
using namespace algebraic_data_type;
typedef algebraic_data_type< unit, unit > Bool;
typedef algebraic_data_type< unit, recursive_indicator > Nat;
typedef algebraic_data_type< std::tuple< bool, bool, bool > > tri_bool;
DECLARE_CONSTRUCTOR( Bool, 0, True, T );
DECLARE_CONSTRUCTOR( Bool, 1, False, T );
DECLARE_CONSTRUCTOR( Nat, 0, O, T );
DECLARE_CONSTRUCTOR( Nat, 1, S, T );
DECLARE_CONSTRUCTOR( tri_bool, 0, tb, T );
int main( )
{
    tri_bool p = tb<>( )( std::make_tuple( true, false, false ) );
    assert( ( p.match< tb< arg, arg, wildstar > >( []( bool l, bool r ) { return l && ! r; } ) ) );
    Bool b = True<>( )( unit( ) );
    assert( b.match_pattern< wildstar >( ) );
    assert( b.match_pattern< True< wildstar > >( ) );
    typedef decltype( std::declval< Bool >( ) ) T;
    assert( ( b.match< False< wildstar >, True< wildstar > >( []( ) { return true; } ) ) );
    Nat n = S<>( )( S<>( )( O<>( )( unit( ) ) ) );
    assert( (
        n.match< O< arg >, S< S< arg > >, arg >(
            common::make_expansion(
                []( const Nat & n ){ return simple_match( n, [](const auto & l, const auto &){ return l.which_constructor == 0; } ); },
                []( const unit & ){ return false; } ) ) ) );
    std::cout << "pass!" << std::endl;
    return 0;
}
//Issue: add basic type, add test case, switch to boost::hana

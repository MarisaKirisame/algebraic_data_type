#include "algebraic_data_type.hpp"
int main( )
{
    using namespace algebraic_data_type;
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

#ifndef PATTERN_TESTER_HPP
#define PATTERN_TESTER_HPP
#include "core.hpp"
namespace algebraic_data_type
{
    struct wildstar;
    struct arg { };

    template< >
    struct pattern_tester< wildstar >
    {
        template< typename ARG >
        static bool match_pattern( const ARG & ) { return true; }
    };

    template< >
    struct pattern_tester< arg >
    {
        template< typename ARG >
        static bool match_pattern( const ARG & ) { return true; }
    };

    template< typename self_type, size_t which, typename ... PR >
    struct pattern_tester< constructor_indicator< self_type, which, PR ... > >
    {
        struct tester_helper
        {
            template< typename L, typename R >
            bool operator ( )( const L &, const R & r ) const
            {
                static_assert( std::is_same< typename L::constructor_type, self_type >::value, "Constructor Mismatch" );
                return L::which_constructor == which && pattern_tester< PR ... >::match_pattern( extract_recursive_wrapper( r ) );
            }
        };

        template< typename ... ARG >
        static bool match_pattern( const algebraic_data_type< ARG ... > & s ) { return simple_match( s, tester_helper( ) ); }

        template< typename T >
        static bool match_pattern( const T & ) { return false; }
    };

    template< typename FIRST, typename SECOND >
    struct pattern_tester< FIRST, SECOND >
    {
        template< typename FST, typename SND >
        static bool match_pattern( const std::pair< FST, SND > & p )
        { return FST::match_pattern( extract_recursive_wrapper( p.first ) ) && SND::match_pattern( extract_recursive_wrapper( p.second ) ); }

        template< typename FST, typename SND >
        static bool match_pattern( const std::tuple< FST, SND > & p )
        { return FST::match_pattern( extract_recursive_wrapper( p.first ) ) && SND::match_pattern( extract_recursive_wrapper( p.second ) ); }
    };

    template< >
    struct pattern_tester< >
    { static bool match_pattern( const std::tuple< > & ) { return true; } };

    template< typename FIRST, typename ... REST >
    struct pattern_tester< FIRST, REST ... >
    {
        template< typename ... T >
        static bool match_pattern( const std::tuple< T ... > & t )
        { return FIRST::match_pattern( extract_recursive_wrapper( std::get< 0 >( t ) ) ) && pattern_tester< REST ... >::match_pattern( tuple_pop( t ) ); }
    };
}
#endif // PATTERN_TESTER_HPP

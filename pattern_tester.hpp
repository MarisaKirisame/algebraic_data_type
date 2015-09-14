#ifndef PATTERN_TESTER_HPP
#define PATTERN_TESTER_HPP
#include "core.hpp"
namespace algebraic_data_type
{
    struct wildstar;
    struct arg { };

    template< typename ... TR, typename T >
    auto simple_match( const algebraic_data_type< TR ... > & adt, const T & t )
    {
        typedef algebraic_data_type< TR ... > adt_type;
        typedef decltype(
            t( std::declval< typename adt_type::template get_constructor< 0 >::type >( ),
               std::declval< typename boost::mpl::front< typename adt_type::variant_arg_type >::type::second_type >( ) ) ) ret_type;
        typename adt_type::template match_visitor< T, ret_type > smv { t };
        return adt.data.apply_visitor( smv );
    }

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
        static bool match_pattern( const std::pair< FST, SND > & p ) { return FST::match_pattern( p.first ) && SND::match_pattern( p.second ); }

        template< typename FST, typename SND >
        static bool match_pattern( const std::tuple< FST, SND > & p ) { return FST::match_pattern( p.first ) && SND::match_pattern( p.second ); }
    };

    template< >
    struct pattern_tester< >
    { static bool match_pattern( const std::tuple< > & ) { return true; } };

    template< typename FIRST, typename ... REST >
    struct pattern_tester< FIRST, REST ... >
    {
        template< typename ... T >
        static bool match_pattern( const std::tuple< T ... > & t )
        { return FIRST::match_pattern( std::get< 0 >( t ) ) && pattern_tester< REST ... >::match_pattern( tuple_pop( t ) ); }
    };
}
#endif // PATTERN_TESTER_HPP

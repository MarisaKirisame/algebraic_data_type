#ifndef PATTERN_TESTER_HPP
#define PATTERN_TESTER_HPP
#include "core.hpp"
namespace algebraic_data_type
{
    struct wildstar;
    struct arg;

    template< typename ... TR, typename T >
    auto simple_match( const algebraic_data_type< TR ... > & adt, const T & t )
    {
        typedef algebraic_data_type< TR ... > adt_type;
        typedef decltype(
            t( tag< typename adt_type::template get_constructor< 0 >::type >( ),
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

    template< >
    struct pattern_tester< multi_tester< > > { static bool match_pattern( const std::tuple< > & ) { return true; } };

    template< typename self_type, size_t which, typename ... PR >
    struct pattern_tester< constructor_indicator< self_type, which, PR ... > >
    {
        struct tester_helper
        {
            template< typename L, typename R >
            bool operator ( )( const L &, const R & r ) const
            {
                typedef typename L::type constructor_type;
                static_assert( std::is_same< typename constructor_type::adt_type, self_type >::value, "Constructor Mismatch" );
                return common::make_expansion(
                            [](const std::true_type &, const auto & r )
                            { return pattern_tester< multi_tester< PR ... > >::match_pattern( extract_recursive_wrapper( r ) ); },
                            [](const std::false_type &, const auto & ) { return false; } )
                            ( std::integral_constant< bool, constructor_type::which_constructor == which >( ), r );
            }
        };

        template< typename ... ARG >
        static bool match_pattern( const algebraic_data_type< ARG ... > & s ) { return simple_match( s, tester_helper( ) ); }

        template< typename T >
        static bool match_pattern( const std::tuple< T > & t ) { return match_pattern( extract_recursive_wrapper( std::get< 0 >( t ) ) ); }

        template< typename T >
        static bool match_pattern( const T & ) { return false; }
    };

    template< typename FIRST, typename SECOND >
    struct pattern_tester< multi_tester< FIRST, SECOND > >
    {
        template< typename FST, typename SND >
        static bool match_pattern( const std::pair< FST, SND > & p )
        {
            return
                    pattern_tester< FIRST >::match_pattern( extract_recursive_wrapper( p.first ) ) &&
                    pattern_tester< SECOND >::match_pattern( extract_recursive_wrapper( p.second ) );
        }

        template< typename FST, typename SND >
        static bool match_pattern( const std::tuple< FST, SND > & p )
        {
            return
                    pattern_tester< FIRST >::match_pattern( extract_recursive_wrapper( std::get< 0 >( p ) ) ) &&
                    pattern_tester< SECOND >::match_pattern( extract_recursive_wrapper( std::get< 1 >( p ) ) );
        }
    };

    template< typename FIRST, typename ... REST >
    struct pattern_tester< multi_tester< FIRST, REST ... > >
    {
        template< typename ... T >
        static bool match_pattern( const std::tuple< T ... > & t )
        {
            return
                    pattern_tester< FIRST >::match_pattern( extract_recursive_wrapper( std::get< 0 >( t ) ) ) &&
                    pattern_tester< multi_tester< REST ... > >::match_pattern( tuple_pop( t ) );
        }
    };
}
#endif // PATTERN_TESTER_HPP

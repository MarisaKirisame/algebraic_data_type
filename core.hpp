#ifndef CORE_HPP
#define CORE_HPP
#include <type_traits>
#include <iostream>
#include <utility>
#include <boost/variant.hpp>
#include <tuple>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/pop_back.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/arg.hpp>
#include <boost/mpl/unpack_args.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/placeholders.hpp>
#include <memory>
#include "../cpp_common/expansion.hpp"
namespace algebraic_data_type
{

    template< typename adt_type, size_t which, typename ... TR > struct constructor_indicator;

    template< typename T >
    struct constructor_variant_getter { T operator ( )( T && t ) { return t; } };

    template< typename adt_type, size_t which, typename ... TR >
    struct constructor_variant_getter< constructor_indicator< adt_type, which, TR ... > >
    { auto operator ( )( constructor_indicator< adt_type, which, TR ... > && c ) { return c.constructor; } };

    template< typename T >
    auto constructor_variant( T && t ) { return constructor_variant_getter< T >( )( std::forward< T >( t ) ); }

    template< typename T >
    struct strip_recursive_wrapper { typedef T type; };

    template< typename T >
    struct strip_recursive_wrapper< boost::recursive_wrapper< T > > { typedef T type; };

    template< typename ... T >
    struct strip_recursive_wrapper< std::tuple< T ... > > { typedef std::tuple< typename strip_recursive_wrapper< T >::type ... > type; };

    template< typename L, typename R >
    struct strip_recursive_wrapper< std::pair< L, R > >
    { typedef std::pair< typename strip_recursive_wrapper< L >::type, typename strip_recursive_wrapper< R >::type > type; };

    template< typename T > struct tag { typedef T type; };

    template< typename ADT_TYPE, size_t which, typename ... TR >
    struct constructor_indicator
    {
        typedef ADT_TYPE adt_type;
        constexpr static size_t which_constructor = which;
        typedef typename strip_recursive_wrapper< typename adt_type::template constructor_parameter_type< which >::type >::type constructor_type;
        std::pair< boost::mpl::int_< which >, constructor_type > constructor;
        constructor_indicator( ) : constructor( std::make_pair( boost::mpl::int_< which >( ), constructor_type( ) ) ) { }

        constructor_indicator( const constructor_indicator & arg ) = delete;
        constructor_indicator( constructor_indicator && arg ) = delete;
        template< typename ... ARG >
        constructor_indicator( ARG && ... arg ) :
            constructor(
                boost::mpl::int_< which >( ),
                constructor_type( std::make_tuple( constructor_variant( std::forward< ARG >( arg ) ) ... ) ) ) { }
    };

    template< typename T > struct pattern_tester;
    template< typename T > struct pattern_matcher;
    template< typename ... match_expression > struct multi_matcher;
    template< typename ... match_expression > struct multi_tester;
    struct recursive_indicator { };

    struct to_variant
    {
        template< typename T, typename ... TR >
        struct apply
        {
            struct get
            {
                template< typename ... TT >
                struct apply { typedef boost::variant< TT ... > type; };
            };
            typedef typename
            boost::mpl::eval_if_c
            <
                boost::mpl::size< T >::value == 0,
                boost::mpl::apply< get, TR ... >,
                boost::mpl::apply< to_variant, typename boost::mpl::pop_front< T >::type, typename boost::mpl::front< T >::type, TR ... >
            >::type type;
        };
    };

    template< typename SELF_TYPE, typename T >
    struct unfold_recursive_wrapper { typedef T type; }; //Open: Add more specialization to deal with different case.

    template< typename SELF_TYPE >
    struct unfold_recursive_wrapper< SELF_TYPE, recursive_indicator > { typedef boost::recursive_wrapper< SELF_TYPE > type; };

    template< typename SELF_TYPE, typename L, typename R >
    struct unfold_recursive_wrapper< SELF_TYPE, std::pair< L, R > >
    {
        typedef
            std::pair
            <
                typename unfold_recursive_wrapper< SELF_TYPE, L >::type,
                typename unfold_recursive_wrapper< SELF_TYPE, R >::type
            >
            type;
    };

    template< typename SELF_TYPE, typename ... T >
    struct unfold_recursive_wrapper< SELF_TYPE, std::tuple< T ... > >
    {
        typedef
            std::tuple< typename unfold_recursive_wrapper< SELF_TYPE, T >::type ... >
            type;
    };

    template< typename T >
    struct wrap_tuple { typedef std::tuple< T > type; }; //Open: Add more specialization to deal with different case.

    template< typename ... T >
    struct wrap_tuple< std::tuple< T ... > > { typedef std::tuple< T ... > type; };

    template< typename SELF_TYPE, typename T >
    struct desugar { typedef typename wrap_tuple< typename unfold_recursive_wrapper< SELF_TYPE, T >::type >::type type; };

    template< typename ... TR >
    struct algebraic_data_type
    {
        typedef algebraic_data_type< TR ... > self_type;
        struct add_pair
        {
            template< typename F, typename T >
            struct apply
            {
                typedef typename
                boost::mpl::push_back
                <
                    F,
                    std::pair
                    <
                        boost::mpl::int_< boost::mpl::size< F >::value >,
                        typename desugar< self_type, T >::type
                    >
                >::type type;
            };
        };
        typedef typename boost::mpl::fold< boost::mpl::vector< TR ... >, boost::mpl::vector< >, add_pair >::type variant_arg_type;
        typename to_variant::template apply< variant_arg_type >::type data;

        template< typename T >
        algebraic_data_type( T && d ) : data( constructor_variant( std::forward< T >( d ) ) ) { }

        template< size_t which, typename ... PR >
        struct get_constructor { typedef constructor_indicator< self_type, which, PR ... > type; };

        template< size_t which >
        struct constructor_parameter_type
        { typedef typename boost::mpl::template at_c< variant_arg_type, which >::type::second_type type; };

        template< typename T, typename ret_type >
        struct match_visitor : boost::static_visitor< ret_type >
        {
            const T & t;
            match_visitor( const T & t ) : t( t ) { }
            template< typename ARG >
            ret_type operator( )( const ARG & arg ) const
            { return t( tag< typename get_constructor< ARG::first_type::value >::type >( ), arg.second ); }
        };

        template< typename T >
        bool match_pattern( ) const { return pattern_tester< T >::match_pattern( * this ); }

        template< typename ... MATCH_EXP, typename F >
        auto match( const F & f ) const { return pattern_matcher< multi_matcher< MATCH_EXP ... > >::match( *this, f ); }
    };

    template< typename T >
    struct recursive_wrapper_extractor
    { T operator ( )( const T & t ) const { return t; } };

    template< typename T >
    struct recursive_wrapper_extractor< boost::recursive_wrapper< T > >
    { T operator ( )( const boost::recursive_wrapper< T > & t ) const { return t.get( ); } };

    template< typename T >
    auto extract_recursive_wrapper( const T & t ) { return recursive_wrapper_extractor< T >( )( t ); }

    template< size_t, typename F >
    auto expand_tuple_inner( const F & f, const std::tuple< > & )
    { return f( ); }

    template< size_t nth, typename F, typename ... T, typename ... REST >
    auto expand_tuple_inner( const F & f, const std::tuple< T ... > & t, const REST & ... r )
    {
        return common::make_expansion(
                [&]( const auto & t, std::true_type ) { return f( r ..., std::get< nth >( t ) ); },
                [&]( const auto & t, std::false_type ) { return expand_tuple_inner< nth + 1 >( f, t, r ..., std::get< nth >( t ) ); } )
                ( t, std::integral_constant< bool, std::tuple_size< std::tuple< T ... > >::value == nth + 1 >( ) );
    }

    template< typename F, typename ... T >
    auto expand_tuple( const F & f, const std::tuple< T ... > & t )
    { return expand_tuple_inner< 0 >( f, t ); }

    struct ignore_tie
    {
        template< typename FIRST, typename ... REST >
        auto operator ( )( const FIRST &, const REST & ... r ) const
        { return std::tie( r ... ); }
    };

    template< typename ... T >
    auto tuple_pop( const std::tuple< T ... > & t )
    { return expand_tuple( ignore_tie( ), t ); }
}
#endif // CORE_HPP

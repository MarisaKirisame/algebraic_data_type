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
#include "../misc/expansion.hpp"
//#include <boost/hana.hpp> //debian testing does not has high enough version of clang and hana is unable to detect clang in unstable. Got to wait for a while.
struct recursive_indicator { };
struct wildstar;
struct arg { };
template< typename CONSTRUCTOR_TYPE, size_t which, typename ... TR >
struct constructor_indicator
{
    constexpr static size_t which_constructor = which;
    typedef CONSTRUCTOR_TYPE constructor_type;
    template< typename ARG >
    CONSTRUCTOR_TYPE operator ( )( ARG && arg ) const
    { return CONSTRUCTOR_TYPE( std::make_pair( boost::mpl::int_< which >( ), std::forward< ARG >( arg ) ) ); }
};

struct to_variant
{
    struct inner
    {
        template< typename T, typename ... TR >
        struct apply
        {
            typedef typename
            boost::mpl::eval_if_c
            <
                boost::mpl::size< T >::value == 1,
                boost::mpl::identity< boost::variant< TR ... > >,
                boost::mpl::apply< inner, typename boost::mpl::pop_front< T >::type, typename boost::mpl::front< T >::type, TR ... >
            >::type type;
        };
    };
    template< typename T >
    struct apply : inner::apply< typename boost::mpl::push_back< T, boost::mpl::void_ >::type > { };
};

template< typename SELF_TYPE, typename T >
struct unfold_recursive { typedef T type; }; //Open: Add more specialization to deal with different case.

template< typename SELF_TYPE >
struct unfold_recursive< SELF_TYPE, recursive_indicator > { typedef boost::recursive_wrapper< SELF_TYPE > type; };

template< typename ... T >
struct pattern_tester;

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

template< typename ... TR >
struct algebraic_data_type;

template< typename self_type, size_t which, typename ... PR >
struct pattern_tester< constructor_indicator< self_type, which, PR ... > >
{
    struct tester_helper
    {
        template< typename L, typename R >
        bool operator ( )( const L &, const R & r ) const
        {
            static_assert( std::is_same< typename L::constructor_type, self_type >::value, "Constructor Mismatch" );
            return L::which_constructor == which && pattern_tester< PR ... >::match_pattern( r );
        }
    };

    template< typename ... ARG >
    static bool match_pattern( const algebraic_data_type< ARG ... > & s ) { return simple_match( s, tester_helper( ) ); }

    template< typename ... ARG >
    static bool match_pattern( const boost::recursive_wrapper< algebraic_data_type< ARG ... > > & s ) { return simple_match( s, tester_helper( ) ); }

    template< typename T >
    static bool match_pattern( const T & ) { return false; }
};

template< typename FIRST, typename SECOND >
struct pattern_tester< FIRST, SECOND >
{
    template< typename FST, typename SND >
    static bool match_pattern( const std::pair< FST, SND > & p )
    { return FST::match_pattern( p.first ) && SND::match_pattern( p.second ); }

    template< typename FST, typename SND >
    static bool match_pattern( const std::tuple< FST, SND > & p )
    { return FST::match_pattern( p.first ) && SND::match_pattern( p.second ); }
};

template< >
struct pattern_tester< >
{ static bool match_pattern( const std::tuple< > & ) { return true; } };

template< size_t nth, typename F, typename ... T, typename ... REST >
auto expand_tuple_inner( const F & f, const std::tuple< T ... > & t, const REST & ... r )
{
    return misc::make_expansion(
        [&]( const auto & t, boost::mpl::true_ ) { return f( r ..., std::get< nth >( t ) ); },
        [&]( const auto & t, boost::mpl::false_ ) { return expand_tuple_inner< nth + 1 >( f, t, r ..., std::get< nth >( t ) ); } )
            ( t, boost::mpl::bool_< std::tuple_size< std::tuple< T ... > >::value == nth + 1 >( ) );
}

template< size_t, typename F >
auto expand_tuple_inner( const F & f, const std::tuple< > & )
{ return f( ); }

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

template< typename FIRST, typename ... REST >
struct pattern_tester< FIRST, REST ... >
{
    template< typename ... T >
    static bool match_pattern( const std::tuple< T ... > & t )
    { return FIRST::match_pattern( std::get< 0 >( t ) ) && pattern_tester< REST ... >::match_pattern( tuple_pop( t ) ); }//TODO: Add Tuple_pop, which rely on expand_tuple
};

template< typename ... TR >
struct algebraic_data_type
{
    template< typename T >
    algebraic_data_type( T && t ) : data( std::forward< T >( t ) ) { }
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
                    typename unfold_recursive< self_type, T >::type
                >
            >::type type;
        };
    };
    typedef typename boost::mpl::fold< boost::mpl::vector< TR ... >, boost::mpl::vector< >, add_pair >::type variant_arg_type;
    typename to_variant::template apply< variant_arg_type >::type data;

    template< size_t which, typename ... PR >
    struct get { typedef constructor_indicator< self_type, which, PR ... > type; };

    template< typename T, typename ret_type >
    struct match_visitor : boost::static_visitor< ret_type >
    {
        const T & t;
        match_visitor( const T & t ) : t( t ) { }
        template< typename ARG >
        ret_type operator( )( const ARG & arg ) const { return t( typename get< ARG::first_type::value >::type( ), arg.second ); }
    };

    template< typename T >
    bool match_pattern( ) const { return pattern_tester< T >::match_pattern( * this ); }

};

template< typename ... TR, typename T >
auto simple_match( const algebraic_data_type< TR ... > & adt, const T & t )
{
    typedef algebraic_data_type< TR ... > adt_type;
    typedef decltype(
        t(  std::declval< typename adt_type::template get< 0 >::type >( ),
            std::declval< typename boost::mpl::front< typename adt_type::variant_arg_type >::type::second_type >( ) ) ) ret_type;
    typename adt_type::template match_visitor< T, ret_type > smv { t };
    return adt.data.apply_visitor( smv );
}

template< typename L, typename R >
auto simple_match( const boost::recursive_wrapper< L > & l, const R & r )
{ return simple_match( l.get( ), r ); }

template< typename match_expression >
struct matcher
{

};

struct unit { }; //Fuck void
typedef algebraic_data_type< unit, unit > Bool;
typedef algebraic_data_type< unit, recursive_indicator > Nat;
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, UNUSED ) \
template< typename ... UNUSED > \
using NAME = typename ADT::get< WHICH, UNUSED ... >::type

DECLARE_CONSTRUCTOR( Bool, 0, True, T );
DECLARE_CONSTRUCTOR( Bool, 1, False, T );
DECLARE_CONSTRUCTOR( Nat, 0, O, T );
DECLARE_CONSTRUCTOR( Nat, 1, S, T );

int main( )
{
    Nat n = O<>()(unit());
    assert( ! simple_match( n, misc::make_expansion( [](const S<> &, const auto &) { return true; }, [](const O<> &, const auto &) { return false; }) ) );
    assert( ! n.match_pattern< S< S< wildstar > > >( ) );
    assert( n.match_pattern< O< wildstar > >( ) );
    return 0;
}

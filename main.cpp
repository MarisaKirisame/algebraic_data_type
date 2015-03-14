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

template< typename ... T >
struct pattern_matcher;

template< >
struct pattern_matcher< wildstar >
{
    template< typename EXP, typename F, typename ... ARG >
    static auto match( const EXP &, const F & f, const ARG & ... rst )
    { return f( rst ... ); }
};

template< >
struct pattern_matcher< arg >
{
    template< typename EXP, typename F, typename ... ARG >
    static auto match( const EXP & e, const F & f, const ARG & ... rst )
    { return f( rst ..., e ); }
};

template< typename T >
struct recursive_wrapper_extractor
{ T operator ( )( const T & t ) const { return t; } };

template< typename T >
struct recursive_wrapper_extractor< boost::recursive_wrapper< T > >
{ T operator ( )( const boost::recursive_wrapper< T > & t ) const { return t.get( ); } };

template< typename T >
auto extract_recursive_wrapper( const T & t ) { return recursive_wrapper_extractor< T >( )( t ); }

template< typename self_type, size_t which, typename T >
struct pattern_matcher< constructor_indicator< self_type, which, T > >
{
    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const algebraic_data_type< ARG ... > & s, const F & f, const REST & ... res )
    {
        return pattern_matcher< T >::match(
                extract_recursive_wrapper(
                    boost::get
                    <
                        std::pair
                        <
                            boost::mpl::int_< which >,
                            typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type
                        >
                    >( s.data ).second ),
                f,
                res ... );
    }

    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const boost::recursive_wrapper< algebraic_data_type< ARG ... > > & s, const F & f, const REST & ... res )
    { return match( s.get( ), f, res ... ); }
};

template< typename EXP, typename STORE, typename F >
struct arg_loader
{
    const STORE & sec;
    const F & f;
    template< typename ... ARG >
    auto operator ( )( const ARG & ... arg ) const
    { return pattern_matcher< EXP >::match( sec, f, arg ... ); }
};

template< typename self_type, size_t which, typename L, typename R >
struct pattern_matcher< constructor_indicator< self_type, which, L, R > >
{
    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const algebraic_data_type< ARG ... > & s, const F & f, const REST & ... res )
    {
        auto p = extract_recursive_wrapper(
                boost::get
                <
                    std::pair
                    <
                        boost::mpl::int_< which >,
                        typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type
                    >
                >( s.data ).second );

        static_assert(
            std::tuple_size< typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type >::value == 2,
            "Not enough constructor argument" );
        return pattern_matcher< L >::match(
                std::get< 0 >( p ),
                arg_loader< R, decltype( std::get< 1 >( p ) ), F > { std::get< 1 >( p ), f },
                res ... );
    }

    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const boost::recursive_wrapper< algebraic_data_type< ARG ... > > & s, const F & f, const REST & ... res )
    { return match( s.get( ), f, res ... ); }
};

template< >
struct pattern_matcher<  >
{
    template< typename EXP, typename F, typename ... ARG >
    static auto match( const EXP & e, const F & f, const ARG & ... rst )
    { return f( rst ..., e ); }
};

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

template< typename EXP, typename STORE, typename F >
struct multi_arg_loader
{
    const STORE & sec;
    const F & f;
    template< bool T, typename ... ARG >
    auto helper( std::enable_if_t< T > *, const ARG & ... arg ) const
    {
        return pattern_matcher< typename boost::mpl::at_c< EXP, 0 >::type >::match(
                    std::get< 0 >( sec ),
                    multi_arg_loader< typename boost::mpl::pop_front< EXP >::type, decltype( tuple_pop( sec ) ), F > { tuple_pop( sec ), f },
                    arg ... );
    }
    template< bool T, typename ... ARG >
    auto helper( std::enable_if_t< ! T > *, const ARG & ... arg ) const
    { return f( arg ... ); }
    template< typename ... ARG >
    auto operator ( )( const ARG & ... arg ) const
    { return helper< (std::tuple_size< STORE >::value > 0), ARG ... >( nullptr, arg ... ); }
};

template< size_t nth, typename F, typename ... T, typename ... REST >
auto expand_tuple_inner( const F & f, const std::tuple< T ... > & t, const REST & ... r )
{
    return misc::make_expansion(
        [&]( const auto & t, boost::mpl::true_ ) { return f( r ..., std::get< nth >( t ) ); },
        [&]( const auto & t, boost::mpl::false_ ) { return expand_tuple_inner< nth + 1 >( f, t, r ..., std::get< nth >( t ) ); } )
            ( t, boost::mpl::bool_< std::tuple_size< std::tuple< T ... > >::value == nth + 1 >( ) );
}

template< typename self_type, size_t which, typename L, typename ... R >
struct pattern_matcher< constructor_indicator< self_type, which, L, R ... > >
{
    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const algebraic_data_type< ARG ... > & s, const F & f, const REST & ... res )
    {
        auto p = extract_recursive_wrapper(
                boost::get
                <
                    std::pair
                    <
                        boost::mpl::int_< which >,
                        typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type
                    >
                >( s.data ).second );

        static_assert(
            std::tuple_size< typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type >::value ==
            std::tuple_size< std::tuple< R ... > >::value + 1,
            "Not enough constructor argument" );
        return pattern_matcher< L >::match(
                std::get< 0 >( p ),
                multi_arg_loader< boost::mpl::vector< R ... >, decltype( tuple_pop( p ) ), F > { tuple_pop( p ), f },
                res ... );
    }

    template< typename ... ARG, typename F, typename ... REST >
    static auto match( const boost::recursive_wrapper< algebraic_data_type< ARG ... > > & s, const F & f, const REST & ... res )
    { return match( s.get( ), f, res ... ); }
};

template< typename ... match_expression > struct matcher { };

template< typename T >
struct pattern_matcher< matcher< T > >
{
    template< typename EXP, typename F, typename ... REST >
    static auto match( const EXP & exp, const F & f, const REST & ... res )
    { return pattern_matcher< T >::match( exp, f, res ... ); }
};

template< typename FST, typename ... SND >
struct pattern_matcher< matcher< FST, SND ... > >
{
    template< typename ... T, typename F, typename ... REST >
    static auto match( const algebraic_data_type< T ... > & exp, const F & f, const REST & ... res )
    { return exp.template match_pattern< FST >( ) ? pattern_matcher< FST >::match( exp, f, res ... ) : pattern_matcher< matcher< SND ... > >::match( exp, f, res ... ); }

    template< typename ... T, typename F, typename ... REST >
    static auto match( const boost::recursive_wrapper< algebraic_data_type< T ... > > & exp, const F & f, const REST & ... res )
    { return match( exp.get( ), f, res ... ); }
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

template< typename FIRST, typename ... REST >
struct pattern_tester< FIRST, REST ... >
{
    template< typename ... T >
    static bool match_pattern( const std::tuple< T ... > & t )
    { return FIRST::match_pattern( std::get< 0 >( t ) ) && pattern_tester< REST ... >::match_pattern( tuple_pop( t ) ); }
};

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
                    typename unfold_recursive< self_type, T >::type
                >
            >::type type;
        };
    };
    typedef typename boost::mpl::fold< boost::mpl::vector< TR ... >, boost::mpl::vector< >, add_pair >::type variant_arg_type;
    typename to_variant::template apply< variant_arg_type >::type data;
    algebraic_data_type( const decltype( data ) & d ) : data( d ) { }

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
        ret_type operator( )( const ARG & arg ) const { return t( typename get_constructor< ARG::first_type::value >::type( ), arg.second ); }
    };

    template< typename T >
    bool match_pattern( ) const { return pattern_tester< T >::match_pattern( * this ); }

    template< typename MATCH_EXP, typename F >
    auto match( const F & f ) const { return pattern_matcher< MATCH_EXP >::match( *this, f ); }

    template< typename FST, typename SND, typename ... MATCH_EXP, typename F >
    auto match( const F & f ) const { return pattern_matcher< matcher< FST, SND, MATCH_EXP ... > >::match( *this, f ); }
};

template< typename ... TR, typename T >
auto simple_match( const algebraic_data_type< TR ... > & adt, const T & t )
{
    typedef algebraic_data_type< TR ... > adt_type;
    typedef decltype(
        t(  std::declval< typename adt_type::template get_constructor< 0 >::type >( ),
            std::declval< typename boost::mpl::front< typename adt_type::variant_arg_type >::type::second_type >( ) ) ) ret_type;
    typename adt_type::template match_visitor< T, ret_type > smv { t };
    return adt.data.apply_visitor( smv );
}

template< typename L, typename R >
auto simple_match( const boost::recursive_wrapper< L > & l, const R & r )
{ return simple_match( l.get( ), r ); }

#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, UNUSED ) \
template< typename ... UNUSED > \
using NAME = typename ADT::get_constructor< WHICH, UNUSED ... >::type

struct unit { }; //Fuck void
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
            misc::make_expansion(
                []( const Nat & n ){ return simple_match( n, [](const auto & l, const auto &){ return l.which_constructor == 0; } ); },
                []( const unit & ){ return false; } ) ) ) );
    std::cout << "pass!" << std::endl;
    return 0;
}

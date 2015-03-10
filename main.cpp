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
//#include <boost/hana.hpp> //debian testing does not has high enough version of clang and hana is unable to detect clang in unstable. Got to wait for a while.
struct recursive_indicator { };

template< typename CONSTRUCTOR_TYPE, size_t which, typename ... PARAMETER_TYPE >
struct constructor_indicator
{
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
    template< size_t which, typename ... T >
    struct get { typedef constructor_indicator< self_type, which, T ... > type; };
};

struct unit { }; //Fuck void
typedef algebraic_data_type< unit, unit > Bool;
typedef algebraic_data_type< unit, recursive_indicator > Nat;
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, FREE_PARAMETER ) \
    template< typename ... FREE_PARAMETER > \
    using NAME = typename ADT::get< WHICH, FREE_PARAMETER ... >::type
DECLARE_CONSTRUCTOR( Bool, 0, True, T );
DECLARE_CONSTRUCTOR( Bool, 1, False, T );
DECLARE_CONSTRUCTOR( Nat, 0, O, T );
DECLARE_CONSTRUCTOR( Nat, 1, S, T );
int main( )
{
    std::declval< decltype( Nat::data ) >( );
    return 0;
}

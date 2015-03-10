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
//#include <boost/hana.hpp> //debian testing does not has high enough version of clang and hana is unable to detect clang in unstable. Got to wait for a while.
template< typename ... TR >
struct constructor_trait { typedef std::tuple< TR ... > type; };
struct self { };
struct S { };
struct O { };
struct TRUE { };
struct FALSE { };
struct nat { boost::variant< std::pair< S, std::unique_ptr< nat > >, O > v; };
struct Bool { boost::variant< TRUE, FALSE > v; };
struct recursive_indicator { };
template< typename T > struct tag { };
template< size_t which, typename ... parameter_type >
struct constructor_indicator { };
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
                    typename unfold_recursive< algebraic_data_type< TR ... >, T >::type
                >
            >::type type;
        };
    };
    typename to_variant::template apply< typename boost::mpl::fold< boost::mpl::vector< TR ... >, boost::mpl::vector< >, add_pair >::type >::type data;
    template< size_t which, typename ... T >
    struct get { typedef constructor_indicator< which, T ... > data; };
    template< size_t I, typename ... CIT, typename ... T >
    algebraic_data_type( const constructor_indicator< I, CIT ... > & CI, const T & ... t ) : data( repack< CIT ... >( t ... ) ) { }
    template< typename ... CIT, typename ... T >
    auto static repack( const T & ... t ) { }
};

int main( )
{
    std::declval< decltype( algebraic_data_type< recursive_indicator >::data ) >( );
    return 0;
}

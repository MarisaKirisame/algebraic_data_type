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
template< typename ... TR >
struct algebraic_data_type
{
    struct add_pair
    {
        template< typename F, typename T >
        struct apply
        { typedef typename boost::mpl::push_back< F, std::pair< boost::mpl::int_< boost::mpl::size< F >::value >, T > >::type type; };
    };
    typedef typename to_variant::template apply< typename boost::mpl::fold< boost::mpl::vector< TR ... >, boost::mpl::vector< >, add_pair >::type > type;
    boost::variant< TR ... > data;
    template< size_t which, typename ... T >
    struct get { typedef constructor_indicator< which, T ... > data; };
    template< size_t I, typename ... CIT, typename ... T >
    algebraic_data_type( const constructor_indicator< I, CIT ... > & CI, const T & ... t ) : data( repack< CIT ... >( t ... ) ) { }
    template< typename ... CIT, typename ... T >
    auto static repack( const T & ... t ) { }
};

int main( )
{
    typedef algebraic_data_type< size_t, int, int >::type type;
    std::declval< type >();
    return 0;
}

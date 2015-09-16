#ifndef CORE_HPP
#define CORE_HPP
namespace algebraic_data_type
{
    template< typename CONSTRUCTOR_TYPE, size_t which, typename ... TR >
    struct constructor_indicator
    {
        constexpr static size_t which_constructor = which;
        typedef CONSTRUCTOR_TYPE constructor_type;
        template< typename ARG >
        CONSTRUCTOR_TYPE operator ( )( ARG && arg ) const
        { return CONSTRUCTOR_TYPE( std::make_pair( boost::mpl::int_< which >( ), std::forward< ARG >( arg ) ) ); }
    };

    template< typename ... T >
    struct pattern_tester;

    template< typename ... T >
    struct pattern_matcher;

    template< typename ... match_expression > struct matcher { };

    struct recursive_indicator { };
    struct to_variant
    {
        struct inner
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
                    boost::mpl::apply< inner, typename boost::mpl::pop_front< T >::type, typename boost::mpl::front< T >::type, TR ... >
                >::type type;
            };
        };
        template< typename T >
        struct apply : inner::apply< T > { };
    };

    template< typename SELF_TYPE, typename T >
    struct unfold_recursive { typedef T type; }; //Open: Add more specialization to deal with different case.

    template< typename SELF_TYPE >
    struct unfold_recursive< SELF_TYPE, recursive_indicator > { typedef boost::recursive_wrapper< SELF_TYPE > type; };

    template< typename SELF_TYPE, typename ... T >
    struct unfold_recursive< SELF_TYPE, std::tuple< T ... > > { typedef std::tuple< typename unfold_recursive< SELF_TYPE, T >::type ... > type; };

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

    template< typename T >
    struct recursive_wrapper_extractor
    { T operator ( )( const T & t ) const { return t; } };

    template< typename T >
    struct recursive_wrapper_extractor< boost::recursive_wrapper< T > >
    { T operator ( )( const boost::recursive_wrapper< T > & t ) const { return t.get( ); } };

    template< typename T >
    auto extract_recursive_wrapper( const T & t ) { return recursive_wrapper_extractor< T >( )( t ); }
}
#endif // CORE_HPP

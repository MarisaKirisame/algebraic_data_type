#ifndef PATTERN_MATCHER_HPP
#define PATTERN_MATCHER_HPP
#include "core.hpp"
#include "pattern_tester.hpp"
namespace algebraic_data_type
{
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
        { return f( rst ..., extract_recursive_wrapper( e ) ); }
    };

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
                    extract_recursive_wrapper( std::get< 0 >( sec ) ),
                    multi_arg_loader< typename boost::mpl::pop_front< EXP >::type, decltype( tuple_pop( sec ) ), F > { tuple_pop( sec ), f },
                    arg ... );
        }
        template< bool T, typename ... ARG >
        auto helper( std::enable_if_t< ! T > *, const ARG & ... arg ) const { return f( arg ... ); }
        template< typename ... ARG >
        auto operator ( )( const ARG & ... arg ) const
        { return helper< (std::tuple_size< STORE >::value > 0), ARG ... >( nullptr, arg ... ); }
    };

    template< size_t nth, typename F, typename ... T, typename ... REST >
    auto expand_tuple_inner( const F & f, const std::tuple< T ... > & t, const REST & ... r )
    {
        return common::make_expansion(
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
            auto p =
                    boost::get
                    <
                        std::pair
                        <
                            boost::mpl::int_< which >,
                            typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type
                        >
                    >( s.data ).second;
            static_assert(
                std::tuple_size< typename algebraic_data_type< ARG ... >::template constructor_parameter_type< which >::type >::value ==
                std::tuple_size< std::tuple< R ... > >::value + 1,
                "Not enough constructor argument" );
            return pattern_matcher< L >::match(
                    extract_recursive_wrapper( std::get< 0 >( p ) ),
                    multi_arg_loader< boost::mpl::vector< R ... >, decltype( tuple_pop( p ) ), F > { tuple_pop( p ), f },
                    res ... );
        }
    };

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
        {
            return exp.template match_pattern< FST >( ) ?
                        pattern_matcher< FST >::match( exp, f, res ... ) :
                        pattern_matcher< matcher< SND ... > >::match( exp, f, res ... );
        }
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

}
#endif // PATTERN_MATCHER_HPP

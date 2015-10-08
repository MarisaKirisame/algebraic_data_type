#ifndef ALGEBRAIC_DATA_TYPE_HPP
#define ALGEBRAIC_DATA_TYPE_HPP
#include "pattern_tester.hpp"
#include "core.hpp"
#include "pattern_matcher.hpp"
#include <boost/preprocessor.hpp>
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, FRESH ) \
    template< typename ... FRESH > \
    auto NAME( FRESH && ... BOOST_PP_CAT( FRESH, 0 ) ) { return constructor< ADT, WHICH >( std::forward< FRESH >( BOOST_PP_CAT( FRESH, 0 ) ) ... ); }

#define DECLARE_CONSTRUCTORS_AUX( r, data, which, name ) \
    DECLARE_CONSTRUCTOR( BOOST_PP_SEQ_ELEM( 0, data ), which, name, BOOST_PP_SEQ_ELEM( 1, data ) )

#define DECLARE_CONSTRUCTORS( ADT, NAMES, FRESH ) \
    BOOST_PP_SEQ_FOR_EACH_I_R( 1, DECLARE_CONSTRUCTORS_AUX, (ADT)(FRESH), BOOST_PP_TUPLE_TO_SEQ( NAMES ) )

#define SEQ_OBJECT( SEQ ) \
    BOOST_PP_TUPLE_REM_CTOR( \
        BOOST_PP_IF( BOOST_PP_EQUAL( 1, BOOST_PP_SEQ_SIZE( SEQ ) ), ( BOOST_PP_SEQ_ELEM( 0, SEQ ) ), ( std::tuple< BOOST_PP_SEQ_ENUM( SEQ ) > ) ) )

#define DECLARE_ADT_AUX_0( data, elem ) BOOST_PP_TUPLE_ELEM( 0, elem )

#define DECLARE_ADT_AUX_1( data, elem ) \
    BOOST_PP_IF( BOOST_PP_EQUAL( BOOST_PP_TUPLE_SIZE( elem ), 1 ), (unit), \
        BOOST_PP_TUPLE_POP_FRONT( elem ) )

#define DECLARE_ADT_AUX_2( data, i, elem ) \
    BOOST_PP_COMMA_IF( BOOST_PP_GREATER( i, 0 ) ) \
    BOOST_PP_IF( BOOST_PP_EQUAL( BOOST_PP_TUPLE_SIZE( elem ), 1 ), , std::tuple< ) \
    BOOST_PP_TUPLE_ENUM( elem ) \
    BOOST_PP_IF( BOOST_PP_EQUAL( BOOST_PP_TUPLE_SIZE( elem ), 1 ), , > )

#define TUPLE_TRANSFORM_AUX( r, count, data ) \
    BOOST_PP_TUPLE_ELEM( 0, data )( BOOST_PP_TUPLE_ELEM( 1, data ), BOOST_PP_TUPLE_ELEM( count, BOOST_PP_TUPLE_ELEM( 2, data ) ) ),

#define TUPLE_TRANSFORM( op, data, tuple ) \
    ( BOOST_PP_REPEAT( BOOST_PP_DEC( BOOST_PP_TUPLE_SIZE( tuple ) ), TUPLE_TRANSFORM_AUX, ( op, data, tuple ) ) \
        op( data, BOOST_PP_TUPLE_ELEM( BOOST_PP_DEC( BOOST_PP_TUPLE_SIZE( tuple ) ), tuple ) ) )

#define TUPLE_FOREACH_I_AUX( r, count, data ) \
    BOOST_PP_TUPLE_ELEM( 0, data )( BOOST_PP_TUPLE_ELEM( 1, data ), count, BOOST_PP_TUPLE_ELEM( count, BOOST_PP_TUPLE_ELEM( 2, data ) ) )

#define TUPLE_FOREACH_I( op, data, tuple ) \
    BOOST_PP_REPEAT( BOOST_PP_TUPLE_SIZE( tuple ), TUPLE_FOREACH_I_AUX, ( op, data, tuple ) )

#define DECLARE_ADT( ADT, CONSTRUCTORS, FRESH ) \
    typedef typename \
        make_algebraic_data_type< TUPLE_FOREACH_I( DECLARE_ADT_AUX_2, _, TUPLE_TRANSFORM( DECLARE_ADT_AUX_1, _, CONSTRUCTORS ) ) >::type ADT; \
    DECLARE_CONSTRUCTORS( ADT, TUPLE_TRANSFORM( DECLARE_ADT_AUX_0, _, CONSTRUCTORS ), FRESH )

namespace algebraic_data_type
{
    typedef std::tuple< > unit; //Fuck void

    template< typename T >
    struct remove_single_tuple { typedef T type; };

    template< typename T >
    struct remove_single_tuple< std::tuple< T > > { typedef T type; };

    template< typename ... T >
    struct make_algebraic_data_type { typedef algebraic_data_type< typename remove_single_tuple< T >::type ... > type; };
}
#endif // ALGEBRAIC_DATA_TYPE_HPP

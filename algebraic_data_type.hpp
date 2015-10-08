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

#define DECLARE_ADT_AUX_0( s, data, elem ) BOOST_PP_TUPLE_ELEM( 0, elem )

#define DECLARE_ADT_AUX_1( s, data, elem ) BOOST_PP_TUPLE_POP_FRONT( elem )

#define DECLARE_ADT_AUX_2( s, data, elem ) SEQ_OBJECT( elem )

#define DECLARE_ADT( ADT, CONSTRUCTORS, FRESH ) \
    BOOST_PP_TUPLE_TO_SEQ( DECLARE_ADT_AUX_0, _, CONSTRUCTORS ) //DECLARE_CONSTRUCTORS( ADT, BOOST_PP_TUPLE_TRANSFORM( DECLARE_ADT_AUX_0, _, CONSTRUCTORS ), FRESH )

namespace algebraic_data_type { typedef std::tuple< > unit; } //Fuck void
#endif // ALGEBRAIC_DATA_TYPE_HPP

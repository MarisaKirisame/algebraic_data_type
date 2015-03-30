#ifndef ALGEBRAIC_DATA_TYPE_HPP
#define ALGEBRAIC_DATA_TYPE_HPP
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
#include "pattern_tester.hpp"
#include "core.hpp"
#include "pattern_matcher.hpp"
#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, UNUSED ) \
    template< typename ... UNUSED > \
    using NAME = typename ADT::get_constructor< WHICH, UNUSED ... >::type
    struct unit { }; //Fuck void
#endif // ALGEBRAIC_DATA_TYPE_HPP

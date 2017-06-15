//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP
#define BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/move/detail/type_traits.hpp>
#include <boost/intrusive/detail/mpl.hpp>

#include <cstddef>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {
namespace container {
namespace container_detail {

using mars_boost::move_detail::integral_constant;
using mars_boost::move_detail::true_type;
using mars_boost::move_detail::false_type;
using mars_boost::move_detail::enable_if_c;
using mars_boost::move_detail::enable_if;
using mars_boost::move_detail::enable_if_convertible;
using mars_boost::move_detail::disable_if_c;
using mars_boost::move_detail::disable_if;
using mars_boost::move_detail::disable_if_convertible;
using mars_boost::move_detail::is_convertible;
using mars_boost::move_detail::if_c;
using mars_boost::move_detail::if_;
using mars_boost::move_detail::identity;
using mars_boost::move_detail::bool_;
using mars_boost::move_detail::true_;
using mars_boost::move_detail::false_;
using mars_boost::move_detail::yes_type;
using mars_boost::move_detail::no_type;
using mars_boost::move_detail::bool_;
using mars_boost::move_detail::true_;
using mars_boost::move_detail::false_;
using mars_boost::move_detail::unvoid_ref;
using mars_boost::move_detail::and_;
using mars_boost::move_detail::or_;
using mars_boost::move_detail::not_;
using mars_boost::move_detail::enable_if_and;
using mars_boost::move_detail::disable_if_and;
using mars_boost::move_detail::enable_if_or;
using mars_boost::move_detail::disable_if_or;


template <class Pair>
struct select1st
{
   typedef Pair                        argument_type;
   typedef typename Pair::first_type   result_type;

   template<class OtherPair>
   const typename Pair::first_type& operator()(const OtherPair& x) const
   {  return x.first;   }

   const typename Pair::first_type& operator()(const typename Pair::first_type& x) const
   {  return x;   }
};

}  //namespace container_detail {
}  //namespace container {
}  //namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

#include <boost/container/detail/config_end.hpp>

#endif   //#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP


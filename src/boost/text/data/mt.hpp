// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Warning! This file is autogenerated.
#ifndef BOOST_TEXT_DATA_MT_HPP
#define BOOST_TEXT_DATA_MT_HPP

#include <boost/text/string_view.hpp>


namespace boost { namespace text { namespace data { namespace mt {

inline string_view standard_collation_tailoring()
{
    return string_view((char const *)
u8R"(
[caseFirst upper]
&[before 1]c<ċ<<<Ċ
&[before 1]g<ġ<<<Ġ
&[before 1]h<għ<<<gĦ<<<Għ<<<GĦ
&[before 1]i<ħ<<<Ħ
&[before 1]z<ż<<<Ż
  )");
}


}}}}

#endif

// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Warning! This file is autogenerated.
#ifndef BOOST_TEXT_DATA_DSB_HPP
#define BOOST_TEXT_DATA_DSB_HPP

#include <boost/text/string_view.hpp>


namespace boost { namespace text { namespace data { namespace dsb {

inline string_view standard_collation_tailoring()
{
    return string_view((char const *)
u8R"(
&C<č<<<Č<ć<<<Ć
&E<ě<<<Ě
&H<ch<<<cH<<<Ch<<<CH
&[before 1] L<ł<<<Ł
&N<ń<<<Ń
&R<ŕ<<<Ŕ
&S<š<<<Š<ś<<<Ś
&Z<ž<<<Ž<ź<<<Ź
  )");
}


}}}}

#endif

#pragma once
#include <iostream>
using namespace std;
#include "ch_tools.h"





BOOST_AUTO_TEST_SUITE(test_suite5)

BOOST_AUTO_TEST_CASE(test_case)
{ 
    ch_mkdir("log");
    ch_mkdir("log/kk");
}




BOOST_AUTO_TEST_SUITE_END()
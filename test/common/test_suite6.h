#pragma once
#include <iostream>
using namespace std;
#include <sys/time.h>
#include "ch_tools.h"
//分钟驱动
BOOST_AUTO_TEST_SUITE(test_suite6)

BOOST_AUTO_TEST_CASE(test_case)
{ 
    struct timeval mtm;
    gettimeofday(&mtm, NULL);
    int ts =  mtm.tv_sec;
    printf("%d\n",ts);
    int ts_1m = ts /60 * 60;
    printf("%d\n",ts_1m);  //

    int ts_1m_new =    ch_getts_1m();
    printf("%d\n",ts_1m_new);
}




BOOST_AUTO_TEST_SUITE_END()
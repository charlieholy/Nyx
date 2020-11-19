#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

template <typename T>
class print
{
public:
    void operator()(const T& elem){
        cout << elem << ' ';
    }
};


BOOST_AUTO_TEST_SUITE(test_suite1)

  BOOST_AUTO_TEST_CASE(test_case)
  {
      int ia[6] = {0,1,2,3,4,5};
      vector<int> iv(ia,ia+6);
      for_each(iv.begin(),iv.end(),print<int>());
  }
BOOST_AUTO_TEST_SUITE_END()
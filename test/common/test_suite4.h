#pragma once
#include <iostream>
using namespace std;
#include <vector>
#include <queue>
#include <iostream>
#include <algorithm> //heap algorithms





BOOST_AUTO_TEST_SUITE(test_suite4)

BOOST_AUTO_TEST_CASE(test_case)
{
  int ia[9] = {0,1,2,3,4,8,9,3,5};
  vector<int> ivec(ia,ia+9);
  make_heap(ivec.begin(),ivec.end());
  for(int i=0;i<ivec.size();++i)
    cout << ivec[i] << ' ';  //9 5 8 3 4 0 2 3 1
  cout << endl;   
}

BOOST_AUTO_TEST_CASE(test_case2)
{
  int ia[9] = {0,1,2,3,4,8,9,3,5};
  priority_queue<int> ipq(ia,ia+9);
  cout << "size=" << ipq.size() << endl;
  while(!ipq.empty()){
    cout << ipq.top() << ' ';
    ipq.pop();
  } 
  cout << endl;
}


BOOST_AUTO_TEST_SUITE_END()
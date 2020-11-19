#pragma once
#include <iostream>
using namespace std;
class INT
{
friend ostream& operator<<(ostream& os,const INT& i);
public:
  INT(int i) : m_i(i){};
  INT& operator++()
  {
    ++(this->m_i);
    return *this;
  }
  const INT operator++(int)
  {
    INT temp = *this;
    ++(*this);
    return temp;
  }
  INT& operator--()
  {
    --(this->m_i);
    return *this;
  }
  const INT operator--(int)
  {
    INT temp = *this;
    --(*this);
    return temp;
  }
  int& operator*() const{
    return (int&)m_i;
  }
private:
  int m_i;
};

ostream& operator << (ostream&os ,const INT& i){
  os  << '[' << i.m_i << ']';
  return os;
}

BOOST_AUTO_TEST_SUITE(test_suite2)

  BOOST_AUTO_TEST_CASE(test_case)
  {
    INT I(5);
    cout << I++;
    cout << ++I;
    cout << I--;
    cout << --I;
    cout << *I;

  }


BOOST_AUTO_TEST_SUITE_END()
#pragma once
#include <iostream>
using namespace std;
#include <new> //for placement new
#include <cstddef> //for ptrdiff_t,size_t
#include <cstdlib> //for exit()
#include <climits> //for UNIT_MAX
#include <iostream> //for cerr

namespace JJ
{
  template <class T>
  inline T* _allocate(ptrdiff_t size,T* ){
    set_new_handler(0);
    T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
    if(tmp == 0){
      cerr << "out of memory" << endl;
      exit(1);
    }
    return tmp;
  }

  template <class T>
  inline void _deallocate(T* buffer){
    ::operator delete(buffer);
  }

  template<class T1,class T2>
  inline void _construct(T1* p,const T2& value){
    new(p) T1(value);       //placement new .invoke ctor of T1
  }

  template<class T>
  inline void _destroy(T* ptr){
    ptr->~T();
  }

  template <class T>
  class allocator{
    public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    //rebind allocator of type U
    template <class U>
    struct rebind{
      typedef allocator<U> other;
    };

  pointer allocate(size_type n,const void* hint=0){
    return _allocate((difference_type)n,(pointer)0);
  }

  void deallocate(pointer p,size_type n) { _deallocate(p);}
  void construct(pointer p,const T& value){
    _construct(p,value);
  }
  void destory(pointer p){_destroy(p);}
  pointer address(reference x){return (pointer)&x;}
  const_pointer const_address(const_reference x){
    return (const_pointer)&x;
  }
  size_type max_size() const {
    return size_type(UINT_MAX/sizeof(T));
  }
};
}  //end of namaspace

#include <vector>

BOOST_AUTO_TEST_SUITE(test_suite3)

BOOST_AUTO_TEST_CASE(test_case)
{
    int ia[5] = {0,1,2,3,4};
    unsigned int i;
    vector<int,JJ::allocator<int>> iv(ia,ia+5);
    for(i=0;i<iv.size();i++)
    cout << iv[i] << ' ';
    cout << endl;
}

BOOST_AUTO_TEST_CASE(test_case2)
{
   auto_ptr<std::string> ps(new string("jjhou"));
   cout << *ps << endl;
   cout << ps->size() << endl;
}


BOOST_AUTO_TEST_SUITE_END()
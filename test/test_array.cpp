#include <iostream>
#include "dip_types.h"

template< typename T >
std::ostream& operator<<( std::ostream& os, const dip::DimensionArray< T >& array ) {
   // Here we test iterators, begin() and end().
   os << "[";
   bool first = true;
   for( auto v : array ) {
      if( !first ) { os << ", "; }
      os << v;
      first = false;
   }
   if( first ) {
      os << "--empty--]";
   } else {
      os << " (size=" << array.size() << ")]";
   }
   return os;
}

int main() {
   std::cout << "- Default initialization and push_back()\n";
   dip::DimensionArray< dip::sint > a;
   std::cout << "a = " << a << std::endl;
   a.push_back( 1 );
   std::cout << "a = " << a << std::endl;
   a.push_back( 2 );
   std::cout << "a = " << a << std::endl;
   a.push_back( 4 );
   std::cout << "a = " << a << std::endl;
   a.push_back( 8 );
   std::cout << "a = " << a << std::endl;
   a.push_back( 16 );
   std::cout << "a = " << a << std::endl;
   a.push_back( 32 );
   std::cout << "a = " << a << std::endl;

   std::cout << "- Initializer list\n";
   dip::DimensionArray< dip::sint > b{ 5, 4, 3, 2, 1 };
   std::cout << "b = " << b << std::endl;

   std::cout << "- Swap\n";
   a.swap( b );
   std::cout << "a = " << a << std::endl;
   std::cout << "b = " << b << std::endl;

   std::cout << "- pop_back()\n";
   a.pop_back();
   std::cout << "a = " << a << std::endl;
   a.pop_back();
   std::cout << "a = " << a << std::endl;
   a.pop_back();
   std::cout << "a = " << a << std::endl;
   a.pop_back();
   std::cout << "a = " << a << std::endl;
   a.pop_back();
   std::cout << "a = " << a << std::endl;
   // a.pop_back(); // one pop too many!
   // std::cout << "a = " << a << std::endl;

   std::cout << "- Standard initialization with 3 ones\n";
   dip::DimensionArray< dip::sint > c( 3, 1 );
   std::cout << "c = " << c << std::endl;

   std::cout << "- Copy constructor (copy b)\n";
   dip::DimensionArray< dip::sint > d( b );
   std::cout << "b = " << b << std::endl;
   std::cout << "d = " << d << std::endl;

   std::cout << "- Move constructor (move from c)\n";
   dip::DimensionArray< dip::sint > e( std::move( c ) );
   std::cout << "c = " << c << std::endl;
   std::cout << "e = " << e << std::endl;

   std::cout << "- Copy assignment (copy b)\n";
   e = b;
   std::cout << "b = " << b << std::endl;
   std::cout << "e = " << e << std::endl;

   std::cout << "- Move assignment (move from e)\n";
   c = std::move( e );
   std::cout << "c = " << c << std::endl;
   std::cout << "e = " << e << std::endl;

   std::cout << "- Equality\n";
   std::cout << c << ( c == d ? "==" : "!=" ) << d << std::endl;

   std::cout << "- Indexing c[3] = 0\n";
   c[ 3 ] = 0;
   std::cout << "c = " << c << std::endl;

   std::cout << "- Equality\n";
   std::cout << c << ( c == d ? "==" : "!=" ) << d << std::endl;

   std::cout << "- Move assignment (move from c)\n";
   d = std::move( c );
   std::cout << "c = " << c << std::endl;
   std::cout << "d = " << d << std::endl;

   std::cout << "- Sorting d, keep b in same order\n";
   d.sort( b );
   std::cout << "b = " << b << std::endl;
   std::cout << "d = " << d << std::endl;

   std::cout << "- insert(), erase() and clear()\n";
   b.insert( 0, 100 );
   std::cout << "b = " << b << std::endl;
   b.insert( 1, 101 );
   std::cout << "b = " << b << std::endl;
   b.erase( 0 );
   std::cout << "b = " << b << std::endl;
   b.erase( 1 );
   std::cout << "b = " << b << std::endl;
   b.clear();
   std::cout << "b = " << b << std::endl;

   std::cout << "- Destructors\n";
   std::cout << "a = " << a << std::endl;
   std::cout << "b = " << b << std::endl;
   std::cout << "c = " << c << std::endl;
   std::cout << "d = " << d << std::endl;
   std::cout << "e = " << e << std::endl;
   // (implicitly called here)

   return 0;
}

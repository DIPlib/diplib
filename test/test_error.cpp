#include <iostream>
#include "diplib/error.h"

// Testing

void test2() {
   dip_Throw("A test case");
};

void test1() {
   try {
      test2();
   }
   catch( dip::Error& e ) {
      dip_AddStackTrace( e );
      throw;
   }
}

int main() {
   try {
      test1();
      //dip_Assert( 6 > 2 );
      //dip_Assert( 6 < 2 );
   }
   catch( dip::Error& e ) {
      dip_AddStackTrace( e );
      throw;
   }
   return 0;
}

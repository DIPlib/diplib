#include <iostream>
#include "diplib.h"

// Testing

#include "dip_overload.h"
#include <typeinfo>

int main() {
   DIP_DECLARE_OPTIONS(MyOptions, 5);
   DIP_DEFINE_OPTION(MyOptions, Option_clean,  0);
   DIP_DEFINE_OPTION(MyOptions, Option_fresh,  1);
   DIP_DEFINE_OPTION(MyOptions, Option_shine,  2);
   DIP_DEFINE_OPTION(MyOptions, Option_flower, 3);
   DIP_DEFINE_OPTION(MyOptions, Option_burn,   4);

   MyOptions opts = {};
   std::cout << "{}: "        << (opts & Option_clean  ? "clean, " : "not clean, ")
                              << (opts & Option_fresh  ? "fresh, " : "not fresh, ")
                              << (opts & Option_shine  ? "shine, " : "not shine, ")
                              << (opts & Option_flower ? "flower, " : "not flower, ")
                              << (opts & Option_burn   ? "burn." : "don't burn.")
                              << std::endl;

   opts = Option_fresh;
   std::cout << "Options_fresh: "
                              << (opts & Option_clean  ? "clean, " : "not clean, ")
                              << (opts & Option_fresh  ? "fresh, " : "not fresh, ")
                              << (opts & Option_shine  ? "shine, " : "not shine, ")
                              << (opts & Option_flower ? "flower, " : "not flower, ")
                              << (opts & Option_burn   ? "burn." : "don't burn.")
                              << std::endl;

   opts = Option_clean | Option_burn;
   std::cout << "Options_clean | Options_burn: "
                              << (opts & Option_clean  ? "clean, " : "not clean, ")
                              << (opts & Option_fresh  ? "fresh, " : "not fresh, ")
                              << (opts & Option_shine  ? "shine, " : "not shine, ")
                              << (opts & Option_flower ? "flower, " : "not flower, ")
                              << (opts & Option_burn   ? "burn." : "don't burn.")
                              << std::endl;

   DIP_DECLARE_OPTIONS(HisOptions, 3);
   DIP_DEFINE_OPTION(HisOptions, Option_ugly, 0);
   DIP_DEFINE_OPTION(HisOptions, Option_cheap, 1);
   DIP_DEFINE_OPTION(HisOptions, Option_fast, 1);

   HisOptions bla = Option_cheap | Option_fast;

   // HisOptions a = Option_shine; // compiler error

   // HisOptions b = Option_fast | Option_flower; // compiler error

   return 0;
}

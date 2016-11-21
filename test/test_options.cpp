#include <iostream>
#include "diplib/types.h"

// Testing

int main() {
   DIP_DECLARE_OPTIONS( MyOptions, 5 );
   DIP_DEFINE_OPTION( MyOptions, Option_clean, 0 );
   DIP_DEFINE_OPTION( MyOptions, Option_fresh, 1 );
   DIP_DEFINE_OPTION( MyOptions, Option_shine, 2 );
   DIP_DEFINE_OPTION( MyOptions, Option_flower, 3 );
   DIP_DEFINE_OPTION( MyOptions, Option_burn, 4 );
   DIP_DEFINE_OPTION( MyOptions, Option_freshNclean, Option_fresh + Option_clean );

   MyOptions opts {};
   std::cout << "{}: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   opts = Option_fresh;
   std::cout << "Options_fresh: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   opts = Option_clean + Option_burn;
   std::cout << "Options_clean + Options_burn: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   opts += Option_shine;
   std::cout << "Options_clean + Options_burn + Option_shine: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   opts = Option_freshNclean;
   std::cout << "Option_freshNclean: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   opts -= Option_clean ;
   std::cout << "Option_freshNclean - Option_clean: "
             << ( opts == Option_clean  ? "clean, "  : "not clean, "  )
             << ( opts == Option_fresh  ? "fresh, "  : "not fresh, "  )
             << ( opts == Option_shine  ? "shine, "  : "not shine, "  )
             << ( opts == Option_flower ? "flower, " : "not flower, " )
             << ( opts == Option_burn   ? "burn."    : "don't burn."  )
             << std::endl;

   DIP_DECLARE_OPTIONS( HisOptions, 3 );
   DIP_DEFINE_OPTION( HisOptions, Option_ugly, 0 );
   DIP_DEFINE_OPTION( HisOptions, Option_cheap, 1 );
   DIP_DEFINE_OPTION( HisOptions, Option_fast, 1 );  // repeated value

   HisOptions bla {};
   std::cout << "{}: "
             << ( bla == Option_ugly   ? "ugly, "  : "not ugly, " )
             << ( bla == Option_cheap  ? "cheap, " : "not cheap, " )
             << ( bla == Option_fast   ? "fast, "  : "not fast." )
             << std::endl;

   bla = Option_cheap;
   std::cout << "Option_cheap: "
             << ( bla == Option_ugly   ? "ugly, "  : "not ugly, " )
             << ( bla == Option_cheap  ? "cheap, " : "not cheap, " )
             << ( bla == Option_fast   ? "fast, "  : "not fast." )
             << " (note that cheap==fast)"
             << std::endl;

   bla = Option_cheap + Option_fast;
   std::cout << "Option_cheap + Option_fast: "
             << ( bla == Option_ugly   ? "ugly, "  : "not ugly, " )
             << ( bla == Option_cheap  ? "cheap, " : "not cheap, " )
             << ( bla == Option_fast   ? "fast, "  : "not fast." )
             << std::endl;

   // HisOptions a = Option_shine; // compiler error: assignment different types

   // HisOptions b = Option_fast + Option_flower; // compiler error: addition different types

   return 0;
}

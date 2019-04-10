#include <iostream>
#include "libics.hpp"

int main(int argc, const char* argv[]) {

   constexpr char const* token1 = "sequence1cpp";
   constexpr char const* token2 = "sequence2cpp";
   constexpr char const* stuff1 = "this is some data";
   constexpr char const* stuff2 = "this is some more data";
   constexpr char const* stuff3 = "this is some other stuff";

   if (argc != 2) {
      std::cerr << "One file names required\n";
      exit(-1);
   }

   try {

      // Open image for update
      ics::ICS ip(argv[1], "rw");

      // Remove history lines
      ip.DeleteHistory("testcpp");   // delete history line added by test_metadata.cpp

      // Add history lines
      ip.AddHistoryString(token1, stuff1);
      ip.AddHistoryString(token1, stuff2);
      ip.AddHistoryString(token2, stuff3);

      // Check
      if (ip.GetNumHistoryStrings() != 3) {
         std::cerr << "Number of history lines not correct.\n";
         exit(-1);
      }

      // Read history lines and compare
      auto it = ip.NewHistoryIterator();
      auto pair = it.KeyValue();
      if (pair.key != token1 || pair.value != stuff1) {
         std::cerr << "1st history string does not match: \"" << pair.key << '/' << pair.value
                   << "\" vs \"" << token1 << '/' << stuff1 << "\"\n";
         exit(-1);
      }
      pair = it.KeyValue();
      if (pair.key != token1 || pair.value != stuff2) {
         std::cerr << "2nd history string does not match: \"" << pair.key << '/' << pair.value
                   << "\" vs \"" << token1 << '/' << stuff2 << "\"\n";
         exit(-1);
      }
      pair = it.KeyValue();
      if (pair.key != token2 || pair.value != stuff3) {
         std::cerr << "3rd history string does not match: \"" << pair.key << '/' << pair.value
                   << "\" vs \"" << token2 << '/' << stuff3 << "\"\n";
         exit(-1);
      }

      // Check earlier deleted line
      it = ip.NewHistoryIterator("testcpp");
      if (!it.String().empty()) {
         std::cerr << "Did not properly delete original 'testcpp' line.\n";
         exit(-1);
      }

      // Read token2 line
      it = ip.NewHistoryIterator(token2);
      pair = it.KeyValue();
      if (pair.key != token2 || pair.value != stuff3) {
         std::cerr << "history string does not match.\n";
         exit(-1);
      }

      // Commit changes
      ip.Close();

   } catch (std::exception const& e) {
      std::cerr << "Exception thrown in libics: " << e.what() << '\n';
      exit(-1);
   }
}

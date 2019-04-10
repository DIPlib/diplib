#include <iostream>
#include <memory>
#include <cstdint>
#include <cstring>
#include "libics.hpp"

int main(int argc, const char* argv[]) {

   if (argc != 2) {
      std::cerr << "One file name required\n";
      exit(-1);
   }

   try {

      // Open image for update
      ics::ICS ip(argv[1], "rw");
      auto layout = ip.GetLayout();
      std::size_t bufsize = ip.GetDataSize();
      std::unique_ptr<std::uint8_t[]> buf1{new std::uint8_t[bufsize]};
      ip.GetData(buf1.get(), bufsize);

      // Add and change metadata
      ip.SetPosition(0, {5.7, 1.3, "m"});
      ip.SetPosition(1, {-8.2, 1.2, "meter"});
      ip.DeleteHistory();
      ip.AddHistoryString("testcpp", "Adding history line.");

      // Commit changes
      ip.Close();

      // Read image
      ip.Open(argv[1], "r");

      // Check metadata
      auto units = ip.GetPosition(0);
      if (units.origin != 5.7 || units.scale != 1.3 || units.units != "m") {
         std::cerr << "Different position metadata read back\n";
         exit(-1);
      }
      units = ip.GetPosition(1);
      if (units.origin != -8.2 || units.scale != 1.2 || units.units != "meter") {
         std::cerr << "Different position metadata read back\n";
         exit(-1);
      }
      auto it = ip.NewHistoryIterator();
      auto pair = it.KeyValue();
      if (pair.key != "testcpp" || pair.value != "Adding history line.") {
         std::cerr << "Different history key/value pair read back\n";
         exit(-1);
      }

      // Check pixel data
      if (bufsize != ip.GetDataSize()) {
         std::cerr << "Data in output file not same size as written.\n";
         exit(-1);
      }
      std::unique_ptr<std::uint8_t[]> buf2{new std::uint8_t[bufsize]};
      ip.GetData(buf2.get(), bufsize);
      ip.Close();
      if (memcmp(buf1.get(), buf2.get(), bufsize) != 0) {
         std::cerr << "Data in output file does not match data in input.\n";
         exit(-1);
      }

   } catch (std::exception const& e) {
      std::cerr << "Exception thrown in libics: " << e.what() << '\n';
      exit(-1);
   }
}

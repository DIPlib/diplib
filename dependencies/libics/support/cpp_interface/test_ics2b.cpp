#include <iostream>
#include <memory>
#include <cstdint>
#include <cstring>
#include "libics.hpp"

int main(int argc, const char* argv[]) {

   if (argc != 3) {
      std::cerr << "Two file names required: in out\n";
      exit(-1);
   }

   try {

      // Read image
      ics::ICS ip(argv[1], "r");
      auto layout = ip.GetLayout();
      std::size_t bufsize = ip.GetDataSize();
      std::unique_ptr<std::uint8_t[]> buf1{new std::uint8_t[bufsize]};
      ip.GetData(buf1.get(), bufsize);
      ip.Close();

      // Write image
      ip.Open(argv[2], "w2");
      ip.SetLayout(layout.dataType, layout.dimensions);
      ip.SetData(buf1.get(), bufsize);
      ip.SetCompression(ics::Compression::Uncompressed, 0);
      ip.Close();

      // Read image
      ip.Open(argv[2], "r");
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

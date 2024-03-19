# (c)2015-2022, Cris Luengo
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # also matches "AppleClang"
   # Compiler flags for Clang C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion -pedantic -Wno-c++17-extensions")
   # Silence warnings of unused -L linker flag while compiling
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage.
   set(CMAKE_C_FLAGS_SANITIZE "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
   set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   # Compiler flags for GNU C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -pedantic")
   # "#pragma omp" causes a warning if OpenMP is not enabled.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
   # "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   # "DIP_EXPORT" in forward class declaration sometimes causes a warning in GCC 6.0 and 7.0.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage; to see which flags are enabled: gcc -march=native -Q --help=target
   set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og") # Does some optimization that doesn't impact debugging.
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og") # Does some optimization that doesn't impact debugging.
   set(CMAKE_C_FLAGS_SANITIZE "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
   set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
   # Compiler flags for Intel C++
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   # Compiler flags for Visual Studio C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS) # Disable unchecked iterator warnings and unsafe (string manipulation) function warnings
   add_definitions(/wd4180) # Disable "qualifier applied to function type has no meaning; ignored" that happens in union_find.h as used in maxima.cpp
   add_definitions(/EHa) # This exception handling model apparently is what MATLAB uses (default is /EHsc)
   add_definitions(/MP) # Enable Multi-Processor compilation
endif()

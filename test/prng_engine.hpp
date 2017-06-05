//  Copyright (c) 2012-2016 M.A. (Thijs) van den Berg, http://sitmo.com/
//
//  Use, modification and distribution are subject to the MIT Software License.
//
//  The MIT License (MIT)
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

// version history:
// version 1,  6 Sep 2012
// version 2, 10 Dec 2013
//      bug fix in the discard() routine, it was discarding to many elements
//      added the version() method
// version 3...5, 13 Dec 2013
//      fixed type-conversion earning
//      fixed potential issues with constructor template matching
// version 6, 4 March 2016
//      made min() max() constexpr for C+11 compiler (thanks to James Joseph Balamuta)

#ifndef SITMO_PRNG_ENGINE_HPP
#define SITMO_PRNG_ENGINE_HPP
#include <iostream>

#ifdef __GNUC__
    #include <stdint.h>                 // respecting the C99 standard.
#endif
#ifdef _MSC_VER
    typedef unsigned __int64 uint64_t;  // Visual Studio 6.0(VC6) and newer..
    typedef unsigned __int32 uint32_t;
#endif

// Double mixing function
#define MIX2(x0,x1,rx,z0,z1,rz) \
    x0 += x1; \
    z0 += z1; \
    x1 = (x1 << rx) | (x1 >> (64-rx)); \
    z1 = (z1 << rz) | (z1 >> (64-rz)); \
    x1 ^= x0; \
    z1 ^= z0;


// Double mixing function with key adition
#define MIXK(x0,x1,rx,z0,z1,rz,k0,k1,l0,l1) \
    x1 += k1; \
    z1 += l1; \
    x0 += x1+k0; \
    z0 += z1+l0; \
    x1 = (x1 << rx) | (x1 >> (64-rx)); \
    z1 = (z1 << rz) | (z1 >> (64-rz)); \
    x1 ^= x0; \
    z1 ^= z0; \


namespace sitmo {

    // enable_if for C__98 compilers
    template<bool C, typename T = void>
    struct sitmo_enable_if { typedef T type; };

    template<typename T>
    struct sitmo_enable_if<false, T> { };

    // SFINAE check for the existence of a "void generate(int*,int*)"member function
    template<typename T>
    struct has_generate_template
    {
        typedef char (&Two)[2];;
        template<typename F, void (F::*)(int *, int *)> struct helper {};
        template<typename C> static char test(helper<C, &C::template generate<int*> >*);
        template<typename C> static Two test(...);
        static bool const value = sizeof(test<T>(0)) == sizeof(char);
    };


class prng_engine
{
public:
    // "req" are requirements as stated in the C++ 11 draft n3242=11-0012
    //
    // req: 26.5.1.3 Uniform random number generator requirements, p.906, table 116, row 1
    typedef uint32_t result_type;

    // req: 26.5.1.3 Uniform random number generator requirements, p.906, table 116, row 3 & 4
#if __cplusplus <= 199711L
    static result_type (min)() { return 0; }
    static result_type (max)() { return 0xFFFFFFFF; }
#else
    static constexpr result_type (min)() { return 0; }
    static constexpr result_type (max)() { return 0xFFFFFFFF; }
#endif

    // -------------------------------------------------
    // Constructors
    // -------------------------------------------------

    // req: 26.5.1.4 Random number engine requirements, p.907 table 117, row 1
    // Creates an engine with the same initial state as all other
    // default-constructed engines of type E.
    prng_engine()
    {
        seed();
    }

    // req: 26.5.1.4 Random number engine requirements, p.907 table 117, row 2
    // Creates an engine that compares equal to x.
    prng_engine(const prng_engine& x)
    {
        for (unsigned short i=0; i<4; ++i) {
            _s[i] = x._s[i];
            _k[i] = x._k[i];
            _o[i] = x._o[i];
        }
        _o_counter = x._o_counter;
    }


    // req: 26.5.1.4 Random number engine requirements, p.907 table 117, row 3
    // Creates an engine with initial O(size of state) state determined by s.
    prng_engine(uint32_t s)
    {
        seed(s);
    }

        // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 4
    // Creates an engine with an initial state that depends on a sequence
    // produced by one call to q.generate.
    template<class Seq>
    prng_engine(Seq& q, typename sitmo_enable_if< has_generate_template<Seq>::value >::type* = 0 )
    {
        seed(q);
    }

    // -------------------------------------------------
    // Seeding
    // -------------------------------------------------

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 5
    void seed()
    {
        for (unsigned short i=0; i<4; ++i) {
            _k[i] = 0;
            _s[i] = 0;
        }
        _o_counter = 0;

        _o[0] = 0x09218ebde6c85537;
        _o[1] = 0x55941f5266d86105;
        _o[2] = 0x4bd25e16282434dc;
        _o[3] = 0xee29ec846bd2e40b;
    }

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 6
    // s needs to be of return_type, which is uint32_t
    void seed(uint32_t s)
    {
        for (unsigned short i=0; i<4; ++i) {
            _k[i] = 0;
            _s[i] = 0;
        }
        _k[0] = s;
        _o_counter = 0;

        encrypt_counter();
    }

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 7
    template<class Seq>
    void seed(Seq& q, typename sitmo_enable_if< has_generate_template<Seq>::value >::type* = 0 )
    {
        typename Seq::result_type w[8];
        q.generate(&w[0], &w[8]);

        for (unsigned short i=0; i<4; ++i) {
            _k[i] = ( static_cast<uint64_t>(w[2*i]) << 32) | w[2*i+1];
            _s[i] = 0;
        }
        _o_counter = 0;

        encrypt_counter();
    }

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 8
    // Advances e’s state ei to ei+1 = TA(ei) and returns GA(ei).
    uint32_t operator()()
    {
        // can we return a value from the current block?
        if (_o_counter < 8) {
            unsigned short _o_index = _o_counter >> 1;
            _o_counter++;
            if (_o_counter&1)
                return _o[_o_index] & 0xFFFFFFFF;
            else
                return _o[_o_index] >> 32;
        }

        // generate a new block and return the first 32 bits
        inc_counter();
        encrypt_counter();
        _o_counter = 1; // the next call
        return _o[0] & 0xFFFFFFFF;   // this call
    }

    // -------------------------------------------------
    // misc
    // -------------------------------------------------

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 9
    // Advances e’s state ei to ei+z by any means equivalent to z
    // consecutive calls e().
    void discard(uint64_t z)
    {
        // check if we stay in the current block
        if (z < 8 - _o_counter) {
            _o_counter += static_cast<unsigned short>(z);
            return;
        }

        // we will have to generate a new block...
        z -= (8 - _o_counter);  // discard the remainder of the current blok
        _o_counter = z % 8;     // set the pointer in the correct element in the new block
        z -= _o_counter;        // update z
        z >>= 3;                // the number of buffers is elements/8
        ++z;                    // and one more because we crossed the buffer line
        inc_counter(z);
        encrypt_counter();
    }

   // -------------------------------------------------
   // IO
   // -------------------------------------------------
   template<class CharT, class Traits>
   friend std::basic_ostream<CharT,Traits>&
   operator<<(std::basic_ostream<CharT,Traits>& os, const prng_engine& s) {
       for (unsigned short i=0; i<4; ++i)
           os << s._k[i] << ' ' << s._s[i] << ' ' << s._o[i] << ' ';
       os << s._o_counter;
       return os;
   }

   template<class CharT, class Traits>
   friend std::basic_istream<CharT,Traits>&
   operator>>(std::basic_istream<CharT,Traits>& is, prng_engine& s) {
       for (unsigned short i=0; i<4; ++i)
           is >> s._k[i] >> s._s[i] >> s._o[i];
       is >> s._o_counter;
       return is;
   }
    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 10
    // This operator is an equivalence relation. With Sx and Sy as the infinite
    // sequences of values that would be generated by repeated future calls to
    // x() and y(), respectively, returns true if Sx = Sy; else returns false.
    bool operator==(const prng_engine& y)
    {
        if (_o_counter != y._o_counter) return false;
        for (unsigned short i=0; i<4; ++i) {
            if (_s[i] != y._s[i]) return false;
            if (_k[i] != y._k[i]) return false;
            if (_o[i] != y._o[i]) return false;
        }
        return true;
    }

    // req: 26.5.1.4 Random number engine requirements, p.908 table 117, row 11
    bool operator!=(const prng_engine& y)
    {
        return !(*this == y);
    }

    // Extra function to set the key
    void set_key(uint64_t k0=0, uint64_t k1=0, uint64_t k2=0, uint64_t k3=0)
    {
        _k[0] = k0; _k[1] = k1; _k[2] = k2; _k[3] = k3;
        encrypt_counter();
    }

    // set the counter
    void set_counter(uint64_t s0=0, uint64_t s1=0, uint64_t s2=0, uint64_t s3=0, unsigned short o_counter=0)
    {
        _s[0] = s0;
        _s[1] = s1;
        _s[2] = s2;
        _s[3] = s3;
        _o_counter = o_counter % 8;
        encrypt_counter();
    }


    // versioning
    uint32_t version()
    {
        return 5;
    }

private:
    void encrypt_counter()
    {
        uint64_t b[4];
        uint64_t k[5];

        for (unsigned short i=0; i<4; ++i) b[i] = _s[i];
        for (unsigned short i=0; i<4; ++i) k[i] = _k[i];

        k[4] = 0x1BD11BDAA9FC1A22 ^ k[0] ^ k[1] ^ k[2] ^ k[3];

        MIXK(b[0], b[1], 14,   b[2], b[3], 16,   k[0], k[1], k[2], k[3]);
        MIX2(b[0], b[3], 52,   b[2], b[1], 57);
        MIX2(b[0], b[1], 23,   b[2], b[3], 40);
        MIX2(b[0], b[3],  5,   b[2], b[1], 37);
        MIXK(b[0], b[1], 25,   b[2], b[3], 33,   k[1], k[2], k[3], k[4]+1);
        MIX2(b[0], b[3], 46,   b[2], b[1], 12);
        MIX2(b[0], b[1], 58,   b[2], b[3], 22);
        MIX2(b[0], b[3], 32,   b[2], b[1], 32);

        MIXK(b[0], b[1], 14,   b[2], b[3], 16,   k[2], k[3], k[4], k[0]+2);
        MIX2(b[0], b[3], 52,   b[2], b[1], 57);
        MIX2(b[0], b[1], 23,   b[2], b[3], 40);
        MIX2(b[0], b[3],  5,   b[2], b[1], 37);
        MIXK(b[0], b[1], 25,   b[2], b[3], 33,   k[3], k[4], k[0], k[1]+3);

        MIX2(b[0], b[3], 46,   b[2], b[1], 12);
        MIX2(b[0], b[1], 58,   b[2], b[3], 22);
        MIX2(b[0], b[3], 32,   b[2], b[1], 32);

        MIXK(b[0], b[1], 14,   b[2], b[3], 16,   k[4], k[0], k[1], k[2]+4);
        MIX2(b[0], b[3], 52,   b[2], b[1], 57);
        MIX2(b[0], b[1], 23,   b[2], b[3], 40);
        MIX2(b[0], b[3],  5,   b[2], b[1], 37);

        for (unsigned int i=0; i<4; ++i) _o[i] = b[i] + k[i];
        _o[3] += 5;
    }

    void inc_counter()
    {
        ++_s[0];
        if (_s[0] != 0) return;

        ++_s[1];
        if (_s[1] != 0) return;

        ++_s[2];
        if (_s[2] != 0) return;

        ++_s[3];
    }

    void inc_counter(uint64_t z)
    {
        if (z > 0xFFFFFFFFFFFFFFFF - _s[0]) {   // check if we will overflow the first 64 bit int
            ++_s[1];
            if (_s[1] == 0) {
                ++_s[2];
                if (_s[2] == 0) {
                    ++_s[3];
                }
            }
        }
        _s[0] += z;
    }

private:
    uint64_t _k[4];             // key
    uint64_t _s[4];             // state (counter)
    uint64_t _o[4];             // cipher output    4 * 64 bit = 256 bit output
    unsigned short _o_counter;  // output chunk counter, the 256 random bits in _o
                                // are returned in eight 32 bit chunks
};


} // namespace sitmo

#undef MIXK
#undef MIX2

#endif

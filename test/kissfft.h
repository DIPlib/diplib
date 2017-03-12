/*
 * Copyright (c) 2003-2010 Mark Borgerding
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *     following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *     the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the author nor the names of any contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 */

/*
 * Modifications by Cris Luengo, 2016-10-04
 * - Simplifications:
 *    - removed `kissfft_utils::traits` class,
 *    - storing a single copy of the `_twiddles` array,
 *    - declaring local variables when they're first initialized,
 *    - not afraid of directly indexing into the std::vector class,
 *    - removed inline functions that were preprocessor macros in the C code,
 *    - using `size_t` for all indices and array sizes.
 * - Added `nfft()` and `inverse()` functions.
 */

#ifndef KISSFFT_CLASS_HH

#include <complex>
#include <vector>
#include <cassert>

template<typename T_Scalar>
class kissfft {

public:

    typedef T_Scalar scalar_type;
    typedef std::complex<scalar_type> cpx_type;

    kissfft(size_t nfft, bool inverse) : _nfft(nfft), _inverse(inverse) {
        fill_twiddles();
        //factorize: start factoring out 4's, then 2's, then 3,5,7,9,...
        size_t n = _nfft;
        size_t p = 4;
        do {
            while (n % p) {
                switch (p) {
                    case 4: p = 2; break;
                    case 2: p = 3; break;
                    default: p += 2; break;
                }
                if (p * p > n)
                    p = n; // no more factors
            }
            n /= p;
            _stageRadix.push_back(p);
            _stageRemainder.push_back(n);
        } while (n > 1);
    }

    void transform(cpx_type const* src, cpx_type *dst, size_t in_stride = 1) const {
        kf_work(0, dst, src, 1, in_stride);
    }

    size_t nfft() const { return _nfft; }
    bool inverse() const { return _inverse; }

private:

    void kf_work(
            int stage,
            cpx_type * Fout,
            cpx_type const* f,
            size_t fstride,
            size_t in_stride
    ) const {
        size_t p = _stageRadix[stage];
        size_t m = _stageRemainder[stage];
        cpx_type *Fout_beg = Fout;
        cpx_type *Fout_end = Fout + p * m;

        if (m == 1) {
            do {
                *Fout = *f;
                f += fstride * in_stride;
            } while (++Fout != Fout_end);
        } else {
            do {
                // recursive call:
                // DFT of size m*p performed by doing
                // p instances of smaller DFTs of size m,
                // each one takes a decimated version of the input
                kf_work(stage + 1, Fout, f, fstride * p, in_stride);
                f += fstride * in_stride;
            } while ((Fout += m) != Fout_end);
        }

        Fout = Fout_beg;

        // recombine the p smaller DFTs
        switch (p) {
            case 2: kf_bfly2(Fout, fstride, m); break;
            case 3: kf_bfly3(Fout, fstride, m); break;
            case 4: kf_bfly4(Fout, fstride, m); break;
            case 5: kf_bfly5(Fout, fstride, m); break;
            default: kf_bfly_generic(Fout, fstride, m, p); break;
        }
    }

    void kf_bfly2(
            cpx_type *Fout,
            size_t const fstride,
            size_t const m
    ) const {
        for (size_t k = 0; k < m; ++k) {
            cpx_type t = Fout[m + k] * _twiddles[k * fstride];
            Fout[m + k] = Fout[k] - t;
            Fout[k] += t;
        }
    }

    void kf_bfly4(
            cpx_type *Fout,
            size_t const fstride,
            size_t const m
    ) const {
        cpx_type scratch[7];
        int const negative_if_inverse = _inverse * -2 + 1;
        for (size_t k = 0; k < m; ++k) {
            scratch[0] = Fout[k + m] * _twiddles[k * fstride];
            scratch[1] = Fout[k + 2 * m] * _twiddles[k * fstride * 2];
            scratch[2] = Fout[k + 3 * m] * _twiddles[k * fstride * 3];
            scratch[5] = Fout[k] - scratch[1];

            Fout[k] += scratch[1];
            scratch[3] = scratch[0] + scratch[2];
            scratch[4] = scratch[0] - scratch[2];
            scratch[4] = cpx_type(scratch[4].imag() * negative_if_inverse, -scratch[4].real() * negative_if_inverse);

            Fout[k + 2 * m] = Fout[k] - scratch[3];
            Fout[k] += scratch[3];
            Fout[k + m] = scratch[5] + scratch[4];
            Fout[k + 3 * m] = scratch[5] - scratch[4];
        }
    }

    void kf_bfly3(
            cpx_type *Fout,
            size_t const fstride,
            size_t const m
    ) const {
        size_t const m2 = 2 * m;
        cpx_type const epi3 = _twiddles[fstride * m];
        cpx_type const* tw1 = _twiddles.data();
        cpx_type const* tw2 = _twiddles.data();

        cpx_type scratch[5];
        size_t k = m;
        do {
            scratch[1] = Fout[m] * *tw1;
            scratch[2] = Fout[m2] * *tw2;

            scratch[3] = scratch[1] + scratch[2];
            scratch[0] = scratch[1] - scratch[2];
            tw1 += fstride;
            tw2 += fstride * 2;

            Fout[m] = cpx_type(Fout->real() - scratch[3].real() * .5, Fout->imag() - scratch[3].imag() * .5);

            scratch[0] *= epi3.imag();

            *Fout += scratch[3];

            Fout[m2] = cpx_type(Fout[m].real() + scratch[0].imag(), Fout[m].imag() - scratch[0].real());

            Fout[m] += cpx_type(-scratch[0].imag(), scratch[0].real());
            ++Fout;
        } while (--k);
    }

    void kf_bfly5(
            cpx_type *Fout,
            size_t const fstride,
            size_t const m
    ) const {
        cpx_type ya = _twiddles[fstride * m];
        cpx_type yb = _twiddles[fstride * 2 * m];

        cpx_type *Fout0 = Fout;
        cpx_type *Fout1 = Fout0 + m;
        cpx_type *Fout2 = Fout0 + 2 * m;
        cpx_type *Fout3 = Fout0 + 3 * m;
        cpx_type *Fout4 = Fout0 + 4 * m;

        cpx_type scratch[13];
        for (size_t u = 0; u < m; ++u) {
            scratch[0] = *Fout0;

            scratch[1] = *Fout1 * _twiddles[u * fstride];
            scratch[2] = *Fout2 * _twiddles[2 * u * fstride];
            scratch[3] = *Fout3 * _twiddles[3 * u * fstride];
            scratch[4] = *Fout4 * _twiddles[4 * u * fstride];

            scratch[7] = scratch[1] + scratch[4];
            scratch[10] = scratch[1] - scratch[4];
            scratch[8] = scratch[2] + scratch[3];
            scratch[9] = scratch[2] - scratch[3];

            *Fout0 += scratch[7];
            *Fout0 += scratch[8];

            scratch[5] = scratch[0] + cpx_type(
                    scratch[7].real() * ya.real() + scratch[8].real() * yb.real(),
                    scratch[7].imag() * ya.real() + scratch[8].imag() * yb.real()
            );

            scratch[6] = cpx_type(
                     scratch[10].imag() * ya.imag() + scratch[9].imag() * yb.imag(),
                    -scratch[10].real() * ya.imag() - scratch[9].real() * yb.imag()
            );

            *Fout1 = scratch[5] - scratch[6];
            *Fout4 = scratch[5] + scratch[6];

            scratch[11] = scratch[0] + cpx_type(
                    scratch[7].real() * yb.real() + scratch[8].real() * ya.real(),
                    scratch[7].imag() * yb.real() + scratch[8].imag() * ya.real()
            );

            scratch[12] = cpx_type(
                    -scratch[10].imag() * yb.imag() + scratch[9].imag() * ya.imag(),
                     scratch[10].real() * yb.imag() - scratch[9].real() * ya.imag()
            );

            *Fout2 = scratch[11] + scratch[12];
            *Fout3 = scratch[11] - scratch[12];

            ++Fout0;
            ++Fout1;
            ++Fout2;
            ++Fout3;
            ++Fout4;
        }
    }

    /* perform the butterfly for one stage of a mixed radix FFT */
    void kf_bfly_generic(
            cpx_type *Fout,
            size_t const fstride,
            size_t m,
            size_t p
    ) const {
        //cpx_type scratchbuf[p]; // CLang doesn't like this. Also, it seems like this is dynamic memory allocation? Slows stuff down???
        cpx_type scratchbuf[8*1024];
        assert(p <= 8*1024);
        for (size_t u = 0; u < m; ++u) {
            size_t k = u;
            for (size_t q1 = 0; q1 < p; ++q1) {
                scratchbuf[q1] = Fout[k];
                k += m;
            }

            k = u;
            for (size_t q1 = 0; q1 < p; ++q1) {
                size_t twidx = 0;
                Fout[k] = scratchbuf[0];
                for (size_t q = 1; q < p; ++q) {
                    twidx += fstride * k;
                    if (twidx >= _nfft) twidx -= _nfft;
                    Fout[k] += scratchbuf[q] * _twiddles[twidx];
                }
                k += m;
            }
        }
    }

    void fill_twiddles() {
        _twiddles.resize(_nfft);
        scalar_type phinc = (_inverse ? 2 : -2) * std::acos(scalar_type(-1)) / _nfft;
        for (size_t i = 0; i < _nfft; ++i)
            _twiddles[i] = std::exp(cpx_type(0, i * phinc));
    }

    size_t const _nfft;
    bool const _inverse;
    std::vector<cpx_type> _twiddles;
    std::vector<size_t> _stageRadix;
    std::vector<size_t> _stageRemainder;
};

#endif

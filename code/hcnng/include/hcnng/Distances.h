#ifndef NE_DISTANCES_H
#define NE_DISTANCES_H
#include <immintrin.h>
#include <iostream>

typedef uint64_t _u64;
typedef int64_t  _s64;
typedef uint32_t _u32;
typedef int32_t  _s32;
typedef uint16_t _u16;
typedef int16_t  _s16;
typedef uint8_t  _u8;
typedef int8_t   _s8;

namespace {
    static inline __m128 _mm_mulhi_epi8(__m128i X) {
        __m128i zero = _mm_setzero_si128();
        __m128i sign_x = _mm_cmplt_epi8(X, zero);
        __m128i xhi = _mm_unpackhi_epi8(X, sign_x);

        return _mm_cvtepi32_ps(
                _mm_add_epi32(_mm_setzero_si128(), _mm_madd_epi16(xhi, xhi)));
    }

    static inline __m128 _mm_mulhi_epi8_shift32(__m128i X) {
        __m128i zero = _mm_setzero_si128();
        X = _mm_srli_epi64(X, 32);
        __m128i sign_x = _mm_cmplt_epi8(X, zero);
        __m128i xhi = _mm_unpackhi_epi8(X, sign_x);

        return _mm_cvtepi32_ps(
                _mm_add_epi32(_mm_setzero_si128(), _mm_madd_epi16(xhi, xhi)));
    }
    static inline __m128 _mm_mul_epi8(__m128i X, __m128i Y) {
        __m128i zero = _mm_setzero_si128();

        __m128i sign_x = _mm_cmplt_epi8(X, zero);
        __m128i sign_y = _mm_cmplt_epi8(Y, zero);

        __m128i xlo = _mm_unpacklo_epi8(X, sign_x);
        __m128i xhi = _mm_unpackhi_epi8(X, sign_x);
        __m128i ylo = _mm_unpacklo_epi8(Y, sign_y);
        __m128i yhi = _mm_unpackhi_epi8(Y, sign_y);

        return _mm_cvtepi32_ps(
                _mm_add_epi32(_mm_madd_epi16(xlo, ylo), _mm_madd_epi16(xhi, yhi)));
    }
    static inline __m128 _mm_mul_epi8(__m128i X) {
        __m128i zero = _mm_setzero_si128();
        __m128i sign_x = _mm_cmplt_epi8(X, zero);
        __m128i xlo = _mm_unpacklo_epi8(X, sign_x);
        __m128i xhi = _mm_unpackhi_epi8(X, sign_x);

        return _mm_cvtepi32_ps(
                _mm_add_epi32(_mm_madd_epi16(xlo, xlo), _mm_madd_epi16(xhi, xhi)));
    }

    static inline __m128 _mm_mul32_pi8(__m128i X, __m128i Y) {
        __m128i xlo = _mm_cvtepi8_epi16(X), ylo = _mm_cvtepi8_epi16(Y);
        return _mm_cvtepi32_ps(
                _mm_unpacklo_epi32(_mm_madd_epi16(xlo, ylo), _mm_setzero_si128()));
    }

    static inline __m256 _mm256_mul_epi8(__m256i X, __m256i Y) {
        __m256i zero = _mm256_setzero_si256();

        __m256i sign_x = _mm256_cmpgt_epi8(zero, X);
        __m256i sign_y = _mm256_cmpgt_epi8(zero, Y);

        __m256i xlo = _mm256_unpacklo_epi8(X, sign_x);
        __m256i xhi = _mm256_unpackhi_epi8(X, sign_x);
        __m256i ylo = _mm256_unpacklo_epi8(Y, sign_y);
        __m256i yhi = _mm256_unpackhi_epi8(Y, sign_y);

        return _mm256_cvtepi32_ps(_mm256_add_epi32(_mm256_madd_epi16(xlo, ylo),
                                                   _mm256_madd_epi16(xhi, yhi)));
    }

    static inline __m256 _mm256_mul32_pi8(__m128i X, __m128i Y) {
        __m256i xlo = _mm256_cvtepi8_epi16(X), ylo = _mm256_cvtepi8_epi16(Y);
        return _mm256_blend_ps(_mm256_cvtepi32_ps(_mm256_madd_epi16(xlo, ylo)),
                               _mm256_setzero_ps(), 252);
    }

    static inline float _mm256_reduce_add_ps(__m256 x) {
        /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
        const __m128 x128 =
                _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
        /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
        const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
        /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
        const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
        /* Conversion to float is a no-op on x86-64 */
        return _mm_cvtss_f32(x32);
    }
}  // namespace


    //  enum Metric { L2 = 0, INNER_PRODUCT = 1, FAST_L2 = 2, PQ = 3 };
    template<typename T>
    class Distance {
    public:
        virtual float compare(const T *a, const T *b, unsigned length) const = 0;
        virtual ~Distance() {
        }
    };
    class DistanceL2 : public Distance<float> {
    public:
        float compare(const float *a, const float *b, unsigned size) const
        __attribute__((hot)) {
            a = (const float *) __builtin_assume_aligned(a, 32);
            b = (const float *) __builtin_assume_aligned(b, 32);
            float result = 0;
//#ifdef USE_AVX2
            // assume size is divisible by 8
      _u16   niters = size / 8;
      __m256 sum = _mm256_setzero_ps();
      for (_u16 j = 0; j < niters; j++) {
        // scope is a[8j:8j+7], b[8j:8j+7]
        // load a_vec
        if (j < (niters - 1)) {
          _mm_prefetch((char *) (a + 8 * (j + 1)), _MM_HINT_T0);
          _mm_prefetch((char *) (b + 8 * (j + 1)), _MM_HINT_T0);
        }
        __m256 a_vec = _mm256_load_ps(a + 8 * j);
        // load b_vec
        __m256 b_vec = _mm256_load_ps(b + 8 * j);
        // a_vec - b_vec
        __m256 tmp_vec = _mm256_sub_ps(a_vec, b_vec);
        /*
    // (a_vec - b_vec)**2
        __m256 tmp_vec2 = _mm256_mul_ps(tmp_vec, tmp_vec);
    // accumulate sum
        sum = _mm256_add_ps(sum, tmp_vec2);
    */
        // sum = (tmp_vec**2) + sum
        sum = _mm256_fmadd_ps(tmp_vec, tmp_vec, sum);
      }

      // horizontal add sum
      result = _mm256_reduce_add_ps(sum);
//#else
//#pragma omp simd reduction(+ : result) aligned(a, b : 32)
//            for (_s32 i = 0; i < (_s32) size; i++) {
//                result += (a[i] - b[i]) * (a[i] - b[i]);
//            }
//#endif
            return result;
        }
    };
    class SlowDistanceL2Float : public Distance<float> {
        virtual float compare(const float *a, const float *b,
                              unsigned length) const {
            float result = 0.0f;
            for (_u32 i = 0; i < length; i++) {
                result += (a[i] - b[i]) * (a[i] - b[i]);
            }
            return result;
        }
    };
    class AVXDistanceL2Float : public Distance<float> {
    public:
        virtual float compare(const float *a, const float *b,
                              unsigned int length) const {
            float result = 0;
#pragma omp simd reduction(+ : result) aligned(a, b : 8)
            for (_s32 i = 0; i < (_s32) length; i++) {
                result += (a[i] - b[i]) * (a[i] - b[i]);
            }
            return result;
        }
    };

enum Metric { L2 = 0, INNER_PRODUCT = 1, FAST_L2 = 2, PQ = 3 };

//bool AvxSupportedCPU = false;
//bool Avx2SupportedCPU = true;

/*
Distance<float> *get_distance_function() {
//        if (Avx2SupportedCPU) {
//            std::cout << "Using AVX2 distance computation" << std::endl;
            return new DistanceL2();
//        } else if (AvxSupportedCPU) {
//            std::cout << "AVX2 not supported. Using AVX distance computation"
//                      << std::endl;
//            return new AVXDistanceL2Float();
//        } else {
//            std::cout << "Older CPU. Using slow distance computation" << std::endl;
//            return new SlowDistanceL2Float();
//        }
}
*/
#endif //DISTANCES_H

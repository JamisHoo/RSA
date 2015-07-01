/******************************************************************************
 *  Copyright (c) 2015 Jamis Hoo
 *  Distributed under the MIT license 
 *  (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *  
 *  Project: RSA
 *  Filename: rsa.cc 
 *  Version: 1.0
 *  Author: Jamis Hoo
 *  E-mail: hoojamis@gmail.com
 *  Date: Jun 30, 2015
 *  Time: 19:28:20
 *  Description: 
 *****************************************************************************/
#include "ttmath/ttmath.h"
#include <iostream>
#include <cassert>


constexpr size_t kBITS = 1024;
constexpr size_t kBITS_A_WORD = sizeof(ttmath::uint) * 8;
constexpr size_t kWORDS = kBITS / kBITS_A_WORD;

template<size_t num_length>
ttmath::UInt<num_length> pow_mod(const ttmath::UInt<num_length>& n, 
                                 const ttmath::UInt<num_length>& pow, 
                                 const ttmath::UInt<num_length>& mod) {
    ttmath::UInt<num_length * 2> N = n, x = 1;
    ttmath::UInt<num_length * 2> Pow = pow;

    while (Pow != 0) {
        bool remainder = (Pow & 1) == 1;
        Pow /= 2;
        if (remainder)
            x *= N, x %= mod;

        N *= N, N %= mod;
    }

    N = x;
    
    return N;
}


template<size_t num_length>
bool is_prime(const ttmath::UInt<num_length>& n) {
    // std::cout << "n == " << n << std::endl;
    constexpr size_t num_trials = 10;

    if (n < 2) return false;
    if (n == 2) return true;
    
    if (n % 2 == 0) return false;

    ttmath::UInt<num_length> s = 0;
    ttmath::UInt<num_length> d = n - 1;

    ttmath::UInt<num_length> quotient, remainder;
    while (1) {
        quotient = d / 2;
        remainder = d % 2;
        if (remainder == 1) 
            break;
        ++s;
        d = quotient;
    }

    for (size_t i = 0; i < num_trials; ++i) {
        ttmath::UInt<num_length> rnd = 0;
        for (size_t j = 0; j < kBITS_A_WORD * num_length; ++j)
            rnd |= (rand() & 1) == 1, rnd <<= 1;
        rnd %= n - 3, rnd += 2;

        auto x = pow_mod(rnd, d, n);

        if (x == 1 || x == n - 1) continue;

        if (s == 1) return false;
        
        for (ttmath::UInt<num_length> j = 0; j < s - 1; ++j) {
            x = pow_mod(x, ttmath::UInt<num_length>(2), n);
            if (x == n - 1) break;
            if (j == s - 2) return false;
        }
    }
    return true;
}


template<size_t num_length>
void generate_random_prime(ttmath::UInt<num_length>& p, const uintmax_t e) {
    for (size_t i = 0; i < kBITS_A_WORD * num_length; ++i)
        p |= (rand() & 1) == 1, p <<= 1;
    
    p ^= 1;

    assert(p % 2 != 0);
    
    while (p % e == 1 || !is_prime(p)) p += 2;
}

template<size_t num_length>
ttmath::UInt<num_length> extended_gcd(ttmath::UInt<num_length> a, ttmath::UInt<num_length> b) {
    ttmath::Int<num_length + 1> s = 0, old_s = 1,
                                t = 1, old_t = 0,
                                r = b, old_r = a;
    while (r != 0) {
        ttmath::Int<num_length + 1> quotient = old_r / r;
        ttmath::Int<num_length + 1> tmp;
        
        tmp = r;
        r = old_r - quotient * r;
        old_r = tmp;

        tmp = s;
        s = old_s - quotient * s;
        old_s = tmp;

        tmp = t;
        t = old_t - quotient * t;
        old_t = tmp;
    }

    if (old_s < 0) old_s += b;
    return old_s;
}


int main() {
    srand(0);

    using namespace std;

    
    uintmax_t e = 0x10001;
    
    ttmath::UInt<kWORDS / 2> p, q;
    
    generate_random_prime(p, e);
    generate_random_prime(q, e);


    ttmath::UInt<kWORDS> n = p;
    n *= q;

    ttmath::UInt<kWORDS> phi_n = p - 1;
    phi_n *= q - 1;

    ttmath::UInt<kWORDS> d = extended_gcd<kWORDS>(e, phi_n);

    ttmath::UInt<kWORDS> message = rand();

    ttmath::UInt<kWORDS> cipher = pow_mod<kWORDS>(message, e, n);
    ttmath::UInt<kWORDS> plain = pow_mod(cipher, d, n);
    // cout << message << endl << plain << endl;
    assert(plain == message);

    ttmath::UInt<kWORDS> signature = pow_mod(message, d, n);
    plain = pow_mod<kWORDS>(signature, e, n);
    // cout << message << endl << plain << endl;
    assert(plain == message);

    cout << "p == " << p << endl;
    cout << "q == " << q << endl;
    cout << "n == " << n << endl;
    cout << "e == " << e << endl;
    cout << "d == " << d << endl;


}

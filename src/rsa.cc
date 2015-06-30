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
constexpr size_t kWORDS = kBITS / sizeof(ttmath::uint) / 8;


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


bool is_prime(const ttmath::UInt<kWORDS / 2>& n) {
    constexpr size_t num_trials = 64;

    if (n < 2) return false;
    if (n == 2) return true;
    
    if (n % 2 == 0) return false;

    ttmath::UInt<kWORDS / 2> s = 0;
    ttmath::UInt<kWORDS / 2> d = n - 1;

    ttmath::UInt<kWORDS / 2> quotient, remainder;
    while (1) {
        quotient = d / 2;
        remainder = d % 2;
        if (remainder == 1) 
            break;
        ++s;
        d = quotient;
    }

    ttmath::UInt<kWORDS / 2> tmp = 2;
    tmp.Pow(s);
    assert(tmp * d == n - 1);

    auto try_composite = [&n, &d, &s](const ttmath::UInt<kWORDS / 2>& a)->bool {
        if (pow_mod(a, d, n) == 1) return false;
        for (ttmath::UInt<kWORDS / 2> i = 0; i < s; ++i) {
            ttmath::UInt<kWORDS / 2> tmp = 2;
            tmp.Pow(i);
            if (pow_mod(a, tmp * d, n) == n - 1) return false;
        }
        return true;
    };

    for (size_t i = 0; i < num_trials; ++i) {
        ttmath::UInt<kWORDS / 2> rnd = 0;
        for (size_t j = 0; j < kBITS / 2; ++j)
            rnd |= (rand() & 1) == 1, rnd <<= 1;
        rnd %= n - 2;
        rnd += 2;

        if (try_composite(rnd)) return false;
    }

    return true;
}


void generate_random_prime(ttmath::UInt<kWORDS / 2>& p, const uintmax_t e) {
    for (size_t i = 0; i < kBITS / 2; ++i)
        p |= (rand() & 1) == 1, p <<= 1;
    
    p ^= 1;

    assert(p % 2 != 0);
    
    while (p % e == 1 || !is_prime(p)) p += 2;
}

ttmath::UInt<kWORDS> extended_gcd(ttmath::UInt<kWORDS> a, ttmath::UInt<kWORDS> b) {
    ttmath::Int<kWORDS + 1> s = 0, old_s = 1,
                            t = 1, old_t = 0,
                            r = b, old_r = a;
    while (r != 0) {
        ttmath::Int<kWORDS + 1> quotient = old_r / r;
        ttmath::Int<kWORDS + 1> tmp;
        
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

    return old_s;
}


int main() {
    using namespace std;

    uintmax_t e = 0x10001;
    
    ttmath::UInt<kWORDS / 2> p, q;
    
    generate_random_prime(p, e);
    generate_random_prime(q, e);


    ttmath::UInt<kWORDS> n = p;
    n *= q;

    ttmath::UInt<kWORDS> phi_n = p - 1;
    phi_n *= q - 1;

    ttmath::UInt<kWORDS> d = extended_gcd(e, phi_n);

    cout << p << endl;
    cout << q << endl;
    cout << n << endl;
    cout << d << endl;

    cout << "------------------------\n";

    ttmath::UInt<kWORDS> message = rand();
    ttmath::UInt<kWORDS> cipher = pow_mod<kWORDS>(message, e, n);
    ttmath::UInt<kWORDS> plain = pow_mod(cipher, d, n);
    cout << message << endl << plain << endl;
    cipher = pow_mod(message, d, n);
    plain = pow_mod<kWORDS>(cipher, e, n);
    cout << message << endl << plain << endl;

}

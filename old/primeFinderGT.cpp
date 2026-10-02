// fastsieve.cpp - fast prime counter (odd-only, bit-packed, segmented,
//                 pre-sieved for 3..13, multithreaded)
//
// Build:  g++ -O3 -march=native -pthread -std=c++20 fastsieve.cpp -o fastsieve
// Run:    ./fastsieve [N] [threads]        (default N = 1e9, threads = all)

#include <algorithm>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

using u64 = uint64_t;

constexpr u64 BLOCK_BITS   = 1ull << 18;        // 32 KB block (tune: 2^17 .. 2^20)
constexpr u64 BLOCK_WORDS  = BLOCK_BITS / 64;
constexpr u64 CHUNK_BLOCKS = 16;                // blocks handed to a thread at once
constexpr u64 PAT_WORDS    = 3 * 5 * 7 * 11 * 13; // 15015 words = pattern period

int main(int argc, char** argv) {
    const u64 N = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1'000'000'000ull;
    unsigned nthreads = argc > 2 ? std::atoi(argv[2]) : std::thread::hardware_concurrency();
    if (nthreads == 0) nthreads = 1;
    if (N < 2) { std::cout << "primes up to " << N << ": 0\n"; return 0; }

    // Setup (not timed): sieving primes 17..sqrt(N), 3..13 are handled by the pattern.
    const u64 root = (u64)std::sqrt((double)N) + 1;
    std::vector<bool> comp(root + 1, false);
    std::vector<uint32_t> primes;
    for (u64 i = 2; i <= root; i++) {
        if (comp[i]) continue;
        if (i >= 17 && i * i <= N) primes.push_back((uint32_t)i);
        for (u64 j = i * i; j <= root; j += i) comp[j] = true;
    }

    // Bit i represents the odd number 2i+1. Pattern marks multiples of 3,5,7,11,13.
    std::vector<u64> pattern(PAT_WORDS, 0);
    for (u64 i = 0; i < PAT_WORDS * 64; i++) {
        u64 v = 2 * i + 1;
        if (v % 3 == 0 || v % 5 == 0 || v % 7 == 0 || v % 11 == 0 || v % 13 == 0)
            pattern[i >> 6] |= 1ull << (i & 63);
    }

    const u64 SIZE    = (N + 1) / 2;                       // odd numbers <= N
    const u64 nblocks = (SIZE + BLOCK_BITS - 1) / BLOCK_BITS;

    auto t0 = std::chrono::high_resolution_clock::now();

    std::atomic<u64> nextChunk{0};
    std::vector<u64> partial(nthreads, 0);

    auto worker = [&](unsigned tid) {
        std::vector<u64> block(BLOCK_WORDS);
        std::vector<u64> off(primes.size());
        u64 local = 0;

        for (;;) {
            u64 b0 = nextChunk.fetch_add(1) * CHUNK_BLOCKS;
            if (b0 >= nblocks) break;
            u64 b1 = std::min(b0 + CHUNK_BLOCKS, nblocks);

            // first multiple (odd, >= p*p) of each prime, relative to block b0
            u64 lo0 = b0 * BLOCK_BITS, s = 2 * lo0 + 1;
            for (size_t k = 0; k < primes.size(); k++) {
                u64 p = primes[k], pp = p * p;
                u64 m = pp >= s ? pp : ((s + p - 1) / p) * p;
                if (!(m & 1)) m += p;
                off[k] = (m - 1) / 2 - lo0;
            }

            for (u64 b = b0; b < b1; b++) {
                u64 lo = b * BLOCK_BITS;

                // 1. copy the 3..13 pattern (wraps at most once)
                u64 q  = (lo / 64) % PAT_WORDS;
                u64 n1 = std::min(BLOCK_WORDS, PAT_WORDS - q);
                std::memcpy(block.data(), &pattern[q], n1 * 8);
                if (n1 < BLOCK_WORDS)
                    std::memcpy(block.data() + n1, pattern.data(), (BLOCK_WORDS - n1) * 8);

                // 2. the pattern marked 3,5,7,11,13 themselves: unmark; 1 is not prime
                if (b == 0) { block[0] &= ~0x6Eull; block[0] |= 1; }

                // 3. cross off multiples of the remaining small primes
                u64* w = block.data();
                for (size_t k = 0; k < primes.size(); k++) {
                    u64 p = primes[k], j = off[k];
                    for (; j < BLOCK_BITS; j += p) w[j >> 6] |= 1ull << (j & 63);
                    off[k] = j - BLOCK_BITS;
                }

                // 4. mask padding bits beyond N in the last block
                if (b == nblocks - 1) {
                    u64 valid = SIZE - lo;
                    if (valid < BLOCK_BITS) {
                        block[valid >> 6] |= ~0ull << (valid & 63);
                        for (u64 i = (valid >> 6) + 1; i < BLOCK_WORDS; i++) block[i] = ~0ull;
                    }
                }

                // 5. count survivors (zero bits)
                u64 c = 0;
                for (u64 i = 0; i < BLOCK_WORDS; i++) c += std::popcount(w[i]);
                local += BLOCK_BITS - c;
            }
        }
        partial[tid] = local;
    };

    std::vector<std::thread> pool;
    for (unsigned t = 1; t < nthreads; t++) pool.emplace_back(worker, t);
    worker(0);
    for (auto& th : pool) th.join();

    u64 total = 1; // the prime 2
    for (u64 v : partial) total += v;

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t1 - t0;

    std::cout << "primes up to " << N << ": " << total << "\n";
    std::cout << "threads: " << nthreads << "\n";
    std::cout << "Execution Time : " << ms.count() << " ms\n";
} // This One Is AI Generated, Hopefully i can Implement something at this level by myself in the future.

#include <print>
#include <vector>
#include <cmath>
#include <iostream>
#include <chrono>

int main (){

  const size_t MAX = 1000000000;
  size_t max = ceil(sqrt(MAX));
  std::vector<bool> isPrime(MAX + 1, true);

  for (size_t i = 0; i <= MAX; i++){
    isPrime[i] = i;
  }

  auto start = std::chrono::high_resolution_clock::now();
  isPrime[0] = 0;
  isPrime[1] = 0;
  for (size_t i = 2; i < max; i++) {
    for (size_t j = i + i ; j <= MAX; j = j + i ) {
      isPrime[j] = 0;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  
  for (size_t i = 0; i <= MAX; i++) {
    if (isPrime[i] == 1) {
      std::print("{} ", i);
    }
  }

  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "\nWaktu eksekusi saringan: " << duration.count() << " ms\n";
  
}


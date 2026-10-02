#include <print>
#include <vector>
#include <cmath>
#include <iostream>
#include <chrono>

int main (){

  const size_t MAXODDLIST = 50;
  size_t iterationStop = ceil(sqrt(MAXODDLIST));
  std::vector<bool> isPrime(MAXODDLIST, true);

  auto start = std::chrono::high_resolution_clock::now();

  isPrime[0] = 0;
  for (size_t i = 1; i < iterationStop; i++) {
    if ( isPrime[i] == 1 ) {
      for (size_t j = ( 3 * i + 1 ) ; j < MAXODDLIST ; j += 2 * i + 1) {
        isPrime[j] = 0;
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
 
  std::print("2 ");
  for (size_t i = 0; i < MAXODDLIST; i++) {
    if (isPrime[i] == 1) {
      std::print("{} ", i * 2 + 1);
    }
  }

  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "\nExecution Time : " << duration.count() << " ms\n";
  
}


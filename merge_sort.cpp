#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>
#include <functional>
#include <limits>

static std::vector<int> generate_data(size_t size)
{
   using value_type = int;
   static std::uniform_int_distribution<value_type> distribution(
     std::numeric_limits<value_type>::min(),
     std::numeric_limits<value_type>::max());
   std::mt19937 engine;
   auto generator = std::bind(distribution, engine);

   std::vector<value_type> data(size);
   std::generate_n(data.begin(), size, generator);
   return data;
}

void merge(std::vector<int> &inp, int left, int middle, int right){
	std::vector<int> temp(middle - left + 1); 
  std::vector<int> temp2(right - middle);
	for(auto i = 0; i < (middle - left + 1); i++){
		temp[i] = inp[left + i];
	}
	for(auto i = 0; i < (right - middle); i++){
		temp2[i] = inp[middle + 1 + i];
	}
	int i=0, j=0, k=left;
	while (i < (middle - left + 1) && j < (right - middle))
	{
		if(temp[i] <= temp2[j]){
			inp[k++] = temp[i++];
		}
		else{
			inp[k++] = temp2[j++];
		}
	}
	while (i < (middle - left + 1)){
		inp[k++] = temp[i++];
	}
	while (j< (right - middle)){
		inp[k++] = temp2[j++];
	}
}

void mergeSortSerial(std::vector<int> &inp, int left, int right){
  if (left < right){
    int middle = (left + right) / 2;
    mergeSortSerial(inp, left, middle); //call 1
    mergeSortSerial(inp, middle + 1, right); //call 2
    merge(inp, left, middle, right);
  }
}

void mergeSortImpl(std::vector<int> &inp, int left, int right){
  if ((right-left) > 10000){
    int middle = (left + right) / 2;
#pragma omp task shared(inp)
    mergeSortImpl(inp, left, middle); //call 1
#pragma omp task shared(inp)
    mergeSortImpl(inp, middle + 1, right); //call 2
#pragma omp taskwait
    merge(inp, left, middle, right);
    }
  else if (right - left > 0)
  {
    mergeSortSerial(inp, left, right);
  }
}

void mergeSortParallel(std::vector<int> &inp, int left, int right)
{
#pragma omp parallel
  {
#pragma omp single
    mergeSortImpl(inp, left, right);
  }
}

void runParallel(int n)
{
  std::vector<int> myData = generate_data(n);
  myData = generate_data(n);

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  mergeSortParallel(myData, 0, n - 1);
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << elapsed_seconds.count() << "\n";
}

void runSerial(int n)
{
  std::vector<int> myData = generate_data(n);
  myData = generate_data(n);

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  mergeSortSerial(myData, 0, n - 1);
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << elapsed_seconds.count() << "\n";
  
}

int main(){
  // runSerial(1000000);
  runParallel(1000000);
}

// g++ -o merge_sort merge_sort.cpp -fopenmp -O3
// perf stat -d ./merge_sort
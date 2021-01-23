g++ -o merge_sort merge_sort.cpp -fopenmp -O3
{ perf stat ./merge_sort ; } 2>temp.txt
cat temp.txt | sed -n '13p' >> parTime.txt

#include <vector>
#include <cmath>
#include <iostream>

#include <bitset>
#include <ctime>


void get_linear_basis(std::vector<int> arr, std::vector<int>& basis)
{
    for(int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < basis.size(); ++j)
        {
            arr[i] = std::min(arr[i], arr[i] ^ basis[j]);
        }
        if(arr[i] != 0)
        {
            basis.push_back(arr[i]);
        }
    }
}

int main()
{
    std::vector<int> arr;
    arr.reserve(20000);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < 200; ++i) {
        arr.push_back(std::rand());
    }
    std::vector<int> basis;
    get_linear_basis(arr, basis);
    std::cout << basis.size() << std::endl;
    for(int b : basis) {
        std::cout << std::bitset<32>(b) << std::endl;
    }
    return 0;
}
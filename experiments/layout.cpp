#include <iostream>
#include "include/domain/price_level.hpp"
#include "include/domain/order.hpp"

int main() {
    std::cout << "PriceLevel\n";
    std::cout << "sizeof  : " << sizeof(mercury::PriceLevel) << '\n';
    std::cout << "alignof : " << alignof(mercury::PriceLevel) << '\n';

    std::cout << "\nOrder\n";
    std::cout << "sizeof  : " << sizeof(mercury::Order) << '\n';
    std::cout << "alignof : " << alignof(mercury::Order) << '\n';
}
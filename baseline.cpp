#include <__chrono/duration.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ratio>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <chrono>

struct City {
    std::string name;
    int count;
    double sum, min, max;
    
    City() : name(""), count(0), sum(0), min(101), max(-101) {}  // Default constructor
    City(std::string city_name) {
        name = city_name;
        count = 0;
        sum = 0;
        max = -101;
        min = 101;
    }
};


int
main(int argc, const char **argv) {
    const char *filename = "measurements.txt";
    if(argc > 1) {
        filename = argv[1];
    }
    
    std::vector<City> city_arr(500);
    std::unordered_map<std::string, int> mp;
    int cur = 0;

    std::string line;
    std::ifstream file(filename);

    auto start_time = std::chrono::high_resolution_clock::now(); 

    if(file.is_open()) {
        while(getline(file, line)) {
            std::string city_name;
            double temp;
            std::size_t pos = 0;
            if((pos = line.find(';', pos)) != std::string::npos) {
                city_name = line.substr(0, pos);
                temp = std::stod(line.substr(pos+1));
            }
            if(mp.find(city_name) == mp.end()) {
                mp[city_name] = cur++; 
            }
            int temp_idx = mp[city_name];
            city_arr[temp_idx].name = city_name;
            city_arr[temp_idx].count++;
            city_arr[temp_idx].min = std::min(city_arr[temp_idx].min, temp);
            city_arr[temp_idx].max = std::max(city_arr[temp_idx].max, temp);
            city_arr[temp_idx].sum += temp;
        }
        file.close();
    }

    sort(city_arr.begin(), city_arr.end(), [](const City& a, const City& b) {
        return a.name < b.name;
    });
    
    for(int i=0;i<500;i++) {
        if(city_arr[i].name.size() != 0) {
            printf("%s=%0.1f/%0.1f/%0.1f%s", 
                city_arr[i].name.c_str(), 
                city_arr[i].min, 
                city_arr[i].sum / city_arr[i].count, 
                city_arr[i].max,
                i == 499 ? "" : ", "
            );
        }
    }
    std::cout<<"\n";

    
    auto end_time = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli> >(end_time-start_time);
    double time_in_seconds = time_span.count() / 1000.0;

    std::cout<<"Time Taken: "<<time_in_seconds<< " secs"<<std::endl;
    std::cout<<"Time Taken: "<<time_in_seconds / 60.0 << " mins"<<std::endl;

    return 0; 
}

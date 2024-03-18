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
#include <sstream>
#include <thread>

#define CHUNK_SIZE 1048576 
#define NTHREADS 8

struct City {
    std::string name;
    int count;
    double sum, min, max;
    
    City() : name(""), count(0), sum(0), min(101), max(-101) {}  // Default constructor
};

struct Result {
    int n;
    struct City city_arr[500];
};

int
readChunk(std::ifstream* file, char* buf, const int chunk_size) {
    static std::mutex fileMutex;
    std::lock_guard<std::mutex> lock(fileMutex);
    file->read(buf, chunk_size);
    int nread = file->gcount();
    int rewind = 0;
    while(buf[nread-1] != '\n') {
        rewind--;
        nread--;
    }
    file->seekg(rewind, std::ios_base::cur);

    return nread;
}


Result* 
processChunk(std::ifstream* file) {
    std::unordered_map<std::string, int> mp;
    int cur = 0;
    Result* result = new Result();
    std::string line;
    char *buf = new char[CHUNK_SIZE + 1]();

    while(1) {
        int nread = readChunk(file, buf, CHUNK_SIZE);
        if(nread <= 0) {
            break;
        }

        std::string s(buf,nread);
        std::stringstream ss(s);
        std::string line;

        while (std::getline(ss, line, '\n')) {
            std::string city_name;
            double temp;
            std::size_t pos = 0;
            if((pos = line.find(';', pos)) != std::string::npos) {
                city_name = line.substr(0, pos);
                temp = std::stod(line.substr(pos+1));
            }
            if(mp.find(city_name) == mp.end()) {
                mp[city_name] = cur++;
                result->n++;
            }
            int temp_idx = mp[city_name];
            result->city_arr[temp_idx].name = city_name;
            result->city_arr[temp_idx].count++;
            result->city_arr[temp_idx].min = std::min(
                    result->city_arr[temp_idx].min, temp);
            result->city_arr[temp_idx].max = std::max(
                    result->city_arr[temp_idx].max, temp);
            result->city_arr[temp_idx].sum += temp;
        }
    }
    delete[] buf;
    return result;
}


int
main(int argc, const char **argv) {
    std::string filename = "measurements.txt";
    if(argc > 1) {
        filename = argv[1];
    }
    std::ifstream file(filename);    
    std::vector<City> city_arr(500);
    std::unordered_map<std::string, int> mp;
    int cur = 0;

    auto start_time = std::chrono::high_resolution_clock::now(); 
    
    std::vector<std::thread> workers;
    std::vector<Result*> results(NTHREADS);

    for (int i = 0; i < NTHREADS; i++) {
        workers.emplace_back([&]() {
            results[i] = processChunk(&file);
        });
    }
    for (auto& thread : workers) {
        thread.join();
    }

    for(int i=0; i<NTHREADS; i++) {
        for(int j=0;j<500;j++) {
            if(results[i]->city_arr[j].name.size()>0) {
               if(mp.find(results[i]->city_arr[j].name) == mp.end()) {
                    mp[results[i]->city_arr[j].name] = cur++;
                    city_arr[mp[results[i]->city_arr[j].name]].name = results[i]->city_arr[j].name; 
               }
               city_arr[mp[results[i]->city_arr[j].name]].count += results[i]->city_arr[j].count; 
               city_arr[mp[results[i]->city_arr[j].name]].sum += results[i]->city_arr[j].sum; 
               city_arr[mp[results[i]->city_arr[j].name]].min = std::min(
                       results[i]->city_arr[j].min, city_arr[mp[results[i]->city_arr[j].name]].min); 
               city_arr[mp[results[i]->city_arr[j].name]].max = std::max(
                       results[i]->city_arr[j].max, city_arr[mp[results[i]->city_arr[j].name]].max); 
            }
        }
        delete results[i];        
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
    file.close();
    return 0; 
}


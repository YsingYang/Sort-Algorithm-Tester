#ifndef TESTER_H
#define TESTER_H

#include <random>
#include <iostream>
#include <time.h>
#include <functional>
#include <set>
#include <algorithm>
#include<time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <memory>
#include <unordered_set>
#include "threadPool.h"

template<typename T>
class Tester{
    typedef  T value_type;
    typedef std::function<void()> sortFunction;
public:
    Tester(T lower = 0.0, T upper = 65535.0);

private:
    std::default_random_engine engine;
    std::uniform_real_distribution<T> distribution;
};

template<>
class Tester<int>{
    typedef  int value_type;
    typedef std::function<void(int *A, const int N, const int K)> sortFunction;


public:
    Tester();
    ~Tester();

    Tester(std::function<void(int *A, const int N, const int K)> func = [](int *A, const int N, const int K){} ,  int k = 10): predicate(func), K(k){
        maxLength = pow(2, K);
        tp = new threadPool;
        recordFile.open("RandomTestRecord", std::ios::app | std::ios::out);
        AllRandomTestRecord.open("AllRandomTestRecord", std::ios::app | std::ios::out);
        permutationTestRecord.open("permutationTestRecord", std::ios::app | std::ios::out);
    }

    void setFunction(std::function<void(int *A, const int N, const int K)> func){
        predicate = func;
    }

    void resetTestArrayLen();

    bool runRandomTest(int upperK);
    bool runRandomTest(int caseCount, int k);
    bool stableTest();
    bool permutationTest(int lowerBound, int upperBound);
    bool ZeroOneTest(int lowerBound, int upperBount);
    bool halverTest(int* a, int* b, int n, double epsilon);

    static std::default_random_engine engine;
    static std::uniform_int_distribution<int> distribution;

private:

    void generateNextSeq(std::vector<int> &vec);
    bool process(int caseSize, int K, int flag);
    int calculateK(int maxLength);
    int runtime(int*, const int, const int);
    inline void initParameter(int k);
private:

    static std::vector<int> TestCounter;

    std::fstream recordFile; /// 随机测试结果
    std::fstream AllRandomTestRecord; ///完全随机测试结果
    std::fstream permutationTestRecord;

    std::function<void(int *A, const int N, const int K)> predicate;
    threadPool *tp;
    std::vector<int> record;
    int maxLength;
    int K;
};

inline void Tester<int>::initParameter(int k){
    if(k != -1 && k != K){
        K = k;
        maxLength = pow(2, k);
    }
}

//Tester<int>::std::default_random_engine engine;

//需要在原头文件特例


//Tester<int>::randomTest()


#endif // TESTER_H

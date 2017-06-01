#define __DEBUG__
#define __OUTFILE__

#include "Tester.h"
#include <algorithm>



std::default_random_engine  Tester<int>::engine = std::default_random_engine(time(0));
std::uniform_int_distribution<int> Tester<int>::distribution =  std::uniform_int_distribution<int>(0, 2147483647);
//std::uniform_int_distribution<int> Tester<int>::distribution =  std::uniform_int_distribution<int>(0, 65535);
std::vector<int> Tester<int>::TestCounter = std::vector<int> (21, 0); // [0 ~ 20]




template <typename T>
Tester<T>::Tester(T lower, T upper) {
    distribution = std::uniform_real_distribution<T>(lower, upper);
    for(int i = 0; i < 100; ++i){
        std::cout<<distribution(engine)<<"   ";
    }
}


Tester<int>::~Tester(){
    delete tp;
    recordFile.close();
    AllRandomTestRecord.close();
    permutationTestRecord.close();
}

int Tester<int>::calculateK(int maxLength){
        int count = 0;
        while(maxLength){
            ++count;
            maxLength >>= 1;
        }
        return count;
}

int Tester<int>::runtime(int *sorted, const int len, const int K){
    clock_t startTime = clock();
    predicate((int *)sorted, len, K);
    return clock() - startTime;
}
/******************************************

                完全随机测试(多线程跑)

******************************************/
//随机测试随机长度
bool Tester<int>::runRandomTest(int caseCount,int k_){
    initParameter(k_);
    bool isPass  = true;

    assert(K >= 0); //这里不检查k值了

    while( caseCount > 0){
        int k =  Tester<int>::distribution(Tester<int>::engine) % (K + 1);
        int len  = pow(2, k);
        printf("%d\n", len);
        //确定数组长度, 随机为 2 ^ k
        tp->addTask(std::bind(&Tester<int>::process, this, len , k, 0));
        ++Tester<int>::TestCounter[k];

        //判断两个数组是否一致
        --caseCount;
    }
    while(1){
        if(tp->size() == 0){
            tp->stop();
            break;
        }
    }
    #ifdef __OUTFILE__
    for(int i = 0; i <= 20; ++i){
        AllRandomTestRecord<<"VecSize :  " << i << "Successful Times  = "<<Tester<int>::TestCounter[i]<<std::endl;
    }
    #endif // __OUTFILE__

    return isPass;
}

/*************************************************************

                    半随机测试, 单线程, 记录runtime时间


**************************************************************/

bool Tester<int>::runRandomTest(int upperK){
    int k = 1;
    bool isPass = true;
    while(k <= upperK){
        int caseSize = pow(2, k);
        process(caseSize, k, 1);
        //tp->addTask(std::bind(&Tester<int>::process, this, caseSize, k));
        ++k;
    }
    while(1){
        if(tp->size() == 0){
            tp->stop();
            break;
        }
    }
    return isPass;
}

bool Tester<int>::process(int caseSize, int K, int flag){
    std::vector<int> sorted, builtSorted;
    for(int i = 0; i < caseSize; ++i){
        std::shared_ptr<int> num(new int(distribution(engine)));
        sorted.emplace_back(*num);
        builtSorted.emplace_back(*num);
    }

    clock_t startTime = clock();
    predicate(&sorted[0], caseSize, K);
    clock_t sortedTime = clock() - startTime;

    startTime = clock();
    predicate(&builtSorted[0], caseSize, K);
    clock_t builtSortedTime = clock() - startTime;

    if((sorted == builtSorted)){

            #ifdef __OUTFILE__
                recordFile << "pass the case " << K << " Zig-Zag Runtime " << sortedTime << " built-In Runtime  " <<builtSortedTime<<std::endl;
            #endif // __OUTFILE__

            printf("pass the case %d : Test successfully, and Zig-Zag Runtime = %ld,   built-In Runtime = %ld\n",K , sortedTime, builtSortedTime);
            return true;
    }
    else{
            printf("Test fail in case %d\n", caseSize);

            #ifdef __OUTFILE__
            recordFile << "Test fail in case : " << caseSize <<std::endl;
            for(int i = 0; i < caseSize; ++i){
                recordFile<<sorted[i]<<" ";
                printf("%d ", sorted[i]);
            }
            recordFile<<std::endl;
            printf("\n");
            for(int i =0; i < caseSize; ++i){
                recordFile<<builtSorted[i]<<" ";
                printf("%d ", builtSorted[i]);
            }
            recordFile<<std::endl;
            printf("\n");
            #endif // __OUTFILE__

            return false;
    }
}

/*******************************************

                                   完全帧测试

*******************************************/
bool Tester<int>::permutationTest(int lowerBound, int upperBound){ //测试从1 ~ k 的permutation
    if (K < upperBound){
        initParameter(upperBound);
    }
    int count = lowerBound;

    while(count <= upperBound){
        int len = pow(2, count);
        std::vector<int> preSort(len, 0);

        for(int i = 0; i < len; ++i){
            preSort[i] = i;
            printf("%d ", preSort[i]);
        }

        std::vector<int> standard = preSort;
        next_permutation(preSort.begin(), preSort.end());

        int counter = 0;
        while(preSort != standard){
            std::vector<int> postSort = preSort;
            int rt = runtime(&postSort[0], len, count);
            if(postSort == standard){
                //printf("Pass case %d successfully, and runtime is %d \n", counter, rt);
            }
            else{
                printf("Fall in , and postSort array \n");
                for(int i = 0; i < len; ++i){
                    printf("%d ", postSort[i]);
                }
                printf("\n\n");
                return false;
            }
            next_permutation(preSort.begin(), preSort.end());

            #ifdef __OUTFILE__
            permutationTestRecord<<"Permutation runtime : "<< rt <<std::endl;
            #endif // __OUTFILE__

            ++counter;
        }
        ++count;
    }
    return true;
}


/***************************************

                    halver  Test

****************************************/

bool Tester<int>::halverTest(int* a, int* b, int n, double epsilon){
        bool isPass = true;
        std::vector<int> sortedArray(a, a + n);
        copy(b, b + n, back_inserter(sortedArray));

        sort(sortedArray.begin(), sortedArray.end());

        for(int k = 0; k <= n; ++k){
            std::unordered_set<int> largerSet(sortedArray.end() - k, sortedArray.end());
            std::unordered_set<int> smallerSet(sortedArray.begin(), sortedArray.begin() + k);

            int aBound = k * epsilon;
            int bBound = k * epsilon;

            int aCount = 0, bCount = 0;


            #ifdef __DEBUG__
            for(auto i : largerSet){
                std::cout<<i <<"  ";
            }
            std::cout<<std::endl;
            for(auto i : smallerSet){
                std::cout<<i<<"  ";
            }

            std::cout<<std::endl;
            #endif // __DEBUG__

            for(int i = 0; i < n; ++i){
                if(largerSet.find(a[i]) != largerSet.end()){
                    ++aCount;
                }
                if(smallerSet.find(b[i]) != smallerSet.end()){
                    ++bCount;
                }
            }
            std::cout<<aCount <<"   "<<bCount <<std::endl;
            isPass &= aCount <= aBound && bCount <=bBound;
            if((isPass & 1) == 0){
                std::cout<<"Fail in case  k = " << k <<std::endl;
            }
        }
        return isPass;
}


/**********************************************


                                0 - 1 测试


***********************************************/

bool Tester<int>::ZeroOneTest(int lowerBound, int upperBount){
    int k = lowerBound;
    bool isPass =true;
    while(k <= upperBount){
        int length = pow(2, k);
        std::vector<int> standard(length, 0);
        std::vector<int> preSort = standard;
        generateNextSeq(preSort);
        while(preSort != standard){
            #ifdef __DEBUG__
                for(int i = 0; i < preSort.size(); ++i){
                    std::cout<<preSort[i]<<" ";
                }
                std::cout<<std::endl;


            #endif // __DEBUG__

            std::vector<int> postSort = preSort;
            int rt = runtime(&postSort[0], length, k);
            for(int i = 0; i < length -1; ++i){
                if(postSort[i] > postSort[i + 1]){
                    std::cout<<"fail in case ";
                    isPass = false;
                }
            }
            if(isPass){
                 printf("pass case successfully and runtime  %d \n ", rt);
            }
            generateNextSeq(preSort);
        }
        ++k;
    }
    return isPass;
}


void Tester<int>::generateNextSeq(std::vector<int> &vec){
    for(int i = vec.size() - 1; i >= 0; --i){
        if(vec[i] == 0){
            vec[i] = 1;
            return;
        }
        vec[i] &= 0 ;
    }
}


//bool Tester<int>::stableTest(int k)


#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <sstream>

std::vector<std::string> lines;

std::mutex mutex;
std::mutex matchmutex;
std::mutex totalTimeMutex;

std::string searchTerm = "And"; // the searched term

int totalMatches = 0;
std::chrono::microseconds totalTimeMicro;

void printWordCount(std::thread::id& ID, int& counter, std::string& searchTerm, std::chrono::microseconds& timeMS) // synchronized printer function
{
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Thread ID: " << ID << " found " << counter << " occurances of the word: " << searchTerm << ", WorkTime was: " << timeMS.count() << " microSeconds" << std::endl;
}

bool findWord(std::string& line) // the Count_if bool condition
{
    return line.find(searchTerm) != std::string::npos;
}

void updateTotalMatches(int& matches)
{
    totalMatches += matches;
}

void updateTotalTime(std::chrono::microseconds& micro)
{
    totalTimeMicro += micro;
}

void threadTask(int offset, int linesPerThread, std::string searchTerm)
{
    std::thread::id threadID = std::this_thread::get_id();
    int wordCount = 0;

    auto start = std::chrono::high_resolution_clock::now();
    wordCount += std::count_if(lines.begin() + offset, lines.begin() + offset + linesPerThread, findWord); // count words found
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); // execution duration

    {
        std::lock_guard<std::mutex> lock(matchmutex);
        updateTotalMatches(wordCount);
    }

    {
        std::lock_guard<std::mutex> timeLock(totalTimeMutex);
        updateTotalTime(duration);
    }

    printWordCount(threadID, wordCount, searchTerm, duration);
}

int main()
{
    const int hardwareThreads = std::thread::hardware_concurrency();
    std::cout << "Hardwarethreads: " << hardwareThreads << std::endl;

    const int searchThreads = 12;
    int linesPerThread = 0;
    std::thread threads[searchThreads];

    //load in file
    std::ifstream file(""); //TextFile name HERE!

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file!" << std::endl;
        return 1;
    }

    //divide task among threads
    linesPerThread = (lines.size() / searchThreads);
    std::cout << "Lines Per thread: " << linesPerThread << std::endl;

    int offset = 0;

    for (int i = 0; i < searchThreads; i++) // start thread execution
    {
        threads[i] = std::thread(threadTask, offset, linesPerThread, searchTerm);
        offset += linesPerThread;
    }

    for (int j = 0; j < searchThreads; j++)
    {
        threads[j].join();
    }

    // total results
    std::cout << "Total matches: " << totalMatches << std::endl;
    std::cout << "Total workTime: " << totalTimeMicro.count() << " microSeconds" << std::endl;
    
    return 0;
}
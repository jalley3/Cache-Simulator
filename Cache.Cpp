/**
 * @file cache.cpp
 * @author Jacob Alley
 * @date 07/11/15
 * @brief Basic cache simulator.
 *
 * @section DESCRIPTION
 * This program provides a basic simulation of cache behavior
 ***************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <math.h>

//function declarations that do not depend on class declarations

std::string hexAddress(std::string);
std::string readWrite (char);
std::string hexChunk(std::string);
int intChunk(std::string);
void printInfo(long,int,int,int);

//class for my Cache Entries
class Access{
public:
  int tag_, index_, offset_;
  std::string address_,rw_;
  bool valid;
  int indexNum, tagNum, offsetNum;
  Access(long cs, int ls,int ss,long ns, std::string addy, char rw){
    int offsetSize = log2(ls);
    int indexSize = log2(ns);
    int tagSize = 32 - offsetSize - indexSize;
    tag_ = intChunk(addy.substr(0,tagSize));
    index_ = intChunk(addy.substr(tagSize, indexSize));
    offset_= intChunk(addy.substr((tagSize+indexSize),offsetSize));
    address_  = hexChunk(addy);
    rw_ = readWrite(rw);
  }
 };
//Cache class to create cache object, which is essentially a vector of vectors
class Cache{
public:
  std::vector<std::vector<Access>> cacheVec;
  int setSize;
  int hitCount = 0;
  Cache(long ns, int ss){
    setSize = ss;
    //initialize number of sets
    for (int i=0;i<ns;i++){
      std::vector<Access> set;
      cacheVec.push_back(set);
    }
  }
  std:: string newEntry (Access entry){
    std::string answer = "Miss"; //default
    int pos = entry.index_;
    for(int i = 0; i < cacheVec[pos].size();i++){
      if(entry.tag_ == cacheVec[pos][i].tag_){
        cacheVec[pos].erase(cacheVec[pos].begin()+i);
        answer = "Hit";
        hitCount++;
        break; //break once it is found
      }
    }
    if(answer == "Miss" && cacheVec[pos].size()== setSize){
      cacheVec[pos].erase(cacheVec[pos].begin());
      }
    cacheVec[pos].push_back(entry);
    return answer;
  }
};
//functions that depend on class objects
void printAccess(Access, int, std::string);
void hitSummary (Cache, int);

int main(int argc, char* argv[])
{
  //check for corect number of arguments and open file
  if(argc == 1)
  {
    std::cout << "** Error: no file provided" << '\n';
  }
  else if (argc > 3){
    std::cout << "**Error: Only 2 files accepted" <<std::endl;
  }
  std::ifstream cacheConfigFile(argv[1]);
  //get variables for later
  int setSize, lineSize;
  long cacheSize;
  cacheConfigFile >> setSize >> lineSize >> cacheSize;
  cacheConfigFile.close();
  // calculate number of sets
  long numSets = cacheSize / (setSize * lineSize);
  printInfo(cacheSize, lineSize, setSize, numSets);
  std::ifstream instructionFile(argv[2]);
  std::vector<std::string> sourceCode;
  std::string lineIn;
  while(!instructionFile.eof())  {
    std::ws(instructionFile);
    std::getline(instructionFile, lineIn);
    if (instructionFile.eof())break;
    sourceCode.push_back(lineIn); //add to the list of instructions from source
  }
  instructionFile.close();//release resources
  Cache c(numSets, setSize);
  for (int i = 0; i < sourceCode.size();i++){
    char rw = sourceCode[i][0];
    std::string addr = "0x";
    addr += sourceCode[i].substr(4);
    std::stringstream ss;
    ss << std::hex << addr;
    int hex2Bin;
    ss >> hex2Bin;
    std::bitset<32> b(hex2Bin);
    addr = b.to_string();
    Access entry(cacheSize, lineSize, setSize, numSets, addr, rw);
    std::string var = c.newEntry(entry); //get hit or miss;
    printAccess(entry,i, var);
  }
  hitSummary(c, sourceCode.size());
  return 0;
}

void printInfo(long cs, int ls, int ss, int  ns){
  std::cout << "\nTotal Cache Size:  " << cs
            << "B\nLine Size:  " << ls
            << "B\nSet Size:  " << ss
            << "\nNumber of Sets:  " << ns << "\n\n";
  std::cout << std::setw(8)  << std::left << "RefNum";
  std::cout << std::setw(10) << std::left << "  R/W";
  std::cout << std::setw(13) << std::left << "Address";
  std::cout << std::setw(6)  << std::left << "Tag";
  std::cout << std::setw(8)  << std::left << "Index";
  std::cout << std::setw(10) << std::left << "Offset";
  std::cout << std::setw(8)  << std::left << "H/M";
  std::cout << std::setfill('*') << std::setw(64) << "\n" << std::setfill(' ');
  std::cout << "\n";
}

std::string readWrite(char rw){
  if (rw == 'r' || rw == 'R')return "Read";
  else return "Write";
}

std::string hexChunk(std::string bin){
  int result =0 ;
  for(size_t count = 0; count < bin.length() ; ++count)
  {
    result *=2;
    result += bin[count]=='1'? 1 :0;
  }
  std::stringstream ss;
  ss <<std::hex << result;
  std::string hexVal(ss.str());
  return hexVal;
}

int intChunk(std::string bin){
  int result = 0 ;
  for(size_t count = 0; count < bin.length() ; ++count)
  {
    result *=2;
    result += bin[count]=='1'? 1 :0;
  }
  return result;
}

void printAccess(Access entry, int i, std::string hitMiss){
  std::cout << "   " << std::setw(5) << std::left << std::dec << i;
  if (entry.rw_ == "Read"){
    std::cout << " " << std::setw(7)<<entry.rw_;
  }else{
    std::cout << std::setw(8)<<entry.rw_;}
  std::cout << "  " << std::setfill('0') << std::setw(8) << std::hex << std::right << entry.address_;
  std::cout << std::setfill(' ') << std::setw(7) << entry.tag_;
  std::cout << std::setw(8) << std::dec << entry.index_;
  std::cout << std::setw(8) << entry.offset_;
  std::cout << std::setw(10) << hitMiss;
  std::cout << "\n";
}

void hitSummary (Cache c, int i){
  int totalMisses = i - c.hitCount;
  double hitRate = c.hitCount/(double)i;
  double missRate = 1-hitRate;
  std::cout << "\n";
  std::cout << "    Simulation Summary\n";
  std::cout << "**************************\n";
  std::cout << "Total Hits:\t"   << c.hitCount << "\n";
  std::cout << "Total Misses:\t" << totalMisses << "\n";
  std::cout << "Hit Rate:\t"     << std::setprecision(5) << hitRate   << "\n";
  std::cout << "Miss Rate:\t"    << std::setprecision(5) << missRate  << "\n";
}

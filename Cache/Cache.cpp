#include <cmath>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <list>
#include <bits/stdc++.h>
using namespace std;

//declaring a struct node which represent cache block
typedef struct node{
  long long int tag=-1;
  int dirty=0;
  int valid=0;
}cacheblock;

//class CacheMem represents cachememory
class CacheMem{
private:
    vector<list<cacheblock>> Cache;
    int numofsets;
    int numblocksinset;
    int tagsize;
    int setsize;
    long long int calculatetag(string address);  //calculates tag for given address
    long long int calculateset(string address);  //calculates set for given address
    vector<vector<long long int>> Visitedblocks; //stores blocks which are evicted from cache
    vector<vector<cacheblock>> Tree;//used for pseudo LRU
public:
  //constructor
      CacheMem(int n,int m,int p,int q){
        Cache.resize(n);                     // creating n number of cachesets
        for(int i=0;i<n;i++){
            Cache[i].resize(m);              // creating m number of cacheblocks in each cacheset
        }
        numofsets=n;                          
        numblocksinset=m;
        tagsize=p;
        setsize=q;
        Tree.resize(n);
        for(int i=0;i<n;i++){
            Tree[i].resize(2*m-1);                   
            for(int j=0;j<m-1;j++){
              Tree[i][j].tag=0;                        // setting all values in tree to 0
            }
        }
        Visitedblocks.resize(n);                      // creating n number of visitedblock sets.
        for(int i=0;i<n;i++){
            Visitedblocks[i].resize(m);               // creating m number of visitedblocks in each set     
        }
      }
      //methods for different policies
      void Random(string address,int p);
      void LRUpolicy(string address,int p);
      void PseudoLRU(string address,int p);
      int FindBlock(int index,long long int tag);
      //initializing all parameters to 0
      int readmisses=0;
      int writemisses=0;
      int misscount=0;
      int compulsorymisscount=0;
      int capacitymisscount=0;
      int conflictmisscount=0;
      int dirtyevicted=0;
  };

int CacheMem ::FindBlock(int index,long long int tag_address){
    auto it = Visitedblocks[index].begin();
    int i=0;
    for(it=Visitedblocks[index].begin();it!=Visitedblocks[index].end();it++){       // searching for the tag in the visitedblocks
      if(Visitedblocks[index][i]==tag_address){
        return 1;
      }
      i++;
    }
    return 0;                 
  }

//Implementation of random policy
void CacheMem::Random(string address,int w){
      int tag_address = calculatetag(address);                        // calculates tag
      int index = calculateset(address);                             //  calculates set index
      auto it = Cache[index].begin();                               // pointer for the cacheset
      cacheblock p;                                                // used to access cacheblocks in the cacheset
      int q;
      for(it=Cache[index].begin();it!=Cache[index].end();it++){
         p = *it;
        if(p.tag==tag_address){
            p.tag=tag_address;
            q=p.dirty;
            p.dirty = (w==1||q==1) ? 1:0;                       // modifying the dirty bit of the block
            p.valid=1;                                         // making the block valid 
            return;
        }
        else if(p.valid==0){                                  // checking if there is any space in the cacheset                        
          if(w==0){
            readmisses++;
          }
          else{
            writemisses++;
          }
          misscount++;
          compulsorymisscount++;
          p.tag=tag_address;                                 // updating the tag of the empty block
          p.dirty = (w==0) ? 0:1;                           // updating the dirty bit of empty block
          p.valid=1;                                       // making the block valid                                 
          return;
        }
      }
      if(w==0){
        readmisses++;
      }
      else{
        writemisses++;
      }
      misscount++;
      int x = FindBlock(index,tag_address);                   // checking whether the given tag is accessed before
      if(x==0){
        compulsorymisscount++;
      }
      else{
        conflictmisscount++;                                      // incrementing conflict misses if the given tag is already accessed before
      }
      capacitymisscount++;
      Visitedblocks[index].push_back(p.tag);                     // pushing the evicted block into the visitedblocks set
      Cache[index].pop_back();                                   // deleting the last block 
      p.tag=tag_address;                                         
      q=p.dirty;
      if(q==1){
        dirtyevicted++;                                         // checking whether evicted block is dirty
      }
      p.dirty = (w==1) ? 1:0;
      p.valid=1;
      Cache[index].push_back(p);                                 // inserting given tag into the last block
  }

//implementation of lru policy
void CacheMem::LRUpolicy(string address,int w){
  long long int tag_address = calculatetag(address);               // calculates tag           
  long long int index = calculateset(address);                     //  calculates set index
  auto it = Cache[index].begin();                                  // pointer for the cacheset
  cacheblock p;                                                    // used to access cacheblocks in the cacheset
  int q;
  for(it=Cache[index].begin();it!=Cache[index].end();it++){
     p = *it;
    if(p.tag==tag_address){                                       // searching for the given tag in the cache set
        Cache[index].erase(it);                                  
        p.tag=tag_address;
        q=p.dirty;
        p.dirty = (w==1||q==1) ? 1:0;                           // modifying the dirty bit of the cacheblock
        p.valid=1;
        Cache[index].insert(Cache[index].begin(),p);            // removing the block and inserting it at the beginning
        return;
    }
    else if(p.valid==0){                                        // if an empty block is found
      if(w==0){
        readmisses++;
      }
      else{
        writemisses++;
      }
      misscount++;
      compulsorymisscount++;
      Cache[index].erase(it);
      p.tag=tag_address;
      p.dirty = (w==0) ? 0:1;                                  // modifying the dirty bit of the cacheblock
      p.valid=1;                                               // modyfing the valid bit of the cacheblock
      Cache[index].insert(Cache[index].begin(),p);            // removing the block and inserting it at the beginning
      return;
    }
  }
  if(w==0){
    readmisses++;
  }
  else{
    writemisses++;
  }
  misscount++;
  int x = FindBlock(index,tag_address);                     // checking whether the given tag is accessed before
  if(x==0){
    compulsorymisscount++;                              
  }
  else{
    conflictmisscount++;                                   // incrementing conflict misses if the given tag is already accessed before
  }
  capacitymisscount++;
  Visitedblocks[index].push_back(p.tag);                  // pushing the evicted block into the visitedblocks set    
  Cache[index].pop_back();                                // deleting the LRU block
  p.tag=tag_address;
  q=p.dirty;
  if(q==1){
    dirtyevicted++;                                       // checking whether evicted block is dirty
  }
  p.dirty = (w==1) ? 1:0;
  p.valid=1;
  Cache[index].push_front(p);                               // inserting the tag at the beginning of the cacheset
}

//implementation of Pseudo LRU policy
void CacheMem::PseudoLRU(string address,int w){
  long long int tag_address = calculatetag(address);                // calculates tag 
  int index = calculateset(address);                                //  calculates set index
  for(int i = numblocksinset-1;i<2*numblocksinset-1;i++){           // searching for the given tag in the cache set
    if(Tree[index][i].tag==tag_address){
      int j=i;
      while(j!=0){
        int parent = (j-1)/2;                                       
          Tree[index][parent].tag=j%2;                              // flipping the bits of the tree such that it doesnt point to the present block
        j=parent;
      }
      int q=Tree[index][i].dirty;
      Tree[index][i].dirty = (w==1||q==1) ? 1:0;                  // modifying the dirty bit of the cache block
      return;
    }
  }
  if(w==0){
    readmisses++;
  }
  else{
    writemisses++;
  }
  int i=0;
  while(i<numblocksinset-1){
      Tree[index][i].tag=(Tree[index][i].tag+1)%2;                      // traversing through the tree and flipping the bits 
      i = 2*i+2-Tree[index][i].tag;
  }
  if(Tree[index][i].tag ==0){                                         // if the pseudoLRU block is empty 
    misscount++;
    int x = FindBlock(index,tag_address);                            // checking whether the given tag is accessed before
    if(x==0){
      compulsorymisscount++;
    }
    else{
      conflictmisscount++;                                          // incrementing conflict misses if the given tag is already accessed before
    }
    Tree[index][i].tag=tag_address;
    if(w==1){
      Tree[index][i].dirty=1;                                       // modifying the dirty bit of the cache block              
    }
  }
  else{
    int r= Tree[index][i].dirty;
    if(r==1){
      dirtyevicted++;                                               // checking whether evicted block is dirty
    }
    misscount++;
    int x = FindBlock(index,tag_address);                           // checking whether the given tag is accessed before
    if(x==0){
      compulsorymisscount++;
    }
    else{
      conflictmisscount++;                                         // incrementing conflict misses if the given tag is already accessed before
    }
    capacitymisscount++;
    Visitedblocks[index].push_back(Tree[index][i].tag);           // pushing the evicted block into the visitedblocks set
    Tree[index][i].tag=tag_address;
    Tree[index][i].dirty=0;
    if(w==1){
      Tree[index][i].dirty=1;                                  // modifying the dirty bit of the cache block
    }
  }
}
//calculates tagaddress using address
long long int CacheMem::calculatetag(string address) {
  string binarynum;
  binarynum.resize(tagsize);
  for(int i=0;i<tagsize;i++){
    binarynum[i]=address[i];
  }
  long long int x=stoi(binarynum,0,2);
  return x;
}
//caclculates set number using address
long long int CacheMem::calculateset(string address) {
  string binarynum;
  if(setsize==0){
    return 0;
  }
  else{
    binarynum.resize(setsize);
    for(int i=tagsize;i<tagsize+setsize;i++){
      binarynum[i-tagsize]=address[i];
    }
    return stoi(binarynum,0,2);
  }
  return 1;
}
/*******************************************End of Class************************************************/
//function for converting hexadecimal character to its binary form
string converthextobin(char c)
  {
      switch(c)
      {
          case '0': return "0000";
          case '1': return "0001";
          case '2': return "0010";
          case '3': return "0011";
          case '4': return "0100";
          case '5': return "0101";
          case '6': return "0110";
          case '7': return "0111";
          case '8': return "1000";
          case '9': return "1001";
          case 'A': return "1010";
          case 'B': return "1011";
          case 'C': return "1100";
          case 'D': return "1101";
          case 'E': return "1110";
          case 'F': return "1111";
          case 'a': return "1010";
          case 'b': return "1011";
          case 'c': return "1100";
          case 'd': return "1101";
          case 'e': return "1110";
          case 'f': return "1111";
      }
      return "1";
  }

  int main() {
      int cachesize;                            // stores cachesize
      int blocksize;                            // stores blocksize
      int associativity;                       // stores associtivity
      int policy;                              // stores replacement policy
      fstream file;                           // file pointer
      file.open("input.txt",ios::in);         // opens input.txt file
      cout<<"Enter Cache Size : "; 
      cin>>cachesize;
      cout<<"Enter Block Size : ";
      cin>>blocksize;
      cout<<"Enter Associativity  : ";
      cin>>associativity;
      cout<<"Enter Policy : ";
      cin>>policy;
      int totalnumblocks = cachesize/blocksize;        //calculating number of cacheblocks in cache memory
      int numofsets;
      int numblocksinset;
      cout<<endl;
      cout<<"Cache Size :       "<<cachesize<<endl;
      cout<<"Block Size :       "<<blocksize<<endl;

      //based on associativity deciding number of sets and number of blocks in each set
      if(associativity==0){
        cout<<"Fully Associative cache"<<endl;
        numofsets=1;                                
        numblocksinset=totalnumblocks;
      }
      else if(associativity==1){
        cout<<"Direct Mapped cache"<<endl;
        numofsets=totalnumblocks;
        numblocksinset=1;
      }
      else {
        cout<<associativity<<" Way Set Associative Cache "<<endl;
        numblocksinset=associativity;
        numofsets=totalnumblocks/associativity;
      }
      //calculating number of bits in each of the fields of 32 bit-adrress
      int bits_setindex = log2(numofsets);
      int bits_blockoffset=log2(blocksize);
      int bits_tag=32-(bits_setindex+bits_blockoffset);

      CacheMem object(numofsets,numblocksinset,bits_tag,bits_setindex);//constructing CacheMem object

      int readaccess=0;
      int writeaccess=0;
      int cacheaccess=0;
      string hex;
      string x;
      int p;

      while(true){
        string bin;
        file>>hex>>x;
        if(file.eof()){
          break;              //if end of file is reached ,break
        }
        if(x=="r"){
          p=0;
          readaccess++;
          cacheaccess++;
        }
        else{
          p=1;
          writeaccess++;
          cacheaccess++;
        }
        int len = hex.length();
        for(int j=2;j<len;j++){
          bin+=converthextobin(hex[j]);    //convert hexadecimal string into binary
        }
        //based on policy select the appropriate policy to be implemented
        if(policy==0){
            object.Random(bin,p);
        }
        else if(policy==1){
            object.LRUpolicy(bin,p);
        }
        else if(policy==2){
            object.PseudoLRU(bin,p);
        }
      }
      file.close();       //closing the opened input file
      if(policy==0){
          cout<<"Random Policy"<<endl;
      }
      else if(policy==1){
          cout<<"LRU Policy"<<endl;
      }
      else if(policy==2){
          cout<<"Pseudo LRU Policy"<<endl;
      }
      //printing all required parameters
      cout<< "Cache accesses :       "<<cacheaccess<<endl;
      cout<< "Read accesses :        "<<readaccess<<endl;
      cout<< "Write accesses :       "<<writeaccess<<endl;
      cout<< "Cache Misses :         "<<object.misscount<<endl;
      cout<< "Compulsory Misses :    "<<object.compulsorymisscount<<endl;
      if(associativity!=0){
        cout<< "Capacity Misses :      "<<0<<endl;                        // capacity misses calculated only for fully assoctivity sets
      }
      else{
        cout<< "Capacity Misses :      "<<object.capacitymisscount<<endl;
      }
      cout<< "Conflict Misses :      "<<object.conflictmisscount<<endl;
      cout<< "Read Misses :          "<<object.readmisses<<endl;
      cout<< "Write Misses :         "<<object.writemisses<<endl;
      cout<< "Dirty blocks evicted : "<<object.dirtyevicted<<endl;
      return 0;
  }

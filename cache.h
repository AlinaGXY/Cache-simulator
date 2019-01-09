#include <math.h>
#include <stdint.h>

#include <iostream>
#include <vector>

#include "block.h"

using namespace std;

class cache
{
 public:
  uint32_t capacity;
  uint32_t block_size;
  uint32_t blockNumber;
  uint32_t setNumber;
  int set_offset;
  int block_offset;
  vector<set> memory;
  cache * next_D;
  cache * next_I;
  bool write_allocate;
  uint32_t statistic[3][2];
  line (*replace)(set &, request & req);

  bool completeField();
  request generateRequest(int type, uint32_t address);
  uint32_t generateAddress(uint32_t tag, uint32_t index);
  void find(request & req);
  void printStatistic();
  cache & operator=(const cache & rhs) {
    capacity = rhs.capacity;
    block_size = rhs.block_size;
    blockNumber = rhs.blockNumber;
    setNumber = rhs.setNumber;
    set_offset = rhs.set_offset;
    block_offset = rhs.block_offset;
    memory = rhs.memory;
    next_D = rhs.next_D;
    next_I = rhs.next_I;
    write_allocate = rhs.write_allocate;
    replace = rhs.replace;
    for (int i = 0; i < 3; i++) {
      statistic[i][0] = rhs.statistic[i][0];
      statistic[i][1] = rhs.statistic[i][1];
    }
    return *this;
  }
};

void cache::find(request & req) {
  statistic[req.type][0]++;
  set & curr = memory[req.index];
  curr.grow();

  //cout << "tag: " << req.tag << endl;
  //cout << "index: " << req.index << endl;

  unordered_map<uint32_t, line>::iterator it = curr.lines.find(req.tag);
  // hit
  if (it != curr.lines.end()) {
    //    cout << "hit" << endl;
    it->second.age = 0;
    if (req.type == 1) {
      it->second.dirty = true;
    }
    return;
  }

  // miss
  statistic[req.type][1]++;  // miss count + 1
  //cout << "miss" << endl;
  //cout << endl;

  // read or fetch or allocated write
  if (req.type != 1 || write_allocate) {
    if (curr.lines.size() == curr.lineNumber) {
      // replace one old line with the new line
      line kick = replace(curr, req);

      // if kick dirty block from L1, write back
      if (kick.dirty && next_D != NULL) {
        uint32_t address = generateAddress(kick.tag, req.index);
        request writeBack = next_D->generateRequest(1, address);
        next_D->find(writeBack);
      }
    }
    else {
      bool dirty = req.type == 1 ? true : false;  // new write data is dirty
      curr.lines.emplace(req.tag, line(dirty, req.type > 1, req.tag, 0));
    }

    if (req.type < 2 && next_D != NULL) {
      request find_in_next_level = next_D->generateRequest(0, req.address);
      next_D->find(find_in_next_level);
    }
    else if (req.type == 2 && next_I != NULL) {
      request find_in_next_level = next_I->generateRequest(2, req.address);
      next_I->find(find_in_next_level);
    }
  }

  // write request but not write allocate, just find in lower level
  else if (!write_allocate && next_D != NULL) {
    request write = next_D->generateRequest(req.type, req.address);
    next_D->find(write);
  }
}

uint32_t cache::generateAddress(uint32_t tag, uint32_t index) {
  uint32_t address = tag;
  address = tag << set_offset;
  address = address & index;
  address = address << block_offset;
  return address;
}

request cache::generateRequest(int type, uint32_t address) {
  request req;
  req.type = type;
  req.tag = address >> (block_offset + set_offset);
  req.index = (address << (32 - block_offset - set_offset)) >> (32 - set_offset);
  req.address = address;
  return req;
}

bool cache::completeField() {
  setNumber = capacity / block_size / blockNumber;
  set_offset = log2(setNumber);
  if (1 << set_offset != setNumber) {
    return false;
  }
  block_offset = log2(block_size);
  if (1 << block_offset != block_size) {
    return false;
  }
  uint32_t tag_offset = 24 - set_offset - block_offset;
  if (tag_offset <= 0) {
    return false;
  }
  for (int i = 0; i < 3; i++) {
    statistic[i][0] = 0;
    statistic[i][1] = 0;
  }

  memory = vector<set>(setNumber, set(blockNumber));

  /*cout << "capacity = " << capacity << endl;
  cout << "block number in one set = " << blockNumber << endl;
  cout << "block offset = " << block_offset << endl;
  cout << "index offset = " << set_offset << endl;
  cout << "tag offset = " << tag_offset << endl;
  cout << "write allocate ?: " << write_allocate << endl;
  */

  return true;
}

void cache::printStatistic() {
  uint32_t total = statistic[0][0] + statistic[1][0] + statistic[2][0];
  uint32_t data_total = statistic[0][0] + statistic[1][0];
  uint32_t data_miss = statistic[0][1] + statistic[1][1];
  uint32_t data_hit = data_total - data_miss;
  cout << "  Total: " << total << endl;
  cout << "  Data:  " << data_total << "    hit: " << data_hit << "    miss: " << data_miss << endl;
  cout << "  Read:  " << statistic[0][0] << "    hit: " << statistic[0][0] - statistic[0][1]
       << "    miss: " << statistic[0][1] << endl;
  cout << "  Write: " << statistic[1][0] << "    hit: " << statistic[1][0] - statistic[1][1]
       << "    miss: " << statistic[1][1] << endl;
  cout << " Instrn: " << statistic[2][0] << "    hit: " << statistic[2][0] - statistic[2][1]
       << "    miss: " << statistic[2][1] << endl;
  cout << "\n" << endl;
}

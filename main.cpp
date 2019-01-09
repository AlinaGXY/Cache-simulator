#include <string.h>

#include <ctime>
#include <fstream>

#include "block.h"
#include "cache.h"

line RandomReplace(set & s, request & req) {
  srand(time(NULL));
  int n = rand() % s.lines.size();
  unordered_map<uint32_t, line>::iterator it = s.lines.begin();
  for (int i = 0; i < n; i++) {
    ++it;
  }
  line goat(it->second);
  line newblock;
  newblock.dirty = req.type == 1 ? true : false;
  newblock.insn = req.type > 1 ? true : false;
  newblock.tag = req.tag;
  newblock.age = 0;
  s.lines.erase(it->first);
  s.lines.emplace(newblock.tag, newblock);
  return goat;
}

line LeastRecent(set & s, request & req) {
  uint32_t max = 0;
  unordered_map<uint32_t, line>::iterator rp;
  unordered_map<uint32_t, line>::iterator it = s.lines.begin();
  unordered_map<uint32_t, line>::iterator end = s.lines.end();
  for (; it != end; ++it) {
    if (it->second.age > max) {
      max = it->second.age;
      rp = it;
    }
  }
  line goat(rp->second);
  line newblock;
  newblock.dirty = req.type == 1 ? true : false;
  newblock.insn = req.type > 1 ? true : false;
  newblock.tag = req.tag;
  newblock.age = 0;
  s.lines.erase(rp->first);
  s.lines.emplace(newblock.tag, newblock);
  return goat;
}

void initialize(cache & l1, cache & l1D, cache & l2, cache & l2D, char ** argv) {
  bool l2exist = atoi(argv[2]) == 2 ? true : false;
  int l1unified = atoi(argv[5]);
  int l2unified = atoi(argv[12]);

  l1.capacity = atoi(argv[6]);
  l1.block_size = atoi(argv[7]);
  l1.blockNumber = atoi(argv[8]);

  l1D.capacity = atoi(argv[9]);
  l1D.block_size = atoi(argv[10]);
  l1D.blockNumber = atoi(argv[11]);

  l2.capacity = atoi(argv[13]);
  l2.block_size = atoi(argv[14]);
  l2.blockNumber = atoi(argv[15]);

  l2D.capacity = atoi(argv[16]);
  l2D.block_size = atoi(argv[17]);
  l2D.blockNumber = atoi(argv[18]);

  l1.write_allocate = l2.write_allocate = l1D.write_allocate = l2D.write_allocate =
      (atoi(argv[3]) == 1 ? true : false);

  if (atoi(argv[12]) == 1) {
    l1.replace = l2.replace = l1D.replace = l2D.replace = RandomReplace;
  }
  else {
    l1.replace = l2.replace = l1D.replace = l2D.replace = LeastRecent;
  }

  if (!l2exist) {
    l1.completeField();
    l1.next_D = NULL;
    l1.next_I = NULL;
  }
  else if (l1unified && l2unified) {
    l1.next_D = &l2;
    l1.next_I = &l2;
    l1.completeField();
    l2.completeField();
  }
  else if (l1unified) {
    l1.next_D = &l2D;
    l1.next_I = &l2;
    l1.completeField();
    l2.completeField();
    l2D.completeField();
  }
  else if (l2unified) {
    l1.next_D = &l2;
    l1.next_I = &l2;
    l1D.next_D = &l2;
    l1D.next_I = &l2;
    l1.completeField();
    l1D.completeField();
    l2.completeField();
  }
  else {
    l1.next_D = &l2D;
    l1.next_I = &l2;
    l1D.next_D = &l2D;
    l1D.next_I = &l2;
    l1.completeField();
    l1D.completeField();
    l2.completeField();
    l2D.completeField();
  }
  l2.next_D = NULL;
  l2.next_I = NULL;
  l2D.next_D = NULL;
  l2D.next_I = NULL;
}

int main(int argc, char * argv[]) {
  if (argc < 19) {
    cout << "Too few arguments" << endl;
    return 1;
  }

  ifstream ifs(argv[1]);  //file name
  if (!ifs) {
    perror("open file");
    return 1;
  }

  cache l1;
  cache l2;
  cache l1D;
  cache l2D;
  int l1unified = atoi(argv[5]);
  int l2unified = atoi(argv[12]);
  bool l2exist = atoi(argv[2]) == 2 ? true : false;

  initialize(l1, l1D, l2, l2D, argv);

  char req[30];
  while (ifs.getline(req, 30)) {
    char * deli = strchr(req, ' ');
    *(deli++) = '\0';
    int type = atoi(req);
    char * pend;
    uint32_t address = strtoll(deli, &pend, 16);

    if (l1unified || type > 1) {
      request req = l1.generateRequest(type, address);
      l1.find(req);
    }
    else {
      request req = l1D.generateRequest(type, address);
      l1D.find(req);
    }

    /* set & s = l1.memory[10];
    unordered_map<uint32_t, line>::iterator it = s.lines.begin();
    unordered_map<uint32_t, line>::iterator end = s.lines.end();
    while (it != end) {
      cout << it->first << endl;
      ++it;
    }
    cout << '\n' << endl;*/
  }

  cout << "\n";
  cout << "  L1(insn/unity)" << endl;
  cout << "---------------------------------------------------------" << endl;
  l1.printStatistic();
  if (!l1unified) {
    cout << "  L1 data" << endl;
    cout << "---------------------------------------------------------" << endl;
    l1D.printStatistic();
  }
  if (l2exist) {
    cout << "  L2(insn/unity)" << endl;
    cout << "---------------------------------------------------------" << endl;
    l2.printStatistic();
    if (!l2unified) {
      cout << "  L2 data" << endl;
      cout << "---------------------------------------------------------" << endl;
      l2D.printStatistic();
    }
  }
}

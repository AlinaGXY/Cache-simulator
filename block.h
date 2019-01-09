#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <unordered_map>

using namespace std;

#ifndef _H_BLOCK
#define _H_BLOCK

class request
{
 public:
  int type;
  uint32_t tag;
  uint32_t index;
  uint32_t address;

  request(int m, int t, int s, uint32_t a) : type(m), tag(t), index(s), address(a) {}
  request() {}
};

class line
{
 public:
  bool dirty;
  bool insn;
  uint32_t tag;
  uint32_t age;

  line() : tag(0), age(0) {}
  line(bool d, bool i, int t, long a) : dirty(d), insn(i), tag(t), age(a) {}
  line(const line & rhs) : dirty(rhs.dirty), insn(rhs.insn), tag(rhs.tag), age(rhs.age) {}
};

class set
{
 public:
  uint32_t lineNumber;
  unordered_map<uint32_t, line> lines;

  set(int n) : lineNumber(n) {}
  void grow() {
    unordered_map<uint32_t, line>::iterator it = lines.begin();
    unordered_map<uint32_t, line>::iterator end = lines.end();
    for (; it != end; ++it) {
      it->second.age++;
    }
  }
};

#endif

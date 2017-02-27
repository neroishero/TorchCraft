/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

extern "C" {
#include <TH/TH.h>
}

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "refcount.h"
#include "frame.h"

namespace torchcraft {
  namespace replayer {

    struct Map{
      THByteTensor* data;
      Map(){ data = nullptr;}
      ~Map() { if (data != nullptr) THByteTensor_free(data); }
    };

    class Replayer : public RefCounted {
      private:
        std::vector<Frame*> frames;
        std::unordered_map<int32_t, int32_t> numUnits;
        Map map;

      public:
        ~Replayer() {
          for (auto f: frames) {
            if (f) f->decref();
          }
        }

        Frame* getFrame(int i) {
          if (i < 0 || i >= frames.size()) return nullptr;
          return frames[i];
        }
        void push(Frame* f) {
          f->incref();
          frames.push_back(f);
        }
        size_t size() const {
          return frames.size();
        }

        void setNumUnits() {
          for(const auto f : frames){
            for(auto u : f->units){
              auto s = u.second.size();
              auto i = u.first;
              if(numUnits.count(i) == 0) {
                numUnits[i] = s;
              }
              else if(s > numUnits[i]){
                numUnits[i] = s;
              }
            }
          }
        }

        int32_t getNumUnits(const int32_t& key) const {
          if (numUnits.find(key) == numUnits.end()) return -1;
          return numUnits.at(key);
        }

        void setMap(THByteTensor* data) {
          // free existing map if needed
          if (map.data != nullptr) {
            THByteTensor_free(map.data);
          }
          map.data = THByteTensor_newWithTensor(data);
        }

        void setMap(uint32_t h, uint32_t w, uint8_t* d) {
          // free existing map if needed
          if (map.data != nullptr) {
            THByteTensor_free(map.data);
          }
          auto storage = THByteStorage_newWithData(d, h * w); // refcount 1
          map.data = THByteTensor_newWithStorage2d(storage, 0, h, w, w, 1);
          // storage has been retained by map.data, so decrease refcount back to 1
          THByteStorage_free(storage);
        }

        THByteTensor* getMap(){
          return map.data;
        }

        friend std::ostream& operator<<(std::ostream& out, const Replayer& o);
        friend std::istream& operator>>(std::istream& in, Replayer& o);
    };

  } // namespace replayer
} // namespace torchcraft


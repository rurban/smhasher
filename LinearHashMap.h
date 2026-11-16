#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Fixed-capacity benchmark map. A probe cap bounds work on pathological hashes;
// hitting it invalidates the measurement rather than silently dropping a key.
class LinearHashMap {
public:
  typedef std::function<size_t(const std::string &)> Hasher;
  enum { kProbeLimit = 1024 };

  LinearHashMap(size_t keys, const Hasher &hasher) : hash_(hasher) {
    if (keys > (std::numeric_limits<size_t>::max() - 1) / 2)
      throw std::length_error("linear hashmap capacity overflow");
    size_t capacity = 8;
    const size_t required = keys + keys / 2 + 1;
    while (capacity < required) {
      if (capacity > std::numeric_limits<size_t>::max() / 2)
        throw std::length_error("linear hashmap capacity overflow");
      capacity *= 2;
    }
    slots_.resize(capacity);
  }

  int *get_or_insert(const std::string &key) {
    size_t index = probe(key, true);
    if (index == slots_.size()) return NULL;
    Slot &slot = slots_[index];
    if (slot.state != Occupied) {
      slot.key = key;
      slot.value = 0;
      slot.state = Occupied;
    }
    return &slot.value;
  }

  bool erase(const std::string &key) {
    size_t index = probe(key, false);
    if (index == slots_.size()) return false;
    slots_[index].state = Deleted;
    slots_[index].key.clear();
    return true;
  }

  bool limit_reached() const { return limit_reached_; }
  uint64_t collisions() const { return collisions_; }
  size_t max_probe() const { return max_probe_; }

private:
  enum State { Empty, Occupied, Deleted };
  struct Slot {
    std::string key;
    int value = 0;
    State state = Empty;
  };

  size_t probe(const std::string &key, bool insert) {
    const size_t mask = slots_.size() - 1;
    const size_t start = hash_(key) & mask;
    size_t deleted = slots_.size();
    const size_t limit = std::min<size_t>(kProbeLimit, slots_.size());
    for (size_t n = 0; n < limit; ++n) {
      const size_t index = (start + n) & mask;
      const Slot &slot = slots_[index];
      max_probe_ = std::max(max_probe_, n + 1);
      if (slot.state == Empty) return insert && deleted != slots_.size() ? deleted : (insert ? index : slots_.size());
      if (slot.state == Deleted) {
        if (deleted == slots_.size()) deleted = index;
      } else if (slot.key == key) {
        return index;
      } else {
        ++collisions_;
      }
    }
    limit_reached_ = true;
    return slots_.size();
  }

  Hasher hash_;
  std::vector<Slot> slots_;
  bool limit_reached_ = false;
  uint64_t collisions_ = 0;
  size_t max_probe_ = 0;
};

#pragma once
// Host-test stub: only CallbackManager is needed by variable.h.
#include <functional>
#include <utility>
#include <vector>

namespace esphome {

template<typename... X> class CallbackManager;

template<typename... Ts> class CallbackManager<void(Ts...)> {
 public:
  void add(std::function<void(Ts...)> &&callback) { this->callbacks_.push_back(std::move(callback)); }
  void call(Ts... args) {
    for (auto &cb : this->callbacks_)
      cb(args...);
  }

 protected:
  std::vector<std::function<void(Ts...)>> callbacks_;
};

}  // namespace esphome

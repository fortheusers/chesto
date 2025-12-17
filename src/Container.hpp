#ifndef CONTAINER_H
#define CONTAINER_H
#include "Element.hpp"

#define ROW_LAYOUT 1
#define COL_LAYOUT 2

namespace Chesto {

class Container : public Element {
public:
  Container(int layout = 0, int padding = 10);
  Element *add(std::unique_ptr<Element> elem);

  // override createNode to use Container's add logic instead
  template <typename T, typename... Args> T *createNode(Args &&...args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = ptr.get();
    add(std::move(ptr));
    return rawPtr;
  }

  int layout = 0;
  int padding = 10;
};

} // namespace Chesto

#endif
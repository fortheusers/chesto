#pragma once
#include "Element.hpp"

#include <vector>
class Element;


// clang-format on
#define ALIGN_LEFT 0x01
#define ALIGN_RIGHT 0x02
#define ALIGN_TOP 0x04
#define ALIGN_BOTTOM 0x08
#define ALIGN_CENTER_HORIZONTAL 0x10
#define ALIGN_CENTER_VERTICAL 0x20
#define ALIGN_CENTER_BOTH 0x30

#define OFFSET_LEFT 0x40
#define OFFSET_RIGHT 0x80
#define OFFSET_TOP 0x100
#define OFFSET_BOTTOM 0x200
#define OFFSET_ALL 0x3C0
// clang-format off

class Constraint {
public:
    Constraint(int flags, int padding = 0, std::vector<Element*> targets = {});
    void clearFlags();
    void addFlags(int flags);
    void clearTargets();
    void addTarget(Element* target);
    void update();

    void apply(Element* element);

    /* positioning flags */
    int positioningFlags = 0;

    /* any referenced Elements that this constraint may define itself relative to */
    std::vector<Element*> targets;

    /* the amount to pad or  offset this element by */
    int paddingOffset = 0;
};
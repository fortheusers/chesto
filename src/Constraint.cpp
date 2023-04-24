#include "Constraint.hpp"
#include "RootDisplay.hpp"

/** 
 * Constraints are used to define the positioning of an element relative to other elements.
 * Currently only within another element or padding from a parent edge is supported.
 * TODO: support for positioning relative to non-overlapping elements (eg. the space between)
 * 
 * If no targets are specified, the constraint will be relative to the parent element.
 * 
 * @param flags the positioning flags to use
 * @param targets the elements to position relative to
*/

Constraint::Constraint(int flags, int padding, std::vector<Element*> targets) {
    positioningFlags |= flags;
    paddingOffset = padding;
    this->targets = targets;
}

void Constraint::clearFlags() {
    positioningFlags = 0;
}

void Constraint::addFlags(int flags) {
    positioningFlags |= flags;
}

void Constraint::clearTargets() {
    targets.clear();
}

void Constraint::addTarget(Element* target) {
    targets.push_back(target);
}

void Constraint::apply(Element* element) {
    // if the vector of targets is blank, grab the parent
    // TODO: anything with the vector, to extract a target from it
    auto target = element->parent;
    int posX = 0, posY = 0;
    int width = RootDisplay::screenWidth, height = RootDisplay::screenHeight; // default to screen size
    if (target != NULL) {
        posX = target->x;
        posY = target->y;
        width = target->width;
        height = target->height;
    }

    // look at the flags and decide what to do
    if (positioningFlags & ALIGN_LEFT)     element->x = posX + paddingOffset;
    if (positioningFlags & ALIGN_RIGHT)    element->x = posX + width - element->width - paddingOffset;
    if (positioningFlags & ALIGN_TOP)      element->y = posY + paddingOffset;
    if (positioningFlags & ALIGN_BOTTOM)   element->y = posY + height - element->height - paddingOffset;

    if (positioningFlags & ALIGN_CENTER_HORIZONTAL)  element->x = posX + width / 2  -  element->width / 2;
    if (positioningFlags & ALIGN_CENTER_VERTICAL)    element->y = posY + height / 2 - element->height / 2;

    // some manual offset constraints, that just move the element
    if (positioningFlags & OFFSET_LEFT)    element->x += paddingOffset;
    if (positioningFlags & OFFSET_RIGHT)   element->x -= paddingOffset;
    if (positioningFlags & OFFSET_TOP)     element->y += paddingOffset;
    if (positioningFlags & OFFSET_BOTTOM)  element->y -= paddingOffset;

}
#include "Constraint.hpp"
#include "RootDisplay.hpp"

namespace Chesto {

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
    // if the vector of targets is not empty, use the first target, otherwise use parent
    auto target = targets.empty() ? element->parent : targets[0];
    auto isSpecificTarget = !targets.empty();
    int posX = 0, posY = 0;
    int width = RootDisplay::screenWidth, height = RootDisplay::screenHeight; // default to screen size, TODO: is this good?

    float effectiveScale = element->getEffectiveScale();

    auto paddingOffset = this->paddingOffset; // copy so we don't flicker/modify it on the fly
    
    if (target != NULL) {
        // For centering constraints, we want to position relative to parent's content area (0,0)
        // For edge alignment, we use parent's position as reference
        posX = target->x;
        posY = target->y;
        width = target->width;
        height = target->height;

        if (isSpecificTarget) {
            // with an element target, we have to inverse our padding offset
            paddingOffset = -paddingOffset;

            //also, if it's top or left, we need to account for our own height/width
            if (positioningFlags & ALIGN_TOP)    posY -= (int)(element->height * effectiveScale);
            if (positioningFlags & ALIGN_LEFT)   posX -= (int)(element->width * effectiveScale);
            // if it's bottom or right, we need to account for the target's height/width
            if (positioningFlags & ALIGN_BOTTOM) posY += (int)(target->height * effectiveScale);
            if (positioningFlags & ALIGN_RIGHT)  posX += (int)(target->width * effectiveScale);
        }
    }

    // padding offset needs scale too
    int scaledPadding = (int)(paddingOffset * effectiveScale);

    // look at the flags and decide what to do
    if (positioningFlags & ALIGN_LEFT)     element->x = posX + scaledPadding;
    if (positioningFlags & ALIGN_RIGHT)    element->x = posX + width - (int)(element->width * effectiveScale) - scaledPadding;
    if (positioningFlags & ALIGN_TOP)      element->y = posY + scaledPadding;
    if (positioningFlags & ALIGN_BOTTOM)   element->y = posY + height - (int)(element->height * effectiveScale) - scaledPadding;

    // For centering, position relative to parent's content area (like centerHorizontallyIn)
    if (positioningFlags & ALIGN_CENTER_HORIZONTAL)  element->x = width / 2  -  (int)(element->width * effectiveScale) / 2;
    if (positioningFlags & ALIGN_CENTER_VERTICAL)    element->y = height / 2 - (int)(element->height * effectiveScale) / 2;

    // some manual offset constraints, that just move the element
    if (positioningFlags & OFFSET_LEFT)    element->x += paddingOffset;
    if (positioningFlags & OFFSET_RIGHT)   element->x -= paddingOffset;
    if (positioningFlags & OFFSET_TOP)     element->y += paddingOffset;
    if (positioningFlags & OFFSET_BOTTOM)  element->y -= paddingOffset;

}

} // namespace Chesto
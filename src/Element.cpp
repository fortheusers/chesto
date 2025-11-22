#include "RootDisplay.hpp"
#include <algorithm>
#include "Constraint.hpp"
#include "Animation.hpp"
#include <string>

namespace Chesto {

// Shared deleter for all elements - respects isProtected flag
static void safeElementDeleter(Element* elem) {
	if (elem && !elem->isProtected) {
		delete elem;
	}
}

Element::~Element()
{
	// unique_ptrs will automatically cleaned up
	elements.clear();
	constraints.clear();
	animations.clear();
}

Element::Element()
{
	needsRedraw = true;
}

bool Element::process(InputEvents* event)
{
	// whether or not we need to update the screen
	bool ret = false;

	// if we're hidden, don't process input
	if (hidden) return ret;

	// if 3ds mock, ignore top screen inputs
#ifdef _3DS_MOCK
	if (event->touchIn(0, 0, 400, 240)) return ret;
#endif

	// do any touch down, drag, or up events
	if (touchable)
	{
		ret |= onTouchDown(event);
		ret |= onTouchDrag(event);
		
		bool touchUpHandled = onTouchUp(event);
		ret |= touchUpHandled;
		
		if (touchUpHandled && (action != NULL || actionWithEvents != NULL)) {
			// an action was fired, whicih may delete our element, so return right away
			return true;
		}
	}

	// call process on subelements
	size_t elementCount = this->elements.size();
	for (size_t x = 0; x < elementCount; x++)
	{
		// ensure element still exists before trying to process it
		if (x < this->elements.size() && this->elements[x])
		{
			bool childHandled = this->elements[x]->process(event);
			ret |= childHandled;

			if (childHandled && this->elements.size() != elementCount) {
				// size changed while we were processing, break out
				break;
			}
		}
	}

	ret |= this->needsRedraw;
	this->needsRedraw = false;

	// if this variable is positive, decrease it, and force a redraw (acts like needsRedraw but over X redraws)
	if (futureRedrawCounter > 0) {
		futureRedrawCounter --;
		ret |= true;
	}

	if (RootDisplay::idleCursorPulsing) {
		// if we are using idle cursor pulsing, and this element's elastic counter is 0, force a redraw
		return ret | (this->elasticCounter > 0);
	}

	return ret;
}

void Element::render(Element* parent)
{	
	//if we're hidden, don't render
	if (hidden) return;

	// this needs to happen before any rendering
	this->recalcPosition(parent);

	// if we're in debug mode, draw an outline
	if (this->hasBackground) {
		// render the element background
		this->renderBackground(true);
	}
	else if (RootDisplay::isDebug) {
		backgroundColor = randomColor();
		this->renderBackground(false);
	}

	// go through every subelement and run render
	for (auto& subelement : elements)
	{
		subelement->render(this);
	}

	CST_Renderer* renderer = getRenderer();

	// if we're touchable, and we have some animation counter left, draw a rectangle+overlay
	if (this->touchable && this->elasticCounter > THICK_HIGHLIGHT)
	{
		auto marginSpacing = cornerRadius > 0 ? 0 : 5;
		CST_Rect d = { this->xAbs - marginSpacing, this->yAbs - marginSpacing, this->width + marginSpacing*2, this->height + marginSpacing*2 };
		if (cornerRadius > 0) {
			// draw a rounded highlight instead
			CST_roundedBoxRGBA(renderer, d.x, d.y, d.x + d.w, d.y + d.h,
				cornerRadius, 0x10, 0xD9, 0xD9, 0x40);
		} else {
			CST_SetDrawBlend(renderer, true);
			CST_SetDrawColorRGBA(renderer, 0x10, 0xD9, 0xD9, 0x40);
			CST_FillRect(renderer, &d);
		}
	}

	if (this->touchable && this->elasticCounter > NO_HIGHLIGHT)
	{
		auto marginSpacing = cornerRadius > 0 ? 0 : 5;
		CST_Rect d = { this->xAbs - marginSpacing, this->yAbs - marginSpacing, this->width + marginSpacing*2, this->height + marginSpacing*2 };
		if (this->elasticCounter == THICK_HIGHLIGHT)
		{
			int ticks = CST_GetTicks() / 100;
			int pulseState = ticks % 20;
			if (pulseState > 9) {
				pulseState = 19 - pulseState;
			}

			if (!RootDisplay::idleCursorPulsing) {
				// if we're not using idle cursor pulsing, just draw a simple rectangle
				pulseState = 0;
			}

			// make it a little thicker by drawing more rectangles TODO: better way to do this?
			auto decreaser = cornerRadius > 0 ? 0 : -2;
			for (int x = decreaser; x <= 3; x++)
			{
				// draw a rectangle with varying brightness depending on the pulse state
				int r = 0x10; //- 0x01 * pulseState;
				int g = 0xD9 - 0x01 * pulseState;
				int b = 0xD9 - 0x01 * pulseState;
				int edgeMod = x==1 ? 0 : abs(x); // slight bias towards the inner
				int a = fmax(0x0, 0xFF - 0x10 * pulseState * edgeMod);
				CST_roundedRectangleRGBA(renderer, d.x + x, d.y + x, d.x + d.w - x, d.y + d.h - x, cornerRadius, r, g, b, a);
			}
		} else {
			// simple rectangle, not pulsing
			CST_roundedRectangleRGBA(renderer, d.x, d.y, d.x + d.w, d.y + d.h, cornerRadius, 0x10, 0xD9, 0xD9, 0xFF);
			// and one inner rectangle too
			CST_roundedRectangleRGBA(renderer, d.x + 1, d.y + 1, d.x + d.w - 1, d.y + d.h - 1, cornerRadius, 0x10, 0xD9, 0xD9, 0xFF);
		}
	}
}

void Element::recalcPosition(Element* parent) {
	// go through all constraints and apply them
	for (auto& constraint : constraints)
	{
		constraint->apply(this);
	}

	// calculate any absolute x/y positions after constraints are applied
	if (parent && !isAbsolute)
	{
		this->xAbs = parent->xAbs + this->x;
		this->yAbs = parent->yAbs + this->y;
	} else {
		this->xAbs = this->x;
		this->yAbs = this->y;
	}

	// go through all animations and apply them
	if (animations.size() > 0) {
		std::vector<size_t> toRemove;
		for (size_t i = 0; i < animations.size(); i++)
		{
			auto& animation = animations[i];
			// if there are any animations, we need to re-render
			needsRedraw = true;

			bool finished = animation->step();
			if (finished) {
				toRemove.push_back(i);
			}
		}

		for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
			animations.erase(animations.begin() + *it);
		}
	}
}

CST_Rect Element::getBounds()
{
	return {
		.x = this->xAbs,
		.y = this->yAbs,
		.w = this->width,
		.h = this->height,
	};
}

void Element::renderBackground(bool fill) {
	CST_Renderer* renderer = getRenderer();
	CST_Rect bounds = getBounds();
	auto r = backgroundColor.r * 0xFF;
	auto g = backgroundColor.g * 0xFF;
	auto b = backgroundColor.b * 0xFF;
	
	if (cornerRadius > 0) {
		const auto renderRect = fill ? CST_roundedBoxRGBA : CST_roundedRectangleRGBA;
		renderRect(renderer, bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h,
			cornerRadius, backgroundColor.r * 0xFF, backgroundColor.g * 0xFF, backgroundColor.b * 0xFF, backgroundOpacity);
	} else {
		CST_SetDrawColorRGBA(renderer, r, g, b, backgroundOpacity);
		const auto renderRect = fill ? CST_FillRect : CST_DrawRect;
		renderRect(renderer, &bounds);
	}
}

void Element::position(int x, int y)
{
	this->x = x;
	this->y = y;
}

bool Element::onTouchDown(InputEvents* event)
{
	if (!event->isTouchDown())
		return false;

	if (!event->touchIn(this->xAbs, this->yAbs, this->width, this->height))
		return false;

	// mouse pushed down, set variable
	this->dragging = true;
	this->lastMouseY = event->yPos;
	this->lastMouseX = event->xPos;

	// turn on deep highlighting during a touch down
	if (this->touchable)
		this->elasticCounter = DEEP_HIGHLIGHT;

	return true;
}

bool Element::onTouchDrag(InputEvents* event)
{
	bool ret = false;

	if (!event->isTouchDrag())
		return false;

	// if we're not in a deeplight (a touchdown event), draw our own drag highlight
	if (this->elasticCounter != DEEP_HIGHLIGHT) {
		if (event->touchIn(this->xAbs, this->yAbs, this->width, this->height)) {
			// if there's currently _no_ highlight, and we're in a drag event on this element,
			// so we should turn on the hover highlight
			this->elasticCounter = THICK_HIGHLIGHT;
			ret |= true;

			// play a hover sound and vibrate
			CST_LowRumble(event, 200);

			// change the cursor to a hand
			CST_SetCursor(CST_CURSOR_HAND);
		} else {
			auto initialElasticCounter = this->elasticCounter;

			// we're in a drag event, but not for this element
			this->elasticCounter = NO_HIGHLIGHT;

			if (initialElasticCounter != NO_HIGHLIGHT) {
				// change the cursor back to the arrow
				CST_SetCursor(CST_CURSOR_ARROW);
				ret |= true;
			}

		}
	}

	// minimum amount of wiggle allowed by finger before calling off a touch event
	int TRESHOLD = 40 / SCALER / SCALER;

	// we've dragged out of the icon, invalidate the click by invoking onTouchUp early
	// check if we haven't drifted too far from the starting variable (treshold: 40)
	if (this->dragging && (abs(event->yPos - this->lastMouseY) >= TRESHOLD || abs(event->xPos - this->lastMouseX) >= TRESHOLD))
	{
		ret |= (this->elasticCounter > 0);
		auto prevElasticCounter = this->elasticCounter;
		this->elasticCounter = NO_HIGHLIGHT;
		if (prevElasticCounter != NO_HIGHLIGHT) {
			// change the cursor back to the arrow
			CST_SetCursor(CST_CURSOR_ARROW);
		}
	}

	return ret;
}

bool Element::onTouchUp(InputEvents* event)
{
	if (!event->isTouchUp())
		return false;

	bool ret = false;

	// ensure we were dragging first (originally checked the treshold above here, but now that actively invalidates it)
	if (this->dragging)
	{
		// check that this click is in the right coordinates for this square
		// and that a subscreen isn't already being shown
		// TODO: allow buttons to activae this too?
		if (event->touchIn(this->xAbs, this->yAbs, this->width, this->height))
		{
			// elasticCounter must be nonzero to allow a click through (highlight must be shown)
			if (this->elasticCounter > 0)
			{
				bool wasHighlighted = (this->elasticCounter > 0);
				
				this->dragging = false;
				this->elasticCounter = 0;
				
				// invoke this element's action
				if (action != NULL) {
					this->action();
					// returns early since an action may delete 'this'
					return true;
				}
				if (actionWithEvents != NULL) {
					this->actionWithEvents(event);
					// returns early since an action may delete 'this'
					return true;
				}
				
				// If we get here, we had elasticCounter but no action
				ret |= wasHighlighted;
			}
		}
	}

	// release mouse
	this->dragging = false;

	// update if we were previously highlighted, cause we're about to remove it
	ret |= (this->elasticCounter > 0);

	this->elasticCounter = 0;

	return ret;
}

void Element::append(std::unique_ptr<Element> element)
{
	if (!element) return;
	
	// check if element already exists (by raw pointer comparison)
	Element* rawPtr = element.get();
	for (const auto& existing : elements) {
		if (existing.get() == rawPtr) {
			return;
		}
	}
	
	// convert to unique_ptr with custom deleter that respects isProtected flag
	Element* ptr = element.release();
	std::unique_ptr<Element, std::function<void(Element*)>> convertedElement(
		ptr,
		safeElementDeleter
	);
	
	ptr->parent = this;
	elements.push_back(std::move(convertedElement));
}

void Element::appendProtected(Element* element)
{
	if (!element) return;
	
	// check if element already exists
	for (const auto& existing : elements) {
		if (existing.get() == element) {
			return;
		}
	}
	
	element->parent = this;
	element->isProtected = true; // Mark as protected
	
	// For protected (stack-allocated) elements, use the shared safe deleter
	// the deleter checks isProtected flag and skips deletion
	elements.push_back(std::unique_ptr<Element, std::function<void(Element*)>>(
		element,
		safeElementDeleter
	));
}


void Element::remove(Element *element)
{
	// single element remove

	auto position = std::find_if(elements.begin(), elements.end(),
		[element](const std::unique_ptr<Element, std::function<void(Element*)>>& e) { 
			return e.get() == element; 
		});
	if (position != elements.end())
		elements.erase(position);
}

void Element::wipeAll(bool delSelf)
{
	// Deletes all children (unique_ptr handles the deletion)
	elements.clear();

	if (delSelf && !isProtected) {
		delete this; // (...is this ok?)
	}
}

void Element::removeAll(bool moveToTrash)
{
	// unique_ptrs automatically handle clean up
	elements.clear();
	constraints.clear();
	animations.clear();
}

Element* Element::child(std::unique_ptr<Element> child)
{
	if (child) {
		child->parent = this;
		child->recalcPosition(this);
		
		// convert to unique_ptr with shared safe deleter
		// TODO: reconcile child() and append() methods
		Element* ptr = child.release();
		elements.push_back(std::unique_ptr<Element, std::function<void(Element*)>>(
			ptr,
			safeElementDeleter
		));
	}
	return this;
}

Element* Element::setPosition(int x, int y)
{
	this->position(x, y);
	return this;
}

Element* Element::setAction(std::function<void()> func)
{
	this->action = func;
	return this;
}

Element* Element::centerHorizontallyIn(Element* parent)
{
	this->x = parent->width / 2 - this->width / 2;
	return this;
}

Element* Element::centerVerticallyIn(Element* parent)
{
	this->y = parent->height / 2 - this->height / 2;
	return this;
}

Element* Element::centerIn(Element* parent)
{
	return centerHorizontallyIn(parent)->centerVerticallyIn(parent);
}

Element* Element::setAbsolute(bool isAbs)
{
	isAbsolute = isAbs;
	return this;
}

CST_Renderer* Element::getRenderer() {
	return RootDisplay::renderer;
}

Element* Element::constrain(int flags, int padding)
{
	constraints.push_back(std::make_unique<Constraint>(flags, padding));
	return this;
}

Element* Element::constrainToTarget(Element* target, int flags, int padding)
{
	constraints.push_back(std::make_unique<Constraint>(flags, padding, std::vector<Element*>{target}));
	return this;
}


Element* Element::animate(
	int duration,
	std::function<void(float)> onStep,
	std::function<void()> onFinish
) {
	animations.push_back(std::make_unique<Animation>(
		CST_GetTicks(),	duration, onStep, onFinish)
	);

	return this;
}

// Move an element up within its parent
Element* Element::moveToFront() {
	if (parent != NULL) {
		// lookup this element in parent's vector
		auto position = std::find_if(parent->elements.begin(), parent->elements.end(),
			[this](const std::unique_ptr<Element, std::function<void(Element*)>>& e) { 
				return e.get() == this; 
			});
		
		if (position != parent->elements.end()) {
			// move it to the end
			auto elem = std::move(*position);
			parent->elements.erase(position);
			parent->elements.push_back(std::move(elem));
		}
	}
	return this;
}

Element* Element::setTouchable(bool touchable)
{
	this->touchable = touchable;
	return this;
}

void Element::screenshot(std::string path) {
    // render the webview to a target that can be saved (TARGET ACCESS)
	CST_Texture* target = SDL_CreateTexture(getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);

	// set the target texture
	SDL_SetRenderTarget(getRenderer(), target);

    // draw a white background first
    SDL_SetRenderDrawColor(getRenderer(), 255, 255, 255, 255);
    SDL_RenderClear(getRenderer());

	// render the texture
    render(parent);

	// reset the target texture
	SDL_SetRenderTarget(getRenderer(), NULL);

	// save the surface to the path
	CST_SavePNG(target, path.c_str());
}

} // namespace Chesto
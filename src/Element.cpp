#include "RootDisplay.hpp"
#include <algorithm>
#include "Constraint.hpp"
#include "Animation.hpp"
#include <string>

Element::~Element()
{
	removeAll();
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
		ret |= onTouchUp(event);
	}

	// call process on subelements
	for (int x = 0; x < this->elements.size(); x++)
		if (this->elements.size() > x && this->elements[x])
			ret |= this->elements[x]->process(event);

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
	for (Element* subelement : elements)
	{
		subelement->render(this);
	}

	CST_Renderer* renderer = getRenderer();

	// if we're touchable, and we have some animation counter left, draw a rectangle+overlay
	if (this->touchable && this->elasticCounter > THICK_HIGHLIGHT)
	{
		CST_Rect d = { this->xAbs - 5, this->yAbs - 5, this->width + 10, this->height + 10 };
		CST_SetDrawBlend(renderer, true);
		CST_SetDrawColorRGBA(renderer, 0x10, 0xD9, 0xD9, 0x40);
		CST_FillRect(renderer, &d);
	}

	if (this->touchable && this->elasticCounter > NO_HIGHLIGHT)
	{
		CST_Rect d = { this->xAbs - 5, this->yAbs - 5, this->width + 10, this->height + 10 };
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
			for (int x = -2; x <= 3; x++)
			{
				// draw a rectangle with varying brightness depending on the pulse state
				int r = 0x10; //- 0x01 * pulseState;
				int g = 0xD9 - 0x01 * pulseState;
				int b = 0xD9 - 0x01 * pulseState;
				int edgeMod = x==1 ? 0 : abs(x); // slight bias towards the inner
				int a = fmax(0x0, 0xFF - 0x10 * pulseState * edgeMod);
				CST_rectangleRGBA(renderer, d.x + x, d.y + x, d.x + d.w - x, d.y + d.h - x, r, g, b, a);
			}
		} else {
			// simple rectangle, not pulsing
			CST_rectangleRGBA(renderer, d.x, d.y, d.x + d.w, d.y + d.h, 0x10, 0xD9, 0xD9, 0xFF);
			// and one inner rectangle too
			CST_rectangleRGBA(renderer, d.x + 1, d.y + 1, d.x + d.w - 1, d.y + d.h - 1, 0x10, 0xD9, 0xD9, 0xFF);
		}
	}
}

void Element::recalcPosition(Element* parent) {
	// calculate absolute x/y positions
	if (parent && !isAbsolute)
	{
		this->xAbs = parent->xAbs + this->x;
		this->yAbs = parent->yAbs + this->y;
	} else {
		this->xAbs = this->x;
		this->yAbs = this->y;
	}

	// go through all constraints and apply them
	for (Constraint* constraint : constraints)
	{
		constraint->apply(this);
	}

	// go through all animations and apply them
	// TODO: animations can modify the actual positions, which can mess up constraints
	if (animations.size() > 0) {
		std::vector<Animation*> toRemove;
		for (Animation* animation : animations)
		{
			// if there are any animations, we need to re-render
			needsRedraw = true;

			bool finished = animation->step();
			if (finished) {
				toRemove.push_back(animation);
			}
		}
		for (Animation* animation : toRemove) {
			animations.erase(std::remove(animations.begin(), animations.end(), animation), animations.end());
			delete animation;
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
	CST_SetDrawColorRGBA(renderer,
		static_cast<Uint8>(backgroundColor.r * 0xFF),
		static_cast<Uint8>(backgroundColor.g * 0xFF),
		static_cast<Uint8>(backgroundColor.b * 0xFF),
		0xFF
	);
	CST_SetDrawBlend(renderer, false);
	const auto RenderRect = fill ? CST_FillRect : CST_DrawRect;
	RenderRect(renderer, &bounds);
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
		this->elasticCounter = NO_HIGHLIGHT;
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
				// invoke this element's action
				if (action != NULL) {
					this->action();
					ret |= true;
				}
				if (actionWithEvents != NULL) {
					this->actionWithEvents(event);
					ret |= true;
				}
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

void Element::append(Element *element)
{
	auto position = std::find(elements.begin(), elements.end(), element);
	if (position == elements.end()) {
		elements.push_back(element);
	}
}

void Element::remove(Element *element)
{
	auto position = std::find(elements.begin(), elements.end(), element);
	if (position != elements.end())
		elements.erase(position);
}

void Element::wipeAll(bool delSelf)
{
	// delete's this element's children, then itself
	for (auto child : elements) {
		child->wipeAll(true);
	}
	elements.clear();

	if (delSelf && !isProtected) {
		delete this;
	}
}

void Element::removeAll(bool moveToTrash)
{
	if (moveToTrash) {
		// store in a list for free-ing up memory later
		for (auto e : elements) {
			RootDisplay::mainDisplay->trash.push_back(e);
		}
	}
	elements.clear();
	constraints.clear(); // remove all constraints too
	animations.clear();
}

Element* Element::child(Element* child)
{
	this->elements.push_back(child);
	child->parent = this;
	child->recalcPosition(this);
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
	constraints.push_back(new Constraint(flags, padding));
	return this;
}

Element* Element::animate(
	int duration,
	std::function<void(float)> onStep,
	std::function<void()> onFinish
) {
	animations.push_back(new Animation(
		CST_GetTicks(),	duration, onStep, onFinish)
	);

	return this;
}

// Move an element up within its parent
Element* Element::moveToFront() {
	if (parent != NULL) {
		parent->remove(this);
		parent->child(this);
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
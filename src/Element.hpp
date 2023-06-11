#pragma once

#include "InputEvents.hpp"
#include "colorspaces.hpp"
#include "DrawUtils.hpp"

#include <functional>
#include <vector>

#define DEEP_HIGHLIGHT 200
#define THICK_HIGHLIGHT 150
#define HIGHLIGHT 100
#define NO_HIGHLIGHT 0

#if defined(USE_RAMFS)
#define RAMFS "resin:/"
#else
#define RAMFS "resin/"
#endif

#if defined(_3DS) || defined(_3DS_MOCK)
#define SCALER 2
#else
#define SCALER 1
#endif

class Constraint;

class Element
{
public:
	Element();
	virtual ~Element();

	/// process any input that is received for this element
	virtual bool process(InputEvents* event);

	/// display the current state of the display
	virtual void render(Element* parent);

	// invoked on touchdown/up events
	bool onTouchDown(InputEvents* event);
	bool onTouchDrag(InputEvents* event);
	bool onTouchUp(InputEvents* event);

	// hide the element
	void hide() { this->hidden = true; }
	// unhide the element
	void unhide() { this->hidden = false; }

	// render the element's background
	void renderBackground(bool fill = true);

	/// the action to call (from binded callback) on touch or button selection
	/// https://stackoverflow.com/questions/14189440/c-class-member-callback-simple-examples
	std::function<void()> action;
	std::function<void(InputEvents* event)> actionWithEvents;

	/// visible GUI child elements of this element
	std::vector<Element*> elements;

	void append(Element* element);
	void remove(Element* element);
	void removeAll(bool moveToTrash = false);

	/// position the element
	void position(int x, int y);

	// recalculate xAbs and yAbs based on the given parent
	void recalcPosition(Element* parent);

	// the scale of the element (and its subelements!)
	float scale = 1.0f;

	/// whether or not this element can be touched (highlights bounds)
	bool touchable = false;

	/// whether or not this element is currently being dragged
	bool dragging = false;

	/// whether or not this element needs the screen redrawn next time it's processed
	bool needsRedraw = false;

	/// whether this element needs a redraw for the next X redraws (decreases each time) (0 is no redraws)
	int futureRedrawCounter = 0;

	/// the last Y, X coordinate of the mouse (from a drag probably)
	int lastMouseY = 0, lastMouseX = 0;

	// whether this element has a background
	bool hasBackground = false;

	// the color of the background
	rgb backgroundColor = {0, 0, 0};

	// if this element should ignore parent position and just position itself
	bool isAbsolute = false;

	/// the parent element (can sometimes be null if it isn't set)
	Element* parent = NULL;

	/// whether this element should skip rendering or not
	bool hidden = false;

	// bounds on screen of this element
	CST_Rect getBounds();

	// whether or not this should be automatically free'd by wipeAll
	bool isProtected = false;

	/// how much time is left in an elastic-type flick/scroll
	/// set by the last distance traveled in a scroll, and counts down every frame
	int elasticCounter = 0;

	/// width and height of this element (must be manually set, isn't usually calculated (but is in some cases, like text or images))
	int width = 0, height = 0;

	typedef Element super;

	// position relative to parent (if given) or screen (NULL parent)
	int x = 0, y = 0;

	// actual onscreen position (calculated at render time)
	int xAbs = 0, yAbs = 0;
	/// rotation angle in degrees
	double angle = 0;

	// x and y offsets (can be used for drawing relative to other elements)
	int xOff = 0, yOff = 0;

	// internal get current renderer or default one
	CST_Renderer* getRenderer();

	// delete this element's children, and their children, and so on
	// if you've been using `new` everywhere, calling this can make sense in your
	// top level component's destructor to free all the memory from that branch
	// (not automatically invoked in case implementor wants to manage it)
	void wipeAll(bool delSelf = false);

	// fun chain-able wrappers to some fields, returns back the same element
	Element* child(Element* child);
	Element* setPosition(int x, int y);
	Element* setAction(std::function<void()> func);

	// alignment chainers
	Element* centerHorizontallyIn(Element* parent);
	Element* centerVerticallyIn(Element* parent);
	Element* centerIn(Element* parent);
	Element* setAbsolute(bool isAbs);

	// constraints that can be added and used by positioning functions
	std::vector<Constraint*> constraints;
	Element* constrain(int flags, int padding = 0);

	Element* moveToFront();
	Element* setTouchable(bool touchable);

	// a function to call to re-align according to all constraints
	// std::function<void()> alignmentCommands;
};

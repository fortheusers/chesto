#pragma once

#include "InputEvents.hpp"
#include "colorspaces.hpp"
#include "DrawUtils.hpp"
#include "Animation.hpp"

#include <functional>
#include <vector>
#include <string>
#include <memory>

#define DEEP_HIGHLIGHT 200
#define THICK_HIGHLIGHT 150
#define HIGHLIGHT 100
#define NO_HIGHLIGHT 0

#if defined(USE_RAMFS)
#define RAMFS "resin:/"
#else
#define RAMFS "resin/"
#endif

namespace Chesto {

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
	std::function<void()> action = NULL;
	std::function<void(InputEvents* event)> actionWithEvents = NULL;

	/// visible GUI child elements of this element
	std::vector<std::unique_ptr<Element, std::function<void(Element*)>>> elements;

	// add already-built node to tree and transfer ownership to parent
	void addNode(std::unique_ptr<Element> node);
	
	// remove specific child by pointer
	void remove(Element* element);
	
	// clear all children, constraints, and animations
	void removeAll();

protected:
	// Internal helper for stack-allocated members
	void addStackMember(Element* element);

public:

	/// position the element
	void position(int x, int y);

	// recalculate xAbs and yAbs based on the given parent
	void recalcPosition(Element* parent);
	
	// the effective scale for this element after global scaling
	float getEffectiveScale() const;

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

	// opacity of the background (0-255)
	int backgroundOpacity = 0xff;

	// if this element should ignore parent position and just position itself
	bool isAbsolute = false;

	/// the parent element (reference only, not owned)
	Element* parent = nullptr;

	/// whether this element should skip rendering or not
	bool hidden = false;

	// bounds on screen of this element
	CST_Rect getBounds();

	// whether this element is protected from automatic deletion logic TODO: do we sitl lneed this?
	bool isProtected = false;

	/// how much time is left in an elastic-type flick/scroll
	/// set by the last distance traveled in a scroll, and counts down every frame
	int elasticCounter = 0;

	/// width and height of this element (must be manually set, isn't usually calculated (but is in some cases, like text or images))
	int width = 0, height = 0;

	typedef Element super;

	// relative position within parent (set by user or constraints)
	int x = 0, y = 0;

	// actual onscreen position (calculated by recalcPosition)
	// these should be read-only, not set directly
	int xAbs = 0, yAbs = 0;
	
	/// rotation angle in degrees
	double angle = 0;

	// corner radius (when non-zero, backgrounds, textures, and rectangles are rounded)
	int cornerRadius = 0;

	// user-definable tag for finding elements (defaults to 0)
	int tag = 0;

	// internal get current renderer or default one
	CST_Renderer* getRenderer();

	// fun chain-able wrappers to some fields, returns back the same element
	Element* setPosition(int x, int y);
	Element* setAction(std::function<void()> func);
	Element* setAbsolute(bool isAbs);
	
	// Create and add node to tree in one step
	// Returns raw pointer for convenience
	template<typename T, typename... Args>
	T* createNode(Args&&... args) {
		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		T* rawPtr = ptr.get();
		addNode(std::move(ptr));
		return rawPtr;
	}

	// constraints that can be added and used by positioning functions
	std::vector<std::unique_ptr<Constraint>> constraints;
	Element* constrain(int flags, int padding = 0);
	Element* constrainToTarget(Element* target, int flags, int padding = 0);

	// animations that can be added and will tween over time (and remove when finished)
	std::vector<std::unique_ptr<Animation>> animations;
	Element* animate(
		int durationIn,
		std::function<void(float)> onStep,
		std::function<void()> onFinish
	);

	Element* moveToFront();
	Element* setTouchable(bool touchable);

	/// Take a screenshot of this element and its children, and save it to the given path
	void screenshot(std::string path);

	/// whether or not to overlay a color mask on top of this element
	bool useColorMask = false;

	/// The color to overlay on top
	CST_Color maskColor = {0,0,0,0};

	// a function to call to re-align according to all constraints
	// std::function<void()> alignmentCommands;
};

} // namespace Chesto

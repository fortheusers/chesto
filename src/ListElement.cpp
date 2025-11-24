#include "ListElement.hpp"

namespace Chesto {

bool ListElement::process(InputEvents* event)
{
	bool ret = false;

	// if we're hidden, don't process input
	if (hidden) return ret;

	// perform inertia scrolling for this element
	ret |= this->handleInertiaScroll(event);

	ret |= super::process(event);

	return ret;
}

bool ListElement::processUpDown(InputEvents* event)
{
	bool ret = false;
	int SPEED = 1;

	// handle up and down for the scroll view
	if (event->isKeyDown())
	{
		// scroll the view by offsetting the elastic counter
		event->wheelScroll += (SPEED * event->held(UP_BUTTON) - SPEED * event->held(DOWN_BUTTON));

		ret |= event->held(UP_BUTTON) || event->held(DOWN_BUTTON);
	}

	return ret;
}

bool ListElement::handleInertiaScroll(InputEvents* event)
{
	bool ret = false;
	ListElement* elem = this;

	if (event->isTouchDown())
	{
		// make sure that the mouse down's X coordinate is over the app list (not sidebar)
		if (event->xPos < elem->xAbs)
			return false;

		// saw mouse down so set it in our element object
		elem->dragging = true;
		elem->lastMouseY = event->yPos;
		elem->initialTouchDown = event->yPos;

		ret |= true;
	}
	// drag event for scrolling up or down
	else if (event->isTouchDrag())
	{
		if (elem->dragging)
		{
			// prevent scrolling until we exceed a treshold distance in the Y direction
			if (this->initialTouchDown >= 0 && (abs(event->yPos - this->initialTouchDown) < 10 * getEffectiveScale()))
				return false;

			this->initialTouchDown = -1;

			int distance = event->yPos - elem->lastMouseY;
			elem->y += distance;
			elem->lastMouseY = event->yPos;

			// use the last distance as the rubber band value
			elem->elasticCounter = distance;

			ret |= true;
		}
	}
	else if (event->isTouchUp())
	{
		// mouse up, no more mouse down (TODO: fire selected event here)
		elem->dragging = false;

		// if the scroll offset is less than the total number of apps
		// (put on the mouse up to make it "snap" when going out of bounds)
		// TODO: account for max number of apps too (prevent scrolling down forever)
		if (elem->y > minYScroll)
			elem->y = minYScroll;

		ret |= true;
	}

	// if mouse is up, and there's some elastic counter left, burn out remaining elastic value
	if (!elem->dragging && elem->elasticCounter != 0)
	{
		elem->y += elem->elasticCounter;

		// reduce the elastic counter by 5% each time (slows down the scroll)
		elem->elasticCounter *= 0.95;
		
		// if we're less than 10 pixels on the counter, just stop
		if (abs(elem->elasticCounter) < 10)
			elem->elasticCounter = 0;

		// TODO: same problem as above todo, also extract into method?
		if (elem->y > minYScroll)
			elem->y = minYScroll;

		ret |= true;
	}

	if (ret) event->isScrolling = true;

	if (event->wheelScroll != 0)
	{
		// apply wheel scroll directly to y position, and then reset
		elem->y += event->wheelScroll * 10;
		if (elem->y > minYScroll)
			elem->y = minYScroll;
		event->wheelScroll *= 0.95;

		if (abs(event->wheelScroll) < 0.1)
			event->wheelScroll = 0;
			
		ret |= true;
	}

	return ret;
}

} // namespace Chesto

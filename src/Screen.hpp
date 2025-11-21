#pragma once

#include "Element.hpp"

namespace Chesto {

/**
 * Screen - Base class for full-screen UI elements
 * 
 * Represents a global screen/view in the application. Screens are expected to:
 * - Take up the full screen dimensions
 * - Implement rebuildUI() for dynamic reconstruction (theme changes, language changes)
 * - Be managed via unique_ptr by RootDisplay
 * - Own all their UI elements via the elements vector (no raw pointer members)
 * 
 * Screens should not store raw pointers to child UI elements. Instead, build the
 * UI in rebuildUI() and let the elements vector own everything. If you need to
 * access specific elements later, use the elements vector with known indices.
 */
class Screen : public Element
{
public:
	Screen();
	virtual ~Screen();

	/**
	 * Rebuild the entire UI for this screen.
	 * Called on initialization and whenever the screen needs to be reconstructed
	 * (e.g., theme changes, language changes).
	 * 
	 * Implementations should:
	 * 1. Call removeAll() to clear existing UI
	 * 2. Create all UI elements with std::make_unique
	 * 3. Configure elements (position, actions, etc.)
	 * 4. Transfer ownership via append(std::move(...))
	 */
	virtual void rebuildUI() = 0;

protected:
	// Helper to get full screen dimensions
	int getScreenWidth() const;
	int getScreenHeight() const;
};

} // namespace Chesto

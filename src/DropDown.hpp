#include "Button.hpp"
#include <map>
#include <functional>
#include <string>

class DropDown : public Element
{
public:
	DropDown(std::map<std::string, std::string> choices, std::function<void(std::string)> action);
	bool process(InputEvents* event);
	void render(Element* parent);
};
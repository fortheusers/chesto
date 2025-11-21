#include <string>
#include <functional>
#include "TextElement.hpp"
#include "Container.hpp"

namespace Chesto {

class AlertDialog : public Element {
public:
    AlertDialog(const std::string& title, const std::string& message);

    void show();

    // can be set by the user
    std::string title;
    std::string message;
    std::function<void()> onConfirm;
    std::function<void()> onCancel;
    bool useAnimation = true;

    void setText(const std::string& newText);

    CST_Color blackColor = CST_Color{0, 0, 0, 0xff}; // default text color
    
    // Raw pointers to children (owned by elements vector)
    TextElement* messageText = nullptr;
    Element* overlay = nullptr;
    Container* vStack = nullptr;

    // max sizes of the inner dialogue (not the whole screen-covering element)
    int dialogWidth = 450;
    int dialogHeight = 200;

    // overridden lifecycle methods
    virtual void render(Element* parent) override;
    virtual bool process(InputEvents* event) override;
};

} // namespace Chesto
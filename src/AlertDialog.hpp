#include <string>
#include <functional>
#include "TextElement.hpp"
#include "Container.hpp"

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
    TextElement* messageText = new TextElement("(Placeholder text)", 20, &blackColor, NORMAL, 400);
    Element* overlay = new Element();
    Container* vStack = new Container(COL_LAYOUT, 50);

    // max sizes of the inner dialogue (not the whole screen-covering element)
    int dialogWidth = 450;
    int dialogHeight = 200;

    // overridden lifecycle methods
    virtual void render(Element* parent) override;
    virtual bool process(InputEvents* event) override;
};
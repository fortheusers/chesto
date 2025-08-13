#include "AlertDialog.hpp"
#include "Container.hpp"
#include "Constraint.hpp"
#include "Button.hpp"

AlertDialog::AlertDialog(const std::string& title, const std::string& message)
    : title(title), message(message)
{
    /**
     *    AlertDialog
     *    - Overlay (full screen dimmer, 100% width/height)
     *      - VStack (border of window, centered with light bg, dialog dimensions)
     *         - VStack (inner content, sized to fit the inner content)
     *            - TextElement (message text, width to the inner content)
     *            - Button (OK button)  
     */
    hidden = true;

    messageText->setText(message);
    messageText->update(); // to set the size

    // just add the single button for now
    auto okButton = (new Button("OK", A_BUTTON));
    okButton->setAction([this]() {
        if (onConfirm) onConfirm();
    });
    okButton->cornerRadius = 10;
    okButton->updateBounds();

    auto innerVStack = new Container(COL_LAYOUT, 50);
    innerVStack->add(messageText);
    innerVStack->add(okButton);
    innerVStack->width = dialogWidth;
    // innerVStack->backgroundColor = fromRGB(0xff, 0, 0);
    // innerVStack->hasBackground = true;

    messageText->constrain(ALIGN_CENTER_HORIZONTAL, 0);
    okButton->constrain(ALIGN_CENTER_HORIZONTAL, 0);

    // prompt background color
    vStack->backgroundColor = fromRGB(0xdd, 0xdd, 0xdd);
    vStack->hasBackground = true;
    vStack->cornerRadius = 15;
    vStack->width = dialogWidth;
    vStack->height = dialogHeight;
    vStack->add(innerVStack);
    
    innerVStack->constrain(ALIGN_CENTER_BOTH, 0);

    // overlay and shade bg color
    overlay->width = RootDisplay::mainDisplay->width;
    overlay->height = RootDisplay::mainDisplay->height;
    overlay->backgroundColor = fromRGB(0, 0, 0);
    overlay->backgroundOpacity = 0x00;
    overlay->cornerRadius = 1; // forces transparency to render properly (via sdl_gfx)
    overlay->hasBackground = true;
    
    overlay->child(vStack);

    vStack->constrain(ALIGN_CENTER_BOTH, 0);

    this->child(overlay);
}

void AlertDialog::setText(const std::string& newText) {
    messageText->setText(newText);
    messageText->update();
}

void AlertDialog::show() {
    // we have to go from being 100% transparent and small size to being opaque and full size
    // TODO: need opacity that affects all children elements
    hidden = false;

    if (useAnimation) {
        // start animation
        animate(250, [this](float progress) {
            // on step
            this->vStack->width = (dialogWidth * progress);
            this->vStack->height = (dialogHeight * progress);
            this->overlay->backgroundOpacity = (int)(0x80 * progress);
        }, [this]() {
            this->vStack->width = dialogWidth;
            this->vStack->height = dialogHeight;
            this->overlay->backgroundOpacity = 0x80;
        });
        return;
    }

    // no animation, just do it!
    // this->setVisible(true);
    this->width = dialogWidth;
    this->width = dialogHeight;

}

void AlertDialog::render(Element* parent) {
    // Implementation for rendering the dialog
    super::render(this);
}

bool AlertDialog::process(InputEvents* event) {
    // Implementation for processing input events
    return super::process(event);
}

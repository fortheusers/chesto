# Chesto
[![GPLv3 License](https://img.shields.io/badge/license-GPLv3-blue.svg?style=flat-square)](https://opensource.org/licenses/GPL-3.0)

![logo](logo.png)

**Chesto** is a declarative and element-based library for creating user interfaces using SDL2. It borrows some design, syntax, and lifecycle philosophies from [React](https://github.com/facebook/react) and [libgui](https://github.com/Maschell/libgui).

Powering the UI for [hb-appstore](https://github.com/vgmoose/hb-appstore) and [vgedit](https://github.com/vgmoose/vgedit), it supports touch screen and gamepad controls, and currently targets Wii U, Switch, and PC.

It is named after the [Chesto Berry](https://bulbapedia.bulbagarden.net/wiki/Chesto_Berry), as it seems to prevent sleep while working on it!

> Eating it makes you sleepless. It prevents the sleep status condition and other sleep-related status conditions.

## Elements
All UI objects in Chesto extend the base [Element](src/Element.hpp) class. This class provides some lifecycle functions, as well as providing the ability to process input, and add relative children elements.

### Lifecycle
The [RootDisplay](src/RootDisplay.hpp) class (or a subclass of it) itself extends the base Element. After calling the constructor, you should build and add the desired subclass'd Elements of yours for the first view.

The order of operations is as follows:
- RootDisplay's constructor will initialize SDL2 and other required libraries (networking, etc)
- You set up the initial Elements + Pages as children of RootDisplay (via subclassing)
- The app will run its main loop until exited (forever):
    - [InputEvents](src/InputEvents.hpp) will be collected (generalizes touch+mouse+keyboard+gamepad into this class)
    - For all children elements of RootDisplay:
        - `bool process(InputEvents* event)` method is invoked, giving that child an opportunity to respond to any new input.
            - The children, if they have their own children, will recursively have `process` invoked on them as well.
            - Should return true if the Element is taking action based on some of the InputEvents.
        - `void render (Element* parent)` method is invoked, only if at least one Element returned true during  `process`.
            - Recursively `render` is called on these children's children
            - Can optionally be given a parent Element to use to relatively position the child.
    - The app waits (SDL_Delay) for the next frame (up to 16ms, to approach 60fps), then returns back to the start of the main loop.

Due to this method of `process`ing, and then `render`ing only if at least one Element in the hierarchy responded to InputEvents, the app should use very little CPU if nothing on the screen is being actively changed/animated.

On top of being able to subclass Element to create groups of other custom Elements laid out how you want them, Chesto also includes some stock elements that have convenient behavior, to be detailed below.

### Base Functionality
The base Element class provides super `process` and `render` methods that go through the children Elements and take care of propogating their invocations to the children's children. If `touchable` is set on the Element, a few touch events (`onTouchDown`, `onTouchDrag`, and `onTouchUp`) will automatically be handled.

If a touch event is successfully received, the Element will be highlighted and the bound `action` will be invoked, which must be set ahead of time by the subclassing Element. For an example of how this looks, see the Button section.

`wipeElements()` can also be called on an Element to completely remove and de-allocate its children, for instance to replace lists of elements, or clean up old elements between page changes.

### Drawing Images
The [ImageElement](src/ImageElement.hpp) class can be used to display images either from disk or the network. For example, a romfs image can be instantiated and positioned like this at coordinates (5, 10) relative to the current Element (`this`):

```C++
ImageElement* icon = new ImageElement(ROMFS "res/icon.png");
icon->position(this->x + 5, this->y + 10);
icon->resize(30, 30);
this->elements.push_back(icon);
```

**TODO:** Network image example (with fallback disk path)

### Drawing Text
The [TextElement](src/TextElement.hpp) class is used to display sentences or paragraphs of text, with or without wrappinig. Text can be instantiated at (40, 20) relative to the current Element:

```C++
SDL_Color gray = { 80, 80, 80, 0xff };
int fontSize = 12;
TextElement* status = new TextElement("All good here!", fontSize, &gray);
status->position(this->x + 40, this->y + 20);
this->elements.push_back(status);
```

**TODO:** Use our own colors instead of exposing SDL_Color

**TODO:** Show how to query sizes of text elements (accessible via SDL's `textSurface`)

### Scrollable Views
The [ListElement](src/ListElement.hpp) class should be subclassed and used to contain other groups that automatically need to be presented in a format so that they can trail off the page and be scrolled through either via touch events or gamepad buttons.

You can provide your own cursor logic here as well by overriding `process` in the LisitElement subclass, if you want the gamepad controls to do more than just scroll the page. It should play nicely with sub-elements that are marked as touchable.

For examples of how this class can be used, see hb-appstore's `gui/AppDetails.cpp` and `gui/AppList.cpp`, who both take different approaches to how the user interactes with the view.

### Binding Buttons
The [Button](src/Button.hpp) class automatically subclasses Element and bundles together a TextElement, as well as some touch/gamepad input handling. Like other Elements, the `action` callback can be set to a member function of another class or function (see [here](https://stackoverflow.com/questions/14189440/c-class-member-callback-simple-examples) for more info), which will be invoked either when the InputEvent gamepad is triggered or the button is touched.

To create a button, give it the text, the button which corresponds to it, whether it's light or dark themed, and a font size. It can optionally also take a width as the last element, otherwise it will automatically fit the width to the inner text.

```C++
Button* start = new Button("Begin!", START_BUTTON, true, 20);
start->position(70, 50);
start->action = std::bind(&TitlePage::launch, this);
this->elements.push_back(start);
```

### Displaying Progress
The [ProgressBar](src/ProgressBar.hpp) element takes a `percent` float between 0.0 and 1.0, as well as an overall width for how long the progress bar should be at 100%.

Can be created as follows. If `dimBg` is set, then the entire screen under this progress bar will be covered with a transparent gray sheet.

```C++
ProgressBar pbar = new ProgressBar();
pbar->width = 740;
pbar->position(1280 / 2 - pbar->width / 2, 720 / 2);
pbar->color = 0xff0000ff;
pbar->dimBg = true;
this->elements.push_back(pbar);
```

**TODO:** Show how to provide a callback to networking utilities to update the progress bar rather than always reacting to an input.

### Networking Helpers

**TODO:** Provide some helper functions or examples talking to the network via CURL and interacting with the UI.

## License
This software is licensed under the GPLv3.

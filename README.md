# Chesto [![GPLv3 License](https://img.shields.io/badge/license-GPLv3-blue.svg?style=flat-square)](https://opensource.org/licenses/GPL-3.0)

![logo](logo.png)

**Chesto** is a declarative and element-based library for creating user interfaces using SDL2. It borrows some design, syntax, and lifecycle philosophies from [React](https://github.com/facebook/react), [libgui](https://github.com/Maschell/libgui), and [SwiftUI](https://developer.apple.com/swiftui/).

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

**TODO:** Add image examples of each code sample!

### Base Functionality
The base Element class provides super `process` and `render` methods that go through the children Elements and take care of propogating their invocations to the children's children. If `touchable` is set on the Element, a few touch events (`onTouchDown`, `onTouchDrag`, and `onTouchUp`) will automatically be handled.

If a touch event is successfully received, the Element will be highlighted and the bound `action` will be invoked, which must be set ahead of time by the subclassing Element. For an example of how this looks, see the Button section.

`removeAll()` can also be called on an Element to completely remove its children, for instance to replace lists of elements, or clean up old elements between page changes.

### Drawing Images
The [ImageElement](src/ImageElement.hpp) class can be used to display images either from disk or the network. For example, a romfs image can be instantiated like this:

```C++
auto icon = addNode<ImageElement>(RAMFS "res/icon.png");
```

### Network Images
The [NetworkImageElement](src/NetImageElement.cpp) class can be used to display images downloaded from the internet. It downloads the image in the background, and will automatically update the displayed image once the download is complete. A fallback can also be provided:

```C++

auto netIcon = addNode<NetImageElement>(
     // URL to download
    "https://github.com/fortheusers/chesto/raw/main/logo.png",
    // Fallback image while downloading
    []() -> Texture*  {
        // fallback element to display while downloading
        return make_shared<ImageElement>(RAMFS "res/icon.png");
    },
    // whether to start loading asap, or wait until .load() is called (eg. when the element is visible)
    true
);
```

### Drawing Text
The [TextElement](src/TextElement.hpp) class is used to display sentences or paragraphs of text, with or without wrappinig. Below instantiates gray text at (40, 20) relative to the current Element:

```C++
CST_Color gray = { 80, 80, 80, 0xff };
int fontSize = 12;
auto status = addNode<TextElement>(
    "All good here!",   // text string
    12,                 // text size
    &gray,              // text color
);
```

### Binding Buttons
The [Button](src/Button.hpp) class automatically subclasses Element and bundles together a TextElement, as well as some touch/gamepad input handling. Like other Elements, the `action` callback can be set to a member function of another class or function (see [here](https://stackoverflow.com/questions/14189440/c-class-member-callback-simple-examples) for more info), which will be invoked either when the InputEvent gamepad is triggered or the button is touched.

To create a button, give it the text, the button which corresponds to it, whether it's light or dark themed, and a font size. It can optionally also take a width as the last element, otherwise it will automatically fit the width to the inner text.

```C++
auto start = addNode<Button>("Begin!", START_BUTTON, true, 20);
start->setAction([]{
    std::cout << "Start button pressed!" << std::endl;
});
```

### DropDowns
The [DropDown](src/DropDown.hpp) class can be used to create a drop-down selection menu. It contains a list of string options, and an `onChange` callback that is invoked when the user selects a new option.

```C++
auto dropdown = addNode<DropDown>(A_BUTTON,
    std::vector<std::pair<std::string, std::string>>{
        // choices as (internal value, visual label) pairs
        {"option1", "First Option"},
        {"option2", "Second Option"},
        {"option3", "Third Option"}
    },
    [](std::string choice){
        // callback after option is selected
        std::cout << "User selected: " << choice << std::endl;
    },
    18,             // text size
    "option2",      // default choice (value and label)
    true            // dark mode
);
```

### Constraints
Elements can be constrained relative to their parent or other Elements using the `constrain` and `constrainToTarget` methods. This allows for dynamic positioning based on screen size or other elements.

```C++
// Center an element horizontally within its parent
element->constrain(ALIGN_CENTER_HORIZONTAL);
// Align an element to the top of its parent with a 10px offset
element2->constrain(ALIGN_TOP, 10);
// Align an element to the bottom of another element with a 5px offset
element3->constrainToTarget(otherElement, ALIGN_BOTTOM, 5);
```

For more on the different Constraint options, see [src/Constraint.hpp](src/Constraint.hpp). Constraints are applied during the internal `recalcPosition` method during/before `render` in the lifecycle.

### Animations
Elements can also have animations applied to them. For example, to slide in from the right, the following code can be used, as an example:

```C++
dest = 100; // target position to animate to

rows->animate(250, [this, container](float progress) {
    // onStep for the animation, called with progress from 0.0 to 1.0 over 250ms
    container->constraints.clear();
    container->constrain(ALIGN_RIGHT, dest - (container->width * (1 - progress))); // tween'd position
}, [this, container]() {
    // onEnd for the animation, called once at the end
    container->constraints.clear();
    container->constrain(ALIGN_RIGHT, dest); // final resting position
});
```

This leverages both the constraint system and the animation system, which are automatically called every frame. The code is in [src/Animation.cpp](src/Animation.cpp).

### Containers
There are a few layout-based containers, for drawing rows or columns of elements. These are similar to VStack or HStack in SwiftUI. See [src/Container.hpp](src/Container.hpp) for more info.

An example of a horizontal row, which uses `add` to put more children elements inside, with a 40px spacing between them:

```C++
auto rows = addNode<Container>(COL_LAYOUT, 40);
auto button1 = std::make_unique<Button>("Button 1", A_BUTTON, true, 20);
rows->add(std::move(button1));
auto button2 = std::make_unique<Button>("Button 2", B_BUTTON, true, 20);
rows->add(std::move(button2));
rows->constrain(ALIGN_CENTER_HORIZONTAL)->constrain(ALIGN_BOTTOM, 50);
```

### Scrollable Views
The [ListElement](src/ListElement.hpp) class should be subclassed and used to contain other groups that automatically need to be presented in a format so that they can trail off the page and be scrolled through either via touch events or gamepad buttons.

You can provide your own cursor logic here as well by overriding `process` in the LisitElement subclass, if you want the gamepad controls to do more than just scroll the page. It should play nicely with sub-elements that are marked as touchable.

```C++
auto list = addNode<ListElement>();
list->width = SCREEN_WIDTH;
list->height = SCREEN_HEIGHT;
list->child(std::move(rows)); // rows is a Container that has its own layout
```

### Screen subsystem
The [Screen](src/Screen.hpp) class can be subclassed to push and pop different full screens of Element's onto the display. This can be used to manage overlays and layers, and HB AppStore uses it to show different pages, pop ups, or modals. DropDown uses this to display its elements.

Screens are the same as Elements (They even subclass Element), but they initialize themselves to be fullscreen, and have another lifecycle method: `rebuildUI`. This method is called when the Screen is first pushed, and can be used to wipe and reconstruct the children Elements according to the Screen's state / field variables.

```C++
// to push a screen
auto myScreen = std::make_shared<MyScreen>(); // subclass screen
RootDisplay::pushScreen(myScreen);

// when it's time to remove it
RootDisplay::popScreen();
```

### i18n System
A basic internationalization system is included in [TextElement.cpp](src/TextElement.hpp). See HB AppStore for more examples on how it can be used. Once loaded, `i18n("my.key.name")` can be used to retrieve the localized string for the current language.

<!-- Syntax/component isn't finalized yet
### AlertDialog
The [AlertDialog](src/AlertDialog.hpp) class can be used to display a simple modal dialog with a message and an OK button. It automatically handles centering and dimming the background.

```C++
auto alert = std::make_shared<AlertDialog>(
    "This is an alert message!", // message text
    [](){                        // callback when OK is pressed
        std::cout << "Alert OK pressed!" << std::endl;
    },
    18,                          // text size
    true                         // dark mode
);
RootDisplay::pushScreen(alert);
```
-->

### Onscreen Keyboard
A cross-platform onscreen keyboard is in [EKeyboard.cpp](src/EKeyboard.hpp). This was originally the vgedit keyboard! It's not currently implemented as a Screen, although it is an overlay. This means that you can push it on top of any existing Element/Screen, but still interact with the views below it.

```C++
auto keyboard = addNode<EKeyboard>();
keyboard->typeAction = std::bind(&keyboardInputCallback, this);
keyboard->preventEnterAndTab = true;
keyboard->updateSize();
```

For a better example of how to use it, see [VGEdit](https://github.com/vgmoose/vgedit). The `keyboardInputCallback` is invoked with the single key that was selected. If the `storeOwnText` field is true, then the keyboard will maintain its own internal text string, which can be accessed with: `.textInput`.

### Displaying Progress
The [ProgressBar](src/ProgressBar.hpp) element takes a `percent` float between 0.0 and 1.0, as well as an overall width for how long the progress bar should be at 100%.

Can be created as follows. If `dimBg` is set, then the entire screen under this progress bar will be covered with a transparent gray sheet.

```C++
auto pbar = addNode<ProgressBar>();
pbar->width = 740;
pbar->color = 0xff0000ff;
pbar->dimBg = true;
```

## Networking Helpers
Chesto maintains a download queue via the [DownloadQueue](src/DownloadQueue.hpp) class, which can be used to download files from the internet in the background. It supports multiple simultaneous downloads, and will retry failed downloads up to a specified number of times.

**NOTE**: To use any networking features, the app must be built with these Makefile flags:
```
CFLAGS += -DNETWORK
LDFLAGS += -lcurl
```

And `resin/res/cacert.pem` must be included in the romfs folder, as it is used to verify SSL certificates, even on platforms with no certificate store.


## Makefile System
A bare minimum Chesto makefile looks like this:

```Makefile
APP_TITLE	:= App Name
APP_AUTHOR 	:= Your Name Here

SOURCES		+=	.
APP_VERSION	:=	1.0.0

include libs/chesto/Makefile
```

As much as possible is tried to be handled by the Makefiles in the `helpers` folder! When you run make, it will also prompt you to specify a platform, as seen below:

```
$ make
This is a Chesto app! For more information see: https://github.com/fortheusers/chesto
No targets were specified, try:
	make <target>
Where <target> is one of: pc, wiiu, switch, 3ds, wii
```

To see other ways these makefiles can be augmented, see the following other Chesto projects:
- [chestotesto](https://gitlab.com/4TU/chestotesto/-/blob/master/Makefile)
- [hb-appstore](https://github.com/fortheusers/hb-appstore/blob/main/Makefile)
- [vgedit](https://github.com/vgmoose/vgedit/blob/main/Makefile)
- [ShoWiFi](https://github.com/vgmoose/showifi/blob/main/Makefile)

If you want to follow the Makefile logic further, see the [main Makefile](https://github.com/fortheusers/chesto/blob/main/Makefile) (which is included in the above snippet, on the last line), and see how it calls out to the other Makefiles depending on what platform is specified.

### Building
Building a Chesto app will depend on what platform is being targeted. In general "just" SDL2 and libcurl are required, but each platform may have its own quirks.

A container is available at [fortheusers/sealeo](https://github.com/fortheusers/sealeo) which has all the dependencies pre-installed for building Chesto apps for all supported platforms. It can be ran via Docker like:

```
docker run -v $(pwd):/code -it ghcr.io/fortheusers/sealeo:evo /bin/bash
make pc   # or other platform
```

To see how to setup dependencies outside of Docker, check [dependency_helper.sh](https://github.com/fortheusers/sealeo/blob/main/dependency_helper.sh).

## License
This software is licensed under the GPLv3.

## Example
For an example of what an app that integrates Chesto looks like, see [ChestoTesto](https://gitlab.com/4TU/chestotesto) by [CompuCat](https://compucat.me).

### Dependencies
Chesto makes use of [resinfs](https://github.com/fortheusers/resinfs) to display images and other assets from memory rather than files. Any files in the top-level `resin` folder will be bundled using this dependency.

To reference any resinfs paths in the app, prefix the string with `RAMFS`. This will automatically be set to the correct value based on the platform, to either `resin:/` or `./resin` (local folder, no bundling).

This is included as a submodule, so Chesto should be cloned with `--recursive` to use this functionality.
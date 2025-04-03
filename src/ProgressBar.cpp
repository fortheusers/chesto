#include "ProgressBar.hpp"
#include "RootDisplay.hpp"

ProgressBar::ProgressBar()
{
	// total width of full progress bar
	this->width = 450;
	this->height = 5; //HACKY: progress bars are 9 px tall and erroneously extend 4 px above their y-position (other elements use y-position as top, not center)
	this->color = 0x56c1dfff;
}

void ProgressBar::render(Element* parent)
{
	// if we're hidden, don't render
	if (hidden) return;

	if (this->percent < 0)
		return;

	auto renderer = getRenderer();

	if (dimBg)
	{
		// draw a big dim layer around the entire window before drawing this progress bar
		CST_Rect dim = { 0, 0, RootDisplay::screenWidth, RootDisplay::screenHeight };

		CST_SetDrawBlend(renderer, true);
		CST_SetDrawColorRGBA(renderer, 0x00, 0x00, 0x00, 0xbb);
		CST_FillRect(renderer, &dim);
	}

	this->recalcPosition(parent);

	int blue = this->color;
	//	int gray = 0x989898ff;

	// draw full grayed out bar first
	CST_Rect gray_rect;
	gray_rect.x = this->xAbs;
	gray_rect.y = this->yAbs - 4;
	gray_rect.w = this->width;
	gray_rect.h = 9;

	CST_SetDrawColorRGBA(renderer, 0x98, 0x98, 0x98, 0xff); //gray2
	CST_FillRect(renderer, &gray_rect);

	// draw ending "circle"
	CST_filledCircleRGBA(renderer, this->xAbs + this->width, this->yAbs, 4, 0x98, 0x98, 0x98, 0xff);

	// draw left "circle" (rounded part of bar)
	CST_filledCircleRGBA(renderer, this->xAbs, this->yAbs, 4, 0x56, 0xc1, 0xdf, 0xff);

	// draw blue progress bar so far
	CST_Rect blue_rect;
	blue_rect.x = this->xAbs;
	blue_rect.y = this->yAbs - 4;
	blue_rect.w = this->width * this->percent;
	blue_rect.h = 9;

	if (this->percent > 1) {
		// prevent going too far past the end
		blue_rect.w = this->width;
	}

	CST_SetDrawColorRGBA(renderer, 0x56, 0xc1, 0xdf, 0xff); // blue2
	CST_FillRect(renderer, &blue_rect);

	// draw right "circle" (rounded part of bar, and ending)
	CST_filledCircleRGBA(renderer, this->xAbs + width * this->percent, this->yAbs, 4, 0x56, 0xc1, 0xdf, 0xff);
}

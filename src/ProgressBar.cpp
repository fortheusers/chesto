#include "ProgressBar.hpp"

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

	if (dimBg)
	{
		// draw a big dim layer around the entire window before drawing this progress bar
		CST_Rect dim = { 0, 0, 1280, 720 };

		CST_SetDrawBlend(parent->renderer, true);
		CST_SetDrawColorRGBA(parent->renderer, 0x00, 0x00, 0x00, 0xbb);
		CST_FillRect(parent->renderer, &dim);
	}

	CST_Rect location;
	int x = this->x + parent->x;
	int y = this->y + parent->y;

	int blue = this->color;
	//	int gray = 0x989898ff;

	// draw full grayed out bar first
	CST_Rect gray_rect;
	gray_rect.x = x;
	gray_rect.y = y - 4;
	gray_rect.w = width;
	gray_rect.h = 9;

	CST_SetDrawColorRGBA(parent->renderer, 0x98, 0x98, 0x98, 0xff); //gray2
	CST_FillRect(parent->renderer, &gray_rect);

	// draw ending "circle"
	CST_filledCircleRGBA(parent->renderer, x + this->width, y, 4, 0x98, 0x98, 0x98, 0xff);

	// draw left "circle" (rounded part of bar)
	CST_filledCircleRGBA(parent->renderer, x, y, 4, 0x56, 0xc1, 0xdf, 0xff);

	// draw blue progress bar so far
	CST_Rect blue_rect;
	blue_rect.x = x;
	blue_rect.y = y - 4;
	blue_rect.w = width * this->percent;
	blue_rect.h = 9;

	CST_SetDrawColorRGBA(parent->renderer, 0x56, 0xc1, 0xdf, 0xff); // blue2
	CST_FillRect(parent->renderer, &blue_rect);

	// draw right "circle" (rounded part of bar, and ending)
	filledCircleRGBA(parent->renderer, x + width * this->percent, y, 4, 0x56, 0xc1, 0xdf, 0xff);
}

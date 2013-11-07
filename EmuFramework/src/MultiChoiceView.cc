/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <util/number.h>
#include <EmuSystem.hh>
#include <MultiChoiceView.hh>
#include <algorithm>

void BaseMultiChoiceView::draw(Gfx::FrameTimeBase frameTime)
{
	using namespace Gfx;
	/*resetTransforms();
	setBlendMode(0);
	setColor(.2, .2, .2, 1.);
	GeomRect::draw(viewFrame);*/
	BaseMenuView::draw(frameTime);
}

void BaseMultiChoiceView::drawElement(const GuiTable1D *table, uint i, Coordinate xPos, Coordinate yPos, Coordinate xSize, Coordinate ySize, _2DOrigin align) const
{
	using namespace Gfx;
	if((int)i == activeItem)
		setColor(0., .8, 1.);
	else
		setColor(COLOR_WHITE);
	item[i]->draw(xPos, yPos, xSize, ySize, align);
}

void MultiChoiceView::freeItems()
{
	if(choiceEntryItem)
	{
		logMsg("freeing menu choices");
		mem_free(choiceEntryItem);
		choiceEntry = nullptr;
		choiceEntryItem = nullptr;
	}
}

void MultiChoiceView::allocItems(int items)
{
	freeItems();
	uint menuItemPtrArrSize  = IG::alignRoundedUp(sizeof(MenuItem*) * items, 16);
	char *storage = (char*)mem_alloc(menuItemPtrArrSize + sizeof(TextMenuItem) * items);
	assert(storage);
	choiceEntryItem = (MenuItem**)storage;
	choiceEntry = (TextMenuItem*)(storage + menuItemPtrArrSize);
	iterateTimes(items, i)
	{
		new(&choiceEntry[i]) TextMenuItem();
	}
}

void MultiChoiceView::init(const char **choice, uint choices, bool highlightCurrent, _2DOrigin align)
{
	//assert(choices <= sizeofArray(choiceEntry));
	allocItems(choices);
	iterateTimes(choices, i)
	{
		choiceEntry[i].init(choice[i]);
		choiceEntryItem[i] = &choiceEntry[i];
	}
	BaseMenuView::init(choiceEntryItem, choices, highlightCurrent, align);
}

void MultiChoiceView::init(MultiChoiceMenuItem &src, bool highlightCurrent, _2DOrigin align)
{
	//assert((uint)src.choices <= sizeofArray(choiceEntry));
	allocItems(src.choices);
	iterateTimes(src.choices, i)
	{
		choiceEntry[i].init(src.choiceStr[i], src.t2.face);
		choiceEntryItem[i] = &choiceEntry[i];
	}
	BaseMenuView::init(choiceEntryItem, src.choices, 0, align);
	activeItem = src.choice;
	if(highlightCurrent)
	{
		tbl.selected = src.choice;
	}
	onSelectD =
		[&](int i, const Input::Event &e)
		{
			return src.set(i, e, *this);
		};
}

void MultiChoiceView::deinit()
{
	BaseMenuView::deinit();
	freeItems();
}

void MultiChoiceView::onSelectElement(const GuiTable1D *table, const Input::Event &e, uint i)
{
	logMsg("set choice %d", i);
	if(onSelectD((int)i, e))
	{
		viewStack.popAndShow();
	}
}

void MultiChoiceSelectMenuItem::init(const char *str, const char **choiceStr, int val, int max, int baseVal, bool active, const char *initialDisplayStr, ResourceFace *face)
{
	onSelect() = [this](DualTextMenuItem &t, const Input::Event &e) { handleChoices(t, e); };
	MultiChoiceMenuItem::init(str, choiceStr, val, max, baseVal, active, initialDisplayStr, face);
}

void MultiChoiceSelectMenuItem::init(const char **choiceStr, int val, int max, int baseVal, bool active, const char *initialDisplayStr, ResourceFace *face)
{
	onSelect() = [this](DualTextMenuItem &t, const Input::Event &e) { handleChoices(t, e); };
	MultiChoiceMenuItem::init(choiceStr, val, max, baseVal, active, initialDisplayStr, face);
}

void MultiChoiceSelectMenuItem::handleChoices(DualTextMenuItem &, const Input::Event &e)
{
	auto &multiChoiceView = *menuAllocator.allocNew<MultiChoiceView>(t.str, Base::mainWindow());
	multiChoiceView.init(*this, !e.isPointer());
	viewStack.pushAndShow(&multiChoiceView, &menuAllocator);
}


#include "demo_silverlight.h"
#include "base/path_service.h"
#include "base/strings/sys_string_conversions.h"
#include "ui/views/controls/activex/silverlight_view.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/layout/grid_layout.h"

#include "demo_main.h"

DemoSilverlight::DemoSilverlight( ) :  
silverlight_view1_(NULL), silverlight_view2_(NULL), button_(NULL) { view_created_ = false; }

DemoSilverlight::~DemoSilverlight() {}

std::wstring DemoSilverlight::GetTitle()
{
    return std::wstring(L"Silverlight");
}

void DemoSilverlight::InitView(views::View* container)
{
    views::GridLayout* layout = new views::GridLayout(container);
    container->SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);

    views::ColumnSet* column_set1 = layout->AddColumnSet(1);
    column_set1->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);

    layout->StartRow(1.0f, 0);
    silverlight_view1_ = new views::SilverlightView();
    layout->AddView(silverlight_view1_);
    silverlight_view2_ = new views::SilverlightView();
    layout->AddView(silverlight_view2_);

    layout->StartRow(0.0f, 1);
    button_ = new views::NativeTextButton(this, L"¼ÓÔØSilverlightÎÄ¼þ");
    layout->AddView(button_);
}

void DemoSilverlight::ButtonPressed(views::Button* sender,
                                    const ui::Event& event)
{
    if(button_ == sender)
    {
		base::FilePath path;
		PathService::Get(base::DIR_EXE, &path);
		base::FilePath path_flash = path.Append(L"res/SilverlightDemo.xap");
		silverlight_view1_->Play(path_flash.value());
        silverlight_view2_->Play(path_flash.value());
    }
}
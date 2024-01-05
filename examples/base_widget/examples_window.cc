// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples_window.h"

#include <string>

#include "base/memory/scoped_vector.h"
#include "base/utf_string_conversions.h"
#include "ui/base/models/combobox_model.h"
#include "ui/base/ui_base_paths.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h" 
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "tree_table_view_example.h"
// #include "table_example.h"

namespace views {
namespace examples {



class ExamplesWindowContents : public WidgetDelegateView, public TabbedPaneListener {
 public:
  ExamplesWindowContents(Operation operation)
      : 
        status_label_(new Label),
        operation_(operation) {
    instance_ = this;
	treeviewexample_ = new TreeTableViewExample;
	// table_example_ = new NativeTableExample;
  }
  virtual ~ExamplesWindowContents() {
	  delete treeviewexample_;
	  treeviewexample_ = NULL;
	  // delete table_example_;
	  // table_example_ = NULL;
  }

  // Prints a message in the status area, at the bottom of the window.
  void SetStatus(const std::string& status) {
    status_label_->SetText(UTF8ToUTF16(status));
  }
	  virtual void TabSelectedAt(int index) 
	  {
	  }
  static ExamplesWindowContents* instance() { return instance_; }

 private:
  // Overridden from WidgetDelegateView:
  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE {
    return ASCIIToUTF16("Views Examples");
  }
  virtual View* GetContentsView() OVERRIDE { return this; }
  virtual void WindowClosing() OVERRIDE {
    instance_ = NULL;
    if (operation_ == QUIT_ON_CLOSE)
      base::MessageLoopForUI::current()->Quit();
  }

  // Overridden from View:
  virtual void ViewHierarchyChanged(bool is_add,
                                    View* parent,
                                    View* child) OVERRIDE {
    if (is_add && child == this)
      InitExamplesWindow();
  }


  // Creates the layout within the examples window.
  void InitExamplesWindow() {
    set_background(Background::CreateStandardPanelBackground());
	FillLayout * layout = new FillLayout;
	SetLayoutManager(layout);
	
	views::TabbedPane* tabbed_pane = new views::TabbedPane();
	tabbed_pane->set_listener(this);

	AddChildView(tabbed_pane);

	// tabbed_pane->AddTab( L"table", table_example_->example_view() );
	tabbed_pane->AddTab( L"tree", treeviewexample_->example_view() );

  }



  static ExamplesWindowContents* instance_;
  Label* status_label_;
  const Operation operation_;
  TreeTableViewExample *treeviewexample_; 
  // NativeTableExample * table_example_;
  DISALLOW_COPY_AND_ASSIGN(ExamplesWindowContents);
};

// static
ExamplesWindowContents* ExamplesWindowContents::instance_ = NULL;

void ShowExamplesWindow(Operation operation) {
  if (ExamplesWindowContents::instance()) {
    ExamplesWindowContents::instance()->GetWidget()->Activate();
  } else {
    Widget::CreateWindowWithBounds(new ExamplesWindowContents(operation),
                                   gfx::Rect(200, 200, 850, 300))->Show();
  }
}

void LogStatus(const std::string& string) {
  ExamplesWindowContents::instance()->SetStatus(string);
}

}  // namespace examples
}  // namespace views

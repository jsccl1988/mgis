// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef UI_VIEWS_MARKUP_FACTORY_MARKUP_COMBOBOX_H_
#define UI_VIEWS_MARKUP_FACTORY_MARKUP_COMBOBOX_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/string16.h"
#include "ui/base/models/combobox_model.h"
#include "ui/views/view.h"
#include "ui/views/views_export.h"

namespace views {

class Combobox;

// Combobox plus the model it does not own. The model member outlives the
// inner Combobox because it is declared first and destroyed last.
class VIEWS_EXPORT MarkupCombobox : public View {
 public:
  MarkupCombobox();
  virtual ~MarkupCombobox();

  void AddItem(const string16& text);
  int item_count() const;

  // View:
  virtual void Layout() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual std::string GetClassName() const OVERRIDE;

 private:
  class Model : public ui::ComboboxModel {
   public:
    virtual int GetItemCount() const OVERRIDE;
    virtual string16 GetItemAt(int index) OVERRIDE;
    void Add(const string16& text);

   private:
    std::vector<string16> items_;
  };

  Model model_;
  Combobox* combo_;

  DISALLOW_COPY_AND_ASSIGN(MarkupCombobox);
};

}  // namespace views

#endif  // UI_VIEWS_MARKUP_FACTORY_MARKUP_COMBOBOX_H_

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tree_table_model.h"
#include "ui/base/l10n/l10n_util.h"
#include "base/logging.h"

namespace ui {



int TreeTableModel::GetIconIndex(TreeTableModelNode* node) {
	return -1;
}


// TableColumn -----------------------------------------------------------------

TreeTableColumn::TreeTableColumn()
	: id(0),
	title(),
	alignment(LEFT),
	width(-1),
	percent(),
	min_visible_width(0),
	sortable(false) {
}

TreeTableColumn::TreeTableColumn(int id, Alignment alignment, int width, float percent)
	: id(id),
	title(l10n_util::GetStringUTF16(id)),
	alignment(alignment),
	width(width),
	percent(percent),
	min_visible_width(0),
	sortable(false) {
}


}  // namespace ui

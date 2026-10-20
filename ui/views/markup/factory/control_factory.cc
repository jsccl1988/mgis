// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/factory/control_factory.h"

#include <cctype>
#include <utility>

#include "ui/views/view.h"

namespace views {
namespace {

std::string ToLower(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    out.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return out;
}

}  // namespace

MarkupAttrs::MarkupAttrs() {}
MarkupAttrs::MarkupAttrs(const MarkupAttrs&) = default;
MarkupAttrs::MarkupAttrs(MarkupAttrs&&) noexcept = default;
MarkupAttrs& MarkupAttrs::operator=(const MarkupAttrs&) = default;
MarkupAttrs& MarkupAttrs::operator=(MarkupAttrs&&) noexcept = default;
MarkupAttrs::~MarkupAttrs() {}

std::string MarkupAttrs::Get(std::string_view key,
                             std::string_view fallback) const {
  const auto it = values.find(std::string(key));
  if (it == values.end())
    return std::string(fallback);
  return it->second;
}

ControlFactory::ControlFactory() {}

void ControlFactory::RegisterTag(std::string_view tag, Creator creator) {
  creators_[ToLower(tag)] = std::move(creator);
}

bool ControlFactory::HasTag(std::string_view tag) const {
  return creators_.find(ToLower(tag)) != creators_.end();
}

std::unique_ptr<View> ControlFactory::Create(std::string_view tag,
                                             const MarkupAttrs& attrs) const {
  const auto it = creators_.find(ToLower(tag));
  if (it == creators_.end())
    return std::unique_ptr<View>();
  return it->second(tag, attrs);
}

std::vector<std::string> ControlFactory::RegisteredTags() const {
  std::vector<std::string> out;
  out.reserve(creators_.size());
  for (const auto& entry : creators_)
    out.push_back(entry.first);
  return out;
}

}  // namespace views

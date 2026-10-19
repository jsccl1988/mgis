// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_GIS_DOCUMENT_H_
#define CONTENT_PUBLIC_GIS_DOCUMENT_H_

namespace content {

// Session document handle. P0 only fixes the type: tools hand drafts to a
// later EditSession, and this class does not mutate features itself.
class GisDocument {
 public:
  virtual ~GisDocument() = 0;

 protected:
  GisDocument();

 private:
  GisDocument(const GisDocument&) = delete;
  GisDocument& operator=(const GisDocument&) = delete;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_GIS_DOCUMENT_H_

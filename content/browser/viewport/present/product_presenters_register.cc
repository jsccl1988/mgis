// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/browser/product_presenters.h"

#include "content/browser/viewport/map/scene_factory.h"
#include "content/browser/viewport/present/scene_impl.h"

namespace content {

void EnsureProductPresentersRegistered() {
  SetSceneFactory(&CreateProductScene);
}

namespace {

struct ProductPresentersRegistrar {
  ProductPresentersRegistrar() { EnsureProductPresentersRegistered(); }
};

ProductPresentersRegistrar g_product_presenters_registrar;

}  // namespace
}  // namespace content

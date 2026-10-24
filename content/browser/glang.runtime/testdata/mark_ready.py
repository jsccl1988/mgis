# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""Minimal .py suite: mark via glang.plugin then return from main()."""

import glang.plugin as g


def main():
    g.mark("py.ready")

#!/usr/bin/env python3
# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.
"""Generate compact EPSG:4326 world demo layers: Polygon / LineString / Point."""

from __future__ import annotations

import json
from pathlib import Path

OUT = Path(__file__).resolve().parent


def ring(*xy: tuple[float, float]) -> list[list[float]]:
    pts = [[float(x), float(y)] for x, y in xy]
    if pts[0] != pts[-1]:
        pts.append(pts[0])
    return pts


def feature(geom: dict, props: dict) -> dict:
    return {"type": "Feature", "properties": props, "geometry": geom}


def polygon(*xy: tuple[float, float]) -> dict:
    return {"type": "Polygon", "coordinates": [ring(*xy)]}


def line(*xy: tuple[float, float]) -> dict:
    return {
        "type": "LineString",
        "coordinates": [[float(x), float(y)] for x, y in xy],
    }


def point(x: float, y: float) -> dict:
    return {"type": "Point", "coordinates": [float(x), float(y)]}


def collection(features: list[dict]) -> dict:
    return {"type": "FeatureCollection", "features": features}


def countries() -> dict:
    # Low-res continent / island outlines (Natural Earth–style, not NE dump).
    feats = [
        feature(
            polygon(
                (-168, 65),
                (-141, 70),
                (-105, 73),
                (-84, 73),
                (-56, 52),
                (-67, 45),
                (-74, 40),
                (-80, 25),
                (-97, 26),
                (-106, 22),
                (-110, 24),
                (-117, 33),
                (-125, 49),
                (-130, 55),
                (-153, 58),
                (-166, 64),
            ),
            {"name": "North America", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-81, 8),
                (-77, 8),
                (-70, 12),
                (-60, 8),
                (-50, 0),
                (-35, -7),
                (-39, -23),
                (-48, -25),
                (-53, -34),
                (-68, -55),
                (-75, -52),
                (-71, -18),
                (-81, -5),
            ),
            {"name": "South America", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-10, 36),
                (-9, 43),
                (-5, 48),
                (-6, 58),
                (2, 53),
                (8, 57),
                (12, 66),
                (25, 71),
                (32, 70),
                (30, 60),
                (28, 45),
                (19, 40),
                (12, 38),
                (-5, 36),
            ),
            {"name": "Europe", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-17, 15),
                (-16, 22),
                (-9, 32),
                (10, 37),
                (25, 32),
                (32, 31),
                (43, 12),
                (51, 12),
                (43, -1),
                (41, -16),
                (32, -29),
                (20, -35),
                (18, -34),
                (12, -17),
                (13, -6),
                (8, 4),
                (-5, 5),
                (-12, 7),
            ),
            {"name": "Africa", "kind": "polygon"},
        ),
        feature(
            polygon(
                (26, 41),
                (36, 36),
                (36, 30),
                (44, 13),
                (51, 25),
                (60, 25),
                (77, 8),
                (98, 8),
                (104, 1),
                (122, 7),
                (122, 18),
                (125, 38),
                (128, 35),
                (142, 46),
                (163, 60),
                (180, 66),
                (180, 71),
                (140, 73),
                (80, 75),
                (45, 68),
                (40, 60),
                (30, 60),
                (28, 45),
            ),
            {"name": "Asia", "kind": "polygon"},
        ),
        feature(
            polygon(
                (113, -22),
                (114, -34),
                (129, -32),
                (137, -35),
                (141, -38),
                (150, -38),
                (153, -28),
                (153, -25),
                (145, -15),
                (142, -11),
                (132, -12),
                (122, -17),
            ),
            {"name": "Australia", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-180, -63),
                (-120, -65),
                (-60, -63),
                (0, -67),
                (60, -67),
                (120, -66),
                (180, -70),
                (180, -82),
                (-180, -82),
            ),
            {"name": "Antarctica", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-73, 76),
                (-60, 82),
                (-22, 81),
                (-22, 70),
                (-44, 60),
                (-53, 60),
            ),
            {"name": "Greenland", "kind": "polygon"},
        ),
        feature(
            polygon(
                (44, -12),
                (50, -12),
                (50, -26),
                (44, -25),
            ),
            {"name": "Madagascar", "kind": "polygon"},
        ),
        feature(
            polygon(
                (129, 31),
                (131, 31),
                (142, 41),
                (145, 43),
                (141, 45),
                (140, 41),
                (131, 34),
            ),
            {"name": "Japan", "kind": "polygon"},
        ),
        feature(
            polygon(
                (-8, 50),
                (-6, 50),
                (-1, 51),
                (1, 53),
                (-2, 58),
                (-5, 58),
                (-5, 55),
            ),
            {"name": "British Isles", "kind": "polygon"},
        ),
        feature(
            polygon(
                (166, -35),
                (173, -35),
                (178, -38),
                (176, -47),
                (167, -46),
                (172, -41),
            ),
            {"name": "New Zealand", "kind": "polygon"},
        ),
    ]
    return collection(feats)


def graticule() -> dict:
    feats = []
    for lon in range(-180, 181, 30):
        feats.append(
            feature(
                line((lon, -80), (lon, 80)),
                {"name": f"meridian {lon}", "kind": "line"},
            )
        )
    for lat in range(-60, 61, 30):
        feats.append(
            feature(
                line((-180, lat), (180, lat)),
                {"name": f"parallel {lat}", "kind": "line"},
            )
        )
    # Equator + prime meridian called out as the coastline/graticule pair.
    feats.append(
        feature(line((-180, 0), (180, 0)), {"name": "equator", "kind": "line"})
    )
    feats.append(
        feature(
            line((0, -80), (0, 80)), {"name": "prime meridian", "kind": "line"}
        )
    )
    return collection(feats)


def capitals() -> dict:
    cities = [
        ("Beijing", 116.4, 39.9),
        ("Washington", -77.0, 38.9),
        ("London", -0.1, 51.5),
        ("Paris", 2.3, 48.9),
        ("Moscow", 37.6, 55.8),
        ("Tokyo", 139.7, 35.7),
        ("New Delhi", 77.2, 28.6),
        ("Cairo", 31.2, 30.0),
        ("Brasilia", -47.9, -15.8),
        ("Canberra", 149.1, -35.3),
        ("Pretoria", 28.2, -25.7),
        ("Ottawa", -75.7, 45.4),
        ("Mexico City", -99.1, 19.4),
        ("Buenos Aires", -58.4, -34.6),
        ("Berlin", 13.4, 52.5),
        ("Rome", 12.5, 41.9),
        ("Jakarta", 106.8, -6.2),
        ("Nairobi", 36.8, -1.3),
        ("Reykjavik", -21.9, 64.1),
        ("Wellington", 174.8, -41.3),
    ]
    return collection(
        [
            feature(point(lon, lat), {"name": name, "kind": "point"})
            for name, lon, lat in cities
        ]
    )


def write(name: str, obj: dict) -> None:
    path = OUT / name
    path.write_text(json.dumps(obj, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {path} features={len(obj['features'])}")


def main() -> int:
    write("countries.geojson", countries())
    write("graticule.geojson", graticule())
    write("capitals.geojson", capitals())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

<!--
Copyright (c) 2026 The Mogu Authors.
All rights reserved.
-->

# 示例数据

这三个文件已经在磁盘上，示例程序只读它们。

| 文件 | 来源 | 时间 |
| --- | --- | --- |
| `ia_asos_20240112T18Z.csv` | 爱荷华 ASOS 地面风。列：`station,valid,lon,lat,sknt,drct`。`sknt` 为节，`drct` 为来向（0 为北）。 | 2024-01-12 18Z（文件内 `valid` 为 18:15–18:56） |
| `fallon_forge_lithology.csv` | OpenEI Geothermal Data Repository，Fallon FORGE 岩性录井（Blankenship 等，DOI 10.15121/1452754）。列为井号、单元代号、顶底深度（m/ft）和岩性名称。表内没有井口坐标。 | 岩性表 2018-03-13 更新 |
| `nwis_iv.json` | USGS NWIS 瞬时值，站点 05481650（Des Moines River near Saylorville, IA），参数 00060（流量，ft³/s）。一条测站记录，不是河漫滩多边形。 | 2026-10-21T12:00:00-05:00，数值 4060（临时值，可修订） |

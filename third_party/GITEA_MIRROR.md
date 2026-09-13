# mgis third_party Gitea mirror

Local Gitea base (from mogu `third_party/manifest.json` → `gitea_mirror`):
`http://localhost:3000/ccl/<lib>`

No packages were re-uploaded in this alignment. Existing remotes/tags are reused in `manifest.json`.

## Repos

| Library | Gitea status | Manifest | URL |
|---|---|---|---|
| glog | exists (tag `v0.6.0`, mogu pin) | **reused** mogu ref | http://localhost:3000/ccl/glog |
| googletest | exists (tag `v1.14.0`, mogu pin) | **reused** mogu ref | http://localhost:3000/ccl/googletest |
| cpp-httplib | exists (tag `v0.18.3`, mogu pin) | **reused** mogu ref | http://localhost:3000/ccl/cpp-httplib |
| googlebenchmark | exists (`main` import SHA) | **reused** Gitea HEAD | http://localhost:3000/ccl/googlebenchmark |
| sqlite3 | exists (import SHA) | **reused** | http://localhost:3000/ccl/sqlite3 |
| geos | exists (import SHA) | **reused** | http://localhost:3000/ccl/geos |
| PROJ | exists (import SHA) | **reused** | http://localhost:3000/ccl/PROJ |
| SQLiteCpp | exists (import SHA) | **reused** | http://localhost:3000/ccl/SQLiteCpp |
| libspatialindex | exists (import SHA) | **reused** | http://localhost:3000/ccl/libspatialindex |
| libspatialite | exists (import SHA) | **reused** | http://localhost:3000/ccl/libspatialite |
| SpatiaLiteCpp | exists (import SHA) | **reused** | http://localhost:3000/ccl/SpatiaLiteCpp |
| gdal | exists (import SHA) | **reused** | http://localhost:3000/ccl/gdal |
| CxImage | exists (import SHA) | **reused** (`install_skip`) | http://localhost:3000/ccl/CxImage |
| wtl8.0 | exists (import SHA) | **reused** (`install_skip`) | http://localhost:3000/ccl/wtl8.0 |
| benchmark (mogu name) | empty repo | unused | http://localhost:3000/ccl/benchmark |
| skia | skipped (empty local) | not listed | — |

## Notes

- User `GET /api/v1/user` needs `GITEA_TOKEN`; **public repo read** worked without auth.
- Empty Gitea placeholders (`benchmark`, `spdlog`) were not used and not force-pushed.
- Vendored trees remain under `third_party/<name>/` for local fetch skip.

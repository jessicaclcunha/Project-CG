# BRDF Roadmap

| BRDF | Status | Key Detail |
|------|--------|------------|
| Phong | ✅ Done | Baseline diffuse+specular |
| CookTorrance GGX | 🟡 Reworking | Must auto-calculate F0 from metallic + albedo |
| Oren-Nayar | ⬜ Week 3 | Diffuse roughness via σ parameter |
| Disney Principled | ⬜ Weeks 4-5 | Burley 2012 paper — multi-parameter (metallic, roughness, subsurface, sheen, clearcoat) |
| Ward Anisotropic | ⬜ Week 4 | αx, αy directional roughness |
| Ashikhmin-Shirley | ⬜ Week 5 | Anisotropic specular + Fresnel-weighted diffuse |
| Blinn-Phong | ⬜ Week 7 | Half-vector variant of Phong |

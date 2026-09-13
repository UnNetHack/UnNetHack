# Graphics iteration log

## 2026-09-13 — wooden club

Replaced the club's square metal head with a continuous tapered wooden body,
rounded striking end, grip bands, and subtle longitudinal grain grooves. Shared
held/ground integration retained. Build, finite bounds, and disposal checks passed;
in-game visual review remains pending.

## 2026-09-13 — axe silhouettes

Replaced the rectangular axe proxy with a curved, beveled blade, mounting socket,
rear poll, and bound grip. Battle-axes receive a second opposing blade. Shared
factory integration covers both held and ground weapons. Production build and
finite bounds/disposal checks passed; visual browser review remains pending.

## 2026-09-12 — war hammer

Added a dedicated transverse hammer head with a broad striking face, tapered
rear peen, central mounting band, grip bindings, and pommel. Both held and ground
weapons use this shared model. Build and finite-geometry/disposal checks passed;
browser visual review remains pending. Axes and clubs remain candidates for
replacing simple proxy geometry.

## 2026-09-12 — flanged mace

Replaced the mace's generic hammer-like cube with six beveled radial flanges,
a metal shaft, leather grip bindings, pommel, and head collars. Uses the shared
equipment factory so held and ground maces receive the same model. Other weapon
families retain their current geometry. Production build and headless construction,
finite bounds, and disposal checks passed. In-game visual review remains pending.

## 2026-09-12 — dedicated rats

Added procedural sewer, rabid, and giant rat models in `creatures.js`, replacing
their generic guardian fallback in Live mode. Rounded ears, dark eyes with small
highlights, whiskers, paired incisors, small feet, and a tapered segmented tail
give them a low, recognizable silhouette. Giant rats use a larger scale. The
existing actor animation interface supplies breathing, leg motion, and tail sway.
Exact name matching intentionally leaves wererats and other creatures unchanged.

Validation: production build passed; headless Three.js construction and finite
bounding-box checks passed for all three variants and two unaffected creatures.
Visual browser review remains outstanding.

Next candidates: a dedicated bat with wings, and differentiated food/tool models
after checking the current item renderer for remaining symbolic fallbacks.

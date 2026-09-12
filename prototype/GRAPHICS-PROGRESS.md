# Graphics iteration log

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

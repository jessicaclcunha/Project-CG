# Samples

This file documents key execution time samples collected throughout the project.

---

## Performance Evaluation

After implementing each method, compare results by:

1. **Image Quality**: Use RMSE vs. the `ALL_LIGHTS` reference.
2. **Performance**: Measure total render time.
3. **Efficiency**: 
$$
E = \frac{1}{\text{RMSE} \cdot T}
$$

---

## Reference time

```bash
Reference time: 83.909s; spp=16
```

Each sample includes an explanation along with relevant metrics such as execution time, distance (RMSE) and efficiency.

Times:

1. [Random Selection](#random-selection): 7.378s

---

## Random Selection

RMSE = 0.000109, MinMaxScaledRMSE = 0.000109

E = 0.000804202


# Light Sampling Algorithms for Direct Illumination (Brainstorm)

This document outlines various light sampling strategies to implement and compare in the context of direct lighting in ray tracing. These aim to improve upon the default `ALL_LIGHTS` and `UNIFORM_ONE` methods.

All tested algorithms should be added to [samples section](#samples).

---

## 1. Power-Weighted Light Sampling

Select a light source based on the product of its irradiance and area.

### PDF Calculation:
For each light \( L_i \):
$$
p_i = \frac{L_i \cdot A_i}{\sum_{j} L_j \cdot A_j}
$$

### Sampling Procedure:
1. Precompute the PDF for all lights.
2. Build a CDF from the PDF.
3. Generate a uniform random number \( r in [0, 1] \).
4. Select the first light \( i \) such that `CDF[i] >= r`.

## RESULTS

tempo = 143.34 seg

RMSE = 0.000137, MinMaxScaledRMSE = 0.000137

E = TODO

---

## 2. Importance Sampling by Estimated Contribution

Estimate the actual contribution of each light source to the current shading point.

### Contribution Estimate:

$$
\text{Estimate}_i = \frac{L_i \cdot A_i \cdot \max(0, \vec{N}_L \cdot \vec{L}) \cdot \max(0, \vec{N}_p \cdot \vec{L})}{\text{distance}^2}
$$

### Sampling Procedure:
Same as Power-Weighted Sampling, but using the contribution estimate instead of just power.

---

## 3. Reservoir Importance Sampling (for Large Light Sets)

Efficiently samples from a large set of lights without evaluating all of them.

### Key Concepts:
- Maintain a "reservoir" of candidate lights.
- Select lights with a probability proportional to their contribution.
- Works well when the number of lights is too large for brute-force importance evaluation.

---

## 4. Multiple Importance Sampling (MIS)

Combine two or more sampling techniques (e.g., uniform and power-based) for better results.

### Weighting:

Use balance heuristic:

$$
w_i = \frac{n_i \cdot \text{pdf}_i}{\sum_j n_j \cdot \text{pdf}_j}
$$


Where `n_i` is the number of samples from strategy `i`, and `pdf_i` is the PDF under strategy `i`.

### Pros:
- Reduces variance by leveraging strengths of different strategies.
- Especially useful in scenes with complex lighting.

---


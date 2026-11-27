# Q3C coordinate systems

Four maps take a sky position to a pixel number:

    (ra, dec)  ->  unit vector v = (x, y, z)  ->  (face f, X, Y)
               ->  integer cell (f, xi, yi)  ->  ipix

Sections 1-5 describe them in this order (section 2 sets up the cube);
section 6 shows how the vector form simplifies the query geometry.

All conventions below were checked against the current C code
(`q3c_ang2ipix_xy`, `q3c_ipix2ang`, `q3c_xy2facenum`) on 2e5 random points:
face numbers agree exactly, X and Y agree to 2e-14, ipix agrees exactly.

## Notation

| symbol | meaning |
|--------|---------|
| x, y, z | Cartesian coordinates of a point on the unit sphere (section 1) |
| x̂, ŷ, ẑ | unit vectors along the x, y, z axes: (1,0,0), (0,1,0), (0,0,1) |
| +x, -y, ... | shorthand for the vectors x̂, -ŷ, ... in the tables |
| v | unit vector (x, y, z) of the sky position being indexed |
| a·b, a×b | dot product, cross product |
| f | face number, 0..5 |
| n_f | outward unit normal of face f: the direction from the cube centre to the centre of face f. The face lies in the plane v·n_f = 1 |
| u_f | unit vector lying in face f, pointing along increasing X on that face |
| w_f | unit vector lying in face f, pointing along increasing Y on that face |
| X, Y | coordinates of a point within face f, each in [-1, 1]; (0,0) is the face centre, ±1 are the face edges |
| x, y (in code) | X/2, Y/2 ∈ [-0.5, 0.5]: what most of the C code stores (section 3) |
| nside | number of finest cells along each edge of a face (2^30) |
| xi, yi | integer column and row of the finest cell containing (X, Y), 0..nside-1 (section 4) |
| R_a(θ) | right-handed rotation by angle θ about axis a (counter-clockwise when looking down from +a) |

So each face has its own orthonormal frame (n_f, u_f, w_f): one axis
perpendicular to the face and two axes lying in it. A point on the face plane
is `n_f + X u_f + Y w_f`, which is how X and Y are defined.

## 1. (ra, dec) <-> xyz

    x = cos(dec) cos(ra),   y = cos(dec) sin(ra),   z = sin(dec)

So (1,0,0) is (ra,dec) = (0,0), (0,1,0) is (90,0), (0,0,1) is the north pole.
The xyz frame is right handed; ra increases counter-clockwise around +z.

## 2. The cube

The cube has its centre at the origin, edges parallel to the x, y, z axes,
and faces on the planes x = ±1, y = ±1, z = ±1 (the cube is
circumscribed around the unit sphere; only directions matter, so its size is
irrelevant). The table gives the frame (n_f, u_f, w_f) of every face, the
resulting formulas for X and Y in terms of v, and the sky position of the
face centre (the point v = n_f, i.e. X = Y = 0):

| face | n_f | u_f (+X) | w_f (+Y) | X = v·u/v·n | Y = v·w/v·n | centre (ra,dec) |
|------|-----|----------|----------|-------------|-------------|-----------------|
| 0    | +z  | +y       | -x       |  y/z        | -x/z        | (–, +90)        |
| 1    | +x  | +y       | +z       |  y/x        |  z/x        | (0, 0)          |
| 2    | +y  | -x       | +z       | -x/y        |  z/y        | (90, 0)         |
| 3    | -x  | -y       | +z       |  y/x        | -z/x        | (180, 0)        |
| 4    | -y  | +x       | +z       | -x/y        | -z/y        | (270, 0)        |
| 5    | -z  | +y       | +x       | -y/z        | -x/z        | (–, -90)        |

The whole table follows from three rules. "Rotating a frame by R" means
replacing each of its three vectors n, u, w by R n, R u, R w.

* Face 1 (centred on ra=0, dec=0) has the identity frame:
  (n_1, u_1, w_1) = (x̂, ŷ, ẑ). So on face 1, X = y/x grows towards
  ra=90° (east) and Y = z/x grows towards the north pole.
* Equatorial face f = 1..4 has the frame of face 1 rotated by R_z(90°·(f-1)).
  Equivalently: n_f points to (ra, dec) = (90°(f-1), 0); u_f = ẑ × n_f,
  the local direction of increasing ra (east); w_f = ẑ (north). Hence on
  every equatorial face X increases with ra and Y with dec.
* Polar faces have the frame of face 1 rotated about the y axis so that n
  moves from x̂ to the pole: face 0 is R_y(-90°) (x̂→ẑ, ẑ→-x̂), face 5 is
  R_y(+90°) (x̂→-ẑ, ẑ→x̂). ŷ is unchanged by these rotations, so faces 0, 1, 5
  all have u = ŷ: on both polar faces X points towards ra=90°. Y on face 0
  points towards ra=180° (w_0 = -x̂), Y on face 5 points towards ra=0°
  (w_5 = +x̂).

Consequences:

* Every frame is right handed with the normal pointing out of the cube:
  `u_f × w_f = n_f` (equivalently `w_f = n_f × u_f`). Picture an observer
  outside the cube looking at face f (looking along -n_f): u_f (+X) points to
  their right and w_f (+Y) points up. An observer at the centre (looking along
  +n_f, as one looks at the sky) sees +X to the left; for an equatorial face
  that is north up and east left, as on a standard sky image.
* Let `M_f` be the 3x3 matrix with rows n_f, u_f, w_f, and `t = M_f v =
  (v·n_f, v·u_f, v·w_f)` the components of v in the face frame (so
  X = t_2/t_1, Y = t_3/t_1). Every M_f is a signed permutation matrix (each row
  and column has a single nonzero entry, ±1), so t is just v with components
  permuted and sign-flipped, no arithmetic.
* Face 5 is the mirror image of face 0 under z -> -z with X flipped.
  Writing X_f(v), Y_f(v) for the X, Y of v on face f:
  `X_5(x,y,z) = -X_0(x,y,-z)`, `Y_5(x,y,z) = Y_0(x,y,-z)`. This is the
  "invert one axis for the south face" step in the ellipse code.
* On the polar faces ra is the polar angle in the (X, Y) plane. On face 0,
  (X, Y) = cot(dec) (sin ra, -cos ra): ra=0 is along -Y, 90° along +X, 180°
  along +Y, so ra turns from +X towards +Y (counter-clockwise in the usual
  X-right, Y-up drawing). On face 5, (X, Y) = |cot dec| (sin ra, cos ra): ra=0
  is along +Y, 90° along +X, so ra turns clockwise. Both are the same
  right-handed rule; the outward normals point in opposite directions.

### Unfolded cube

Each face drawn in its own (X right, Y up) frame:

            +---+
            | 0 |
        +---+---+---+---+
        | 4 | 1 | 2 | 3 |
        +---+---+---+---+
            | 5 |
            +---+

X_f, Y_f below denote the X, Y coordinates on face f of a point on the
shared edge. Edges that touch in this picture are seamless: X (horizontally) or Y
(vertically) carries over unchanged, e.g. the top edge of face 1 (Y=+1) is the
bottom edge of face 0 (Y=-1) with the same X. Face 3's right edge joins face 4's
left edge. The remaining polar edges are glued with a rotation:

| polar edge   | neighbour          | relation     |
|--------------|--------------------|--------------|
| face 0, X=+1 | face 2, top (Y=+1) | X_2 =  Y_0   |
| face 0, Y=+1 | face 3, top        | X_3 = -X_0   |
| face 0, X=-1 | face 4, top        | X_4 = -Y_0   |
| face 5, X=+1 | face 2, bottom     | X_2 = -Y_5   |
| face 5, Y=-1 | face 3, bottom     | X_3 = -X_5   |
| face 5, X=-1 | face 4, bottom     | X_4 =  Y_5   |

## 3. xyz <-> (face, X, Y)

**Face selection.** f is the face whose normal n_f is closest to v (largest
v·n_f), i.e. the face that the ray from the origin along v pierces. In
components: take the coordinate of v with the largest absolute value,
k = argmax(|x|, |y|, |z|), and its sign; then +z→0, +x→1, +y→2, -x→3, -y→4, -z→5.

Tie breaking in the current code (matters only exactly on face edges):
the equatorial face is chosen as `floor(((ra+45)/90) mod 4) + 1`, i.e. the
half-open range ra ∈ [90(f-1)-45, 90(f-1)+45), so an equatorial-equatorial tie
goes to the face with larger ra (x = y > 0 goes to face 2). The code then computes Y on that
equatorial face and switches to face 0 only if Y > 1 (to face 5 only if
Y < -1), strictly, so an equatorial-polar tie goes to the equatorial face.

**Forward (gnomonic projection onto the face).**

    X = (v·u_f) / (v·n_f),    Y = (v·w_f) / (v·n_f),    X, Y ∈ [-1, 1]

This is the central (gnomonic) projection onto the plane `v·n_f = 1`, so great
circles map to straight lines on a face. There is no area-equalising
correction (unlike the COBE quad-cube); the pixel grid is uniform in X, Y.

**Inverse.**

    v = (n_f + X u_f + Y w_f) / sqrt(1 + X² + Y²)

**Scaled face coordinates in the code.** Most of the code (`q3c_ang2ipix_xy`
output, `q3c_project_poly`, the query boxes, `Q3C_HALF`) stores
`x = X/2, y = Y/2 ∈ [-0.5, 0.5]`. `q3c_xy2facenum` takes the unscaled X, Y,
hence the `2 * points[...]` at its call sites.

## 4. (X, Y) <-> integer cell coordinates (xi, yi)

Each face is divided into an nside × nside grid of equal squares in (X, Y)
(nside = 2^30 in Q3C, set in `prepare.c`; it must be a power of 2). xi is the
column, yi the row; both run 0..nside-1 and increase in the same directions as
X and Y (along u_f and w_f). xi = 0 is the edge X = -1, xi = nside-1 the edge
X = +1. Everything below is identical for yi and Y.

**Forward** (`q3c_ang2ipix_xy`):

    xi = floor((X + 1)/2 · nside),    and xi = nside-1 if X = +1

The clamp puts the edge X = +1 into the last column. So column xi is the
half-open interval

    -1 + 2 xi/nside  <=  X  <  -1 + 2 (xi+1)/nside

(closed on both sides for the last column).

**Inverse** (`q3c_ipix2ang`) returns the cell centre:

    X = -1 + (2 xi + 1)/nside

**Cell sizes on the sky.** The grid is uniform in X, Y, not on the sky. The
solid angle of an element dX dY is dX dY / (1 + X² + Y²)^(3/2), so a cell at a
face corner covers 3^(3/2) ≈ 5.2 times less sky than one at the face centre.
The side of a finest cell is 2/nside ≈ 1.9e-9 rad ≈ 0.38 mas at the face
centre.

**Coarser levels (quadtree).** Because nside is a power of 2, the grid nests.
At a level with n0 cells per face side (n0 = 1 is the whole face, n0 = nside
the finest grid), the column index is

    Ix = floor((X + 1)/2 · n0) = xi >> log2(nside/n0)

(`q3c_cell_index` computes it from x = X/2), and coarse cell (Ix, Iy)
contains exactly the finest cells with xi ∈ [Ix·m, (Ix+1)·m),
yi ∈ [Iy·m, (Iy+1)·m), where m = nside/n0. `q3c_pixarea` uses the depth
d = log2(m) instead: d = 0 is a finest cell, d = 30 the whole face.

## 5. (face, xi, yi) <-> ipix

    ipix = f · nside² + interleave(xi, yi)

Each face therefore owns the contiguous range [f·nside², (f+1)·nside²).
`interleave` is the Z-order (Morton) code: counting bits from the least
significant (bit 0), bit b of xi goes to bit 2b of the result and bit b of yi
goes to bit 2b+1. With nside = 2^30 the in-face part uses bits 0-59 and the
face number bits 60-62. Within each 2×2 block the order is (xi, yi) =
(0,0), (1,0), (0,1), (1,1), i.e. X first.

This ordering is what makes the coarse levels of section 4 useful: the m × m
finest cells of a coarse cell (Ix, Iy) have consecutive ipix values,

    ipix(f, Ix·m, Iy·m)  <=  ipix  <  ipix(f, Ix·m, Iy·m) + m²

so any coarse cell is a single ipix range, which is what the queries emit
(`q3c_push_box_cells`, `q3c_output_stack`).

In the code `q3c_xiyi2ipix` does the forward interleaving through the lookup
tables `xbits`/`ybits` (16 bits of xi or yi at a time); `q3c_ipix2ang` and
`q3c_pixarea` undo it through `xbits1`/`ybits1`. All tables are built in
`init_q3c1`.

## 6. Vector form of the query geometry

The vector frames remove the equatorial/north/south branches from the query
code. Example: a cone search of radius r around the unit vector c (the
query centre) selects the points with `v·c >= cos r`. On face f substitute
`v = (n_f + X u_f + Y w_f) / sqrt(1 + X² + Y²)` and write
`c_n = c·n_f, c_u = c·u_f, c_w = c·w_f` (the components of c in the face
frame). For points with v·c > 0 the condition becomes

    (c_n + X c_u + Y c_w)² >= cos²r (1 + X² + Y²)

i.e. `axx X² + ayy Y² + axy XY + ax X + ay Y + a <= 0` with

    axx = cos²r - c_u²,   ayy = cos²r - c_w²,   axy = -2 c_u c_w,
    ax  = -2 c_n c_u,     ay  = -2 c_n c_w,     a   = cos²r - c_n²

This reproduces all three branches of `q3c_get_poly_coefs` term by term
(before its final ×4, ×2 rescaling to x = X/2, y = Y/2). Squaring drops the
sign of `c_n + X c_u + Y c_w`, so it also admits the antipodal cone; the
existing code has the same property.

## Where the conventions are hard-coded today

Each of these has its own equatorial / face 0 / face 5 branches:

* `q3cube.c`: `q3c_ang2ipix_xy`, `q3c_get_facenum`, `q3c_ipix2ang`,
  `q3c_xy2facenum`, `q3c_get_poly_coefs`, `q3c_fast_get_circle_xy_minmax`,
  `q3c_fast_get_ellipse_xy_minmax`,
  `q3c_fast_get_ellipse_xy_minmax_and_poly_coefs` and the
  `*_equatorial_*` / `*_polar_*` ellipse helpers.
* `q3c_poly.c`: `q3c_project_poly`.

Caution for a refactor: `q3c_ang2ipix` is what existing functional indexes were
built with. If a vector version gives a different floating-point result for a
point lying within rounding error of a cell or face edge, that point gets a
different ipix than the stored one and can be missed by queries. Either keep
`q3c_ang2ipix` bit-identical (including the tie breaking above), or use the
vector form only in the query geometry, where the cell boxes are conservative.

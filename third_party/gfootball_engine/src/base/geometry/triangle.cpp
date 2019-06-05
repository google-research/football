// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "../../base/geometry/triangle.hpp"

#include <cmath>

namespace blunted {

  // for triangle-triangle intersection; code by Tomas Moller, 1997.
  // See article "A Fast Triangle-Triangle Intersection Test",
  // Journal of Graphics Tools, 2(2), 1997
  /* sort so that a<=b */
  #define SORT(a,b)       \
    if(a>b)    \
    {          \
      float c = a; \
      a=b;     \
      b=c;     \
  }

  #define COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) { \
    if(D0D1>0.0f) \
    { \
            /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
            A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
    } \
    else if(D0D2>0.0f)\
    { \
            /* here we know that d0d1<=0.0 */ \
        A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
    } \
    else if(D1*D2>0.0f || D0!=0.0f) \
    { \
            /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
            A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
    } \
    else if(D1!=0.0f) \
    { \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
    } \
    else if(D2!=0.0f) \
    { \
            A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
    } \
    else \
    { \
            /* triangles are coplanar */ \
            return IsCoplanar(N1,triangle); \
    } \
  }

  /* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
  #define EDGE_EDGE_TEST(V0,U0,U1)                      \
    Bx=U0.coords[i0]-U1.coords[i0];                                   \
    By=U0.coords[i1]-U1.coords[i1];                                   \
    Cx=V0.coords[i0]-U0.coords[i0];                                   \
    Cy=V0.coords[i1]-U0.coords[i1];                                   \
    f=Ay*Bx-Ax*By;                                      \
    d=By*Cx-Bx*Cy;                                      \
    if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
    {                                                   \
      e=Ax*Cy-Ay*Cx;                                    \
      if(f>0)                                           \
      {                                                 \
        if(e>=0 && e<=f) return true;                      \
      }                                                 \
      else                                              \
      {                                                 \
        if(e<=0 && e>=f) return true;                      \
      }                                                 \
    }

  #define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
  {                                              \
    real Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
    Ax=V1.coords[i0]-V0.coords[i0];                            \
    Ay=V1.coords[i1]-V0.coords[i1];                            \
    /* test edge U0,U1 against V0,V1 */          \
    EDGE_EDGE_TEST(V0,U0,U1);                    \
    /* test edge U1,U2 against V0,V1 */          \
    EDGE_EDGE_TEST(V0,U1,U2);                    \
    /* test edge U2,U1 against V0,V1 */          \
    EDGE_EDGE_TEST(V0,U2,U0);                    \
  }

  #define POINT_IN_TRI(V0,U0,U1,U2)           \
  {                                           \
    real a,b,c,d0,d1,d2;                     \
    /* is T1 completly inside T2? */          \
    /* check if V0 is inside tri(U0,U1,U2) */ \
    a=U1.coords[i1]-U0.coords[i1];                          \
    b=-(U1.coords[i0]-U0.coords[i0]);                       \
    c=-a*U0.coords[i0]-b*U0.coords[i1];                     \
    d0=a*V0.coords[i0]+b*V0.coords[i1]+c;                   \
                                              \
    a=U2.coords[i1]-U1.coords[i1];                          \
    b=-(U2.coords[i0]-U1.coords[i0]);                       \
    c=-a*U1.coords[i0]-b*U1.coords[i1];                     \
    d1=a*V0.coords[i0]+b*V0.coords[i1]+c;                   \
                                              \
    a=U0.coords[i1]-U2.coords[i1];                          \
    b=-(U0.coords[i0]-U2.coords[i0]);                       \
    c=-a*U2.coords[i0]-b*U2.coords[i1];                     \
    d2=a*V0.coords[i0]+b*V0.coords[i1]+c;                   \
    if(d0*d1>0.0)                             \
    {                                         \
      if(d0*d2>0.0) return true;                 \
    }                                         \
  }

  Triangle::Triangle() {
  }

  Triangle::Triangle(const Triangle &triangle) {

    memcpy(vertices, triangle.vertices, sizeof(triangle.vertices));
    memcpy(textureVertices, triangle.textureVertices, sizeof(triangle.textureVertices));
    memcpy(normals, triangle.normals, sizeof(triangle.normals));
    memcpy(tangents, triangle.tangents, sizeof(triangle.tangents));
    memcpy(biTangents, triangle.biTangents, sizeof(triangle.biTangents));
  }

  Triangle::Triangle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
    SetVertex(0, v1);
    SetVertex(1, v2);
    SetVertex(2, v3);
    for (int v = 0; v < 3; v++) {
      for (int tu = 0; tu < 8; tu++) {
        textureVertices[v][tu].Set(0, 0, 0);
      }
      normals[v].Set(0, 0, 0);
      tangents[v].Set(0, 0, 0);
      biTangents[v].Set(0, 0, 0);
    }
  }

  Triangle::~Triangle() {
  }


  bool Triangle::operator == (const Triangle &triangle) const {
    if (vertices[0] == triangle.GetVertex(0) &&
        vertices[1] == triangle.GetVertex(1) &&
        vertices[2] == triangle.GetVertex(2)) {
      return true;
    }
    return false;
  }


  inline void Triangle::SetVertex(unsigned char pos, const real x, const real y, const real z) {
//    assert(pos < 3);
    vertices[pos].coords[0] = x;
    vertices[pos].coords[1] = y;
    vertices[pos].coords[2] = z;
  }

  inline void Triangle::SetVertex(unsigned char pos, const Vector &vec) {
//    assert(pos < 3);
    vertices[pos].coords[0] = vec.coords[0];
    vertices[pos].coords[1] = vec.coords[1];
    vertices[pos].coords[2] = vec.coords[2];
  }

  inline void Triangle::Translate(const real x, const real y, const real z) {
    for (int i = 0; i < 3; i++) {
      vertices[i].coords[0] += x;
      vertices[i].coords[1] += y;
      vertices[i].coords[2] += z;
    }
  }

  inline void Triangle::SetTextureVertex(unsigned char pos, const real x, const real y, const real z) {
    SetTextureVertex(0, pos, x, y, z);
  }

  inline void Triangle::SetTextureVertex(unsigned char texture_unit, unsigned char pos, const Vector3 &xyz) {
    SetTextureVertex(texture_unit, pos, xyz.coords[0], xyz.coords[1], xyz.coords[2]);
  }

  inline void Triangle::SetTextureVertex(unsigned char texture_unit, unsigned char pos, const real x, const real y, const real z) {
//    assert(pos < 3);
//    assert(texture_unit < 8);
    textureVertices[pos][texture_unit].coords[0] = x;
    textureVertices[pos][texture_unit].coords[1] = y;
    textureVertices[pos][texture_unit].coords[2] = z;
  }

  inline void Triangle::SetNormal(unsigned char pos, const real x, const real y, const real z) {
//    assert(pos < 3);
    normals[pos].coords[0] = x;
    normals[pos].coords[1] = y;
    normals[pos].coords[2] = z;
  }

  inline void Triangle::SetNormal(unsigned char pos, const Vector &vec) {
//    assert(pos < 3);
    normals[pos].coords[0] = vec.coords[0];
    normals[pos].coords[1] = vec.coords[1];
    normals[pos].coords[2] = vec.coords[2];
  }

  inline void Triangle::SetNormals(const real x, const real y, const real z) {
    for (int i = 0; i < 3; i++) {
      normals[i].coords[0] = x;
      normals[i].coords[1] = y;
      normals[i].coords[2] = z;
    }
  }

  inline void Triangle::SetNormals(const Vector &vec) {
    for (int i = 0; i < 3; i++) {
      normals[i].coords[0] = vec.coords[0];
      normals[i].coords[1] = vec.coords[1];
      normals[i].coords[2] = vec.coords[2];
    }
  }

  inline const Vector3 &Triangle::GetVertex(unsigned char pos) const {
    //assert(pos < 3);
    return vertices[pos];
  }

  inline const Vector3 &Triangle::GetTextureVertex(unsigned char pos) const {
    return GetTextureVertex(0, pos);
  }

  inline const Vector3 &Triangle::GetTextureVertex(unsigned char texture_unit, unsigned char pos) const {
    //assert(pos < 3);
    //assert(texture_unit < 8);
    return textureVertices[pos][texture_unit];
  }

  inline const Vector3 &Triangle::GetNormal(unsigned char pos) const {
    //assert(pos < 3);
    return normals[pos];
  }

  void Triangle::Rewind() {
    // reverses vertex winding
    Vector3 tmp = vertices[1];
    vertices[1] = vertices[2];
    vertices[2] = tmp;
  }

  AABB Triangle::GetAABB() {
    AABB aabb;

    aabb.SetMinXYZ(Vector3(vertices[0].coords[0], vertices[0].coords[1], vertices[0].coords[2]));
    aabb.SetMaxXYZ(Vector3(vertices[0].coords[0], vertices[0].coords[1], vertices[0].coords[2]));

    for (int i = 1; i < 3; i++) {
      if (vertices[i].coords[0] < aabb.minxyz.coords[0]) aabb.minxyz.coords[0] = vertices[i].coords[0];
      if (vertices[i].coords[0] > aabb.maxxyz.coords[0]) aabb.maxxyz.coords[0] = vertices[i].coords[0];
      if (vertices[i].coords[1] < aabb.minxyz.coords[1]) aabb.minxyz.coords[1] = vertices[i].coords[1];
      if (vertices[i].coords[1] > aabb.maxxyz.coords[1]) aabb.maxxyz.coords[1] = vertices[i].coords[1];
      if (vertices[i].coords[2] < aabb.minxyz.coords[2]) aabb.minxyz.coords[2] = vertices[i].coords[2];
      if (vertices[i].coords[2] > aabb.maxxyz.coords[2]) aabb.maxxyz.coords[2] = vertices[i].coords[2];
    }

    aabb.MakeDirty();

    return aabb;
  }


  // ----- intersections

  void Triangle::IntersectsPlane(bool intersects[], const Vector3 &pvector, const Vector3 &pnormal) const {
    // returns:
    //   bool[edge] intersects plane, where
    //   edge = 0: vertex0 to vertex1
    //   edge = 1: vertex1 to vertex2
    //   edge = 2: vertex2 to vertex0

    intersects[0] = false;
    intersects[1] = false;
    intersects[2] = false;

    real determinant = -pvector.GetDotProduct(pnormal);

    // determinant + pnormal.coords[0] * pvector.coords[0] +
    //               pnormal.coords[1] * pvector.coords[1] +
    //               pnormal.coords[2] * pvector.coords[2] = 0;
    // printf("base point: %f %f %f; base normal: %f %f %f\n", pvector.coords[0], pvector.coords[1], pvector.coords[2], pnormal.coords[0], pnormal.coords[1], pnormal.coords[2]);

    real distance[3];

    for (int i = 0; i < 3; i++) {
      distance[i] = determinant + pnormal.coords[0] * vertices[i].coords[0] +
                                  pnormal.coords[1] * vertices[i].coords[1] +
                                  pnormal.coords[2] * vertices[i].coords[2];
      //printf("distance: %f glunits\n", distance[i]);
    }

    if ((distance[0] == 0 && distance[1] == 0) ||
        (distance[1] == 0 && distance[2] == 0) ||
        (distance[2] == 0 && distance[0] == 0)) {
      // one triangle edge is aligned with the plane. no intersection
      // else, a triangle x(0, -100, 0) would detect intersection; 0 == pos, -100 == neg
      return;
    }

    if (sign(distance[0]) != sign(distance[1])) intersects[0] = true;
    if (sign(distance[1]) != sign(distance[2])) intersects[1] = true;
    if (sign(distance[2]) != sign(distance[0])) intersects[2] = true;
  }

  bool Triangle::IntersectsLine(const Line &u_ray) const {
    Vector3 tmp;
    return IntersectsLine(u_ray, tmp);
  }

  bool Triangle::IntersectsLine(const Line &u_ray, Vector3 &intersectVec) const {
    Vector3    u, v;                // triangle vectors
    Vector3    dir, w0, w;          // ray vectors
    real       r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = vertices[1] - vertices[0];
    v = vertices[2] - vertices[0];

    dir = u_ray.GetVertex(1) - u_ray.GetVertex(0);             // ray direction vector
    w0 = u_ray.GetVertex(0) - vertices[0];
    a = -normals[0].GetDotProduct(w0);
    b = normals[0].GetDotProduct(dir);
    if (fabs(b) < 0.000001f) {     // ray is parallel to triangle plane
      if (a == 0)                // ray lies in triangle plane
        return false; // 2
      else return false;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    // hack to make edges look better, should be < 0 and > 1

    if (r <= 0.0 || r >= 1.0)                   // ray goes away from triangle
      return false;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect

    intersectVec = u_ray.GetVertex(0) + dir * r;           // intersect point of ray and plane

    // is I inside T?
    real uu, uv, vv, wu, wv, D;
    uu = u.GetDotProduct(u);
    uv = u.GetDotProduct(v);
    vv = v.GetDotProduct(v);
    w = intersectVec - vertices[0];
    wu = w.GetDotProduct(u);
    wv = w.GetDotProduct(v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    real s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
      return false;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
      return false;

    return true;                      // I is in T
  }


  bool Triangle::IsCoplanar(const Vector3 &N, const Triangle &triangle) const {
    Vector3 A;
    short i0,i1;

    // first project onto an axis-aligned plane, that maximizes the area
    // of the triangles, compute indices: i0,i1.
    A.coords[0] = fabs(N.coords[0]);
    A.coords[1] = fabs(N.coords[1]);
    A.coords[2] = fabs(N.coords[2]);
    if (A.coords[0] > A.coords[1]) {
      if (A.coords[0] > A.coords[2]) {
        i0 = 1;      /* A[0] is greatest */
        i1 = 2;
      } else {
        i0 = 0;      /* A[2] is greatest */
        i1 = 1;
      }
    } else /* A[0]<=A[1] */ {
      if (A.coords[2]>A.coords[1]) {
        i0 = 0;      /* A[2] is greatest */
        i1 = 1;
      } else {
        i0 = 0;      /* A[1] is greatest */
        i1 = 2;
      }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(vertices[0],vertices[1],triangle.GetVertex(0),triangle.GetVertex(1),triangle.GetVertex(2));
    EDGE_AGAINST_TRI_EDGES(vertices[1],vertices[2],triangle.GetVertex(0),triangle.GetVertex(1),triangle.GetVertex(2));
    EDGE_AGAINST_TRI_EDGES(vertices[2],vertices[0],triangle.GetVertex(0),triangle.GetVertex(1),triangle.GetVertex(2));

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(vertices[0],triangle.GetVertex(0),triangle.GetVertex(1),triangle.GetVertex(2));
    POINT_IN_TRI(triangle.GetVertex(0),vertices[0],vertices[1],vertices[2]);

    return false;
  }

  bool Triangle::IntersectsTriangle(const Triangle &triangle) const {
    /* Triangle/triangle intersection test routine,
       by Tomas Moller, 1997. code editor for blunted
       See article "A Fast Triangle-Triangle Intersection Test",
       Journal of Graphics Tools, 2(2), 1997 */

    Vector3 E1, E2;
    Vector3 N1, N2;
    real d1, d2;
    real du0, du1, du2, dv0, dv1, dv2;
    Vector3 D;
    real isect1[2], isect2[2];
    real du0du1 = 0, du0du2 = 0, dv0dv1 = 0, dv0dv2 = 0;
    short index;
    real vp0, vp1, vp2;
    real up0, up1, up2;
    real bb, cc, max;
    real a, b, c, x0, x1;
    real d, e, f, y0, y1;
    real xx, yy, xxyy, tmp;

    // compute plane equation of triangle(V0,V1,V2)
    E1 = vertices[1] - vertices[0];
    E2 = vertices[2] - vertices[0];
    N1 = E1.GetCrossProduct(E2);
    d1 = -N1.GetDotProduct(vertices[0]);
    // plane equation 1: N1.X+d1=0

    // put U0,U1,U2 into plane equation 1 to compute signed distances to the plane
    du0 = N1.GetDotProduct(triangle.GetVertex(0)) + d1;
    du1 = N1.GetDotProduct(triangle.GetVertex(1)) + d1;
    du2 = N1.GetDotProduct(triangle.GetVertex(2)) + d1;

    // compute plane of triangle (U0, U1, U2)
    E1 = triangle.GetVertex(1) - triangle.GetVertex(0);
    E2 = triangle.GetVertex(2) - triangle.GetVertex(0);
    N2 = E1.GetCrossProduct(E2);
    d2 = -N2.GetDotProduct(triangle.GetVertex(0));
    // plane equation 2: N2.X+d2=0

    // put V0,V1,V2 into plane equation 2
    dv0 = N2.GetDotProduct(vertices[0]) + d2;
    dv1 = N2.GetDotProduct(vertices[1]) + d2;
    dv2 = N2.GetDotProduct(vertices[2]) + d2;

    // compute direction of intersection line
    D = N1.GetCrossProduct(N2);

    // compute and index to the largest component of D
    max = fabs(D.coords[0]);
    index = 0;
    bb = fabs(D.coords[1]);
    cc = fabs(D.coords[2]);
    if (bb > max) max = bb, index = 1;
    if (cc > max) max = cc, index = 2;

    /* this is the simplified projection onto L*/
    vp0 = vertices[0].coords[index];
    vp1 = vertices[1].coords[index];
    vp2 = vertices[2].coords[index];

    up0 = triangle.GetVertex(0).coords[index];
    up1 = triangle.GetVertex(1).coords[index];
    up2 = triangle.GetVertex(2).coords[index];

    // compute interval for triangle 1
    COMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

    // compute interval for triangle 2
    COMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

    xx = x0 * x1;
    yy = y0 * y1;
    xxyy = xx * yy;

    tmp = a * xxyy;
    isect1[0] = tmp + b * x1 * yy;
    isect1[1] = tmp + c * x0 * yy;

    tmp = d * xxyy;
    isect2[0] = tmp + e * xx * y1;
    isect2[1] = tmp + f * xx * y0;

    SORT(isect1[0], isect1[1]);
    SORT(isect2[0], isect2[1]);

    if (isect1[1] < isect2[0] || isect2[1] < isect1[0]) return false;
    return true;
  }


  // ----- utility

  void Triangle::CalculateTangents() {

    // http://www.3dkingdoms.com/weekly/weekly.php?a=37

    Vector3 edge1 = vertices[1] - vertices[0];
    Vector3 edge2 = vertices[2] - vertices[0];
    Vector3 edge1uv = GetTextureVertex(1) - GetTextureVertex(0);
    Vector3 edge2uv = GetTextureVertex(2) - GetTextureVertex(0);

    float cp = edge1uv.coords[1] * edge2uv.coords[0] - edge1uv.coords[0] * edge2uv.coords[1];

    if (cp != 0.0f) {
      float mul = 1.0f / cp;
      for (int v = 0; v < 3; v++) {
        tangents[v]   = (edge1 * -edge2uv.coords[1] + edge2 * edge1uv.coords[1]) * mul;
        biTangents[v] = (edge1 * -edge2uv.coords[0] + edge2 * edge1uv.coords[0]) * mul;
        tangents[v].Normalize();
        biTangents[v].Normalize();

        // handedness check: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#Handedness
        // "To check whether it must be inverted or not, the check is simple : TBN must form a right-handed coordinate system, i.e. cross(n,t) must have the same orientation than b.
        //  In mathematics, “Vector A has the same orientation as Vector B” translates as dot(A,B)>0, so we need to check if dot( cross(n,t) , b ) > 0.
        //  If it’s false, just invert t :
        //  if (glm::dot(glm::cross(n, t), b) < 0.0f) {
        //    t = t * -1.0f;
        //  }"
        Vector3 crossProduct = normals[v].GetCrossProduct(tangents[v]);
        float dotProduct = crossProduct.GetDotProduct(biTangents[v]);
        //printf("dot: %f\n", dotProduct);
        // check for > 0 and change bitangents, since we want a left handed TBN (todo: is that so?)
        if (dotProduct > 0.0f) {
          biTangents[v] = -biTangents[v];
        }

      }
    } else {
      for (int v = 0; v < 3; v++) {
        // no texture coords; make something up
        tangents[v].Set(1, 0, 0);
        biTangents[v].Set(0, 1, 0);
      }
    }
  }

  inline const Vector3 &Triangle::GetTangent(unsigned char pos) const {
    //assert(pos < 3);
    return tangents[pos];
  }

  inline const Vector3 &Triangle::GetBiTangent(unsigned char pos) const {
    //assert(pos < 3);
    return biTangents[pos];
  }

}

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
}

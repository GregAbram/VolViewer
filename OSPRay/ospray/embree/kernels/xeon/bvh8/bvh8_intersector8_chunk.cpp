// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "bvh8_intersector8_chunk.h"
#include "geometry/triangle4_intersector8_moeller.h"
#include "geometry/triangle8_intersector8_moeller.h"

#if EMBREE_AVX512_WORKAROUND
# include "embree2/rtcore_ray.h"
#endif

#define DBG(x) 

#define START_SINDEX 5


#ifdef __EMBREE_KNL_WORKAROUND__
#include "embree2/rtcore_ray.h"

RTCORE_API void rtcIntersect16 (const void* valid, RTCScene scene, RTCRay16& ray) 
{
  printf("we're getting a intersect16 here, but only have avx2 .... let's chop it up into 2x8\n");
  
  const embree::avxf *valid8 = (const embree::avxf *)valid;
  RTCRay8 ray8[2];
  
#define SPLIT(field)                                                  \
  *(embree::avxf *)ray8[0].field = ((embree::avxf *)ray.field)[0];    \
  *(embree::avxf *)ray8[1].field = ((embree::avxf *)ray.field)[1]; 
  
  //   /* ray data */
  // public:
  //   float orgx[8];  //!< x coordinate of ray origin
  //   float orgy[8];  //!< y coordinate of ray origin
  //   float orgz[8];  //!< z coordinate of ray origin
  SPLIT(orgx);
  SPLIT(orgy);
  SPLIT(orgz);
  
  //   float dirx[8];  //!< x coordinate of ray direction
  //   float diry[8];  //!< y coordinate of ray direction
  //   float dirz[8];  //!< z coordinate of ray direction 
  SPLIT(dirx);
  SPLIT(diry);
  SPLIT(dirz);
  
  //   float tnear[8]; //!< Start of ray segment 
  //   float tfar[8];  //!< End of ray segment (set to hit distance)
  SPLIT(tnear);
  SPLIT(tfar);
  
  //   float time[8];  //!< Time of this ray for motion blur
  //   int   mask[8];  //!< Used to mask out objects during traversal
  SPLIT(time);
  SPLIT(mask);
  
  //   /* hit data */
  // public:
  //   float Ngx[8];   //!< x coordinate of geometry normal
  //   float Ngy[8];   //!< y coordinate of geometry normal
  //   float Ngz[8];   //!< z coordinate of geometry normal
  SPLIT(Ngx);
  SPLIT(Ngy);
  SPLIT(Ngz);
  
  //   float u[8];     //!< Barycentric u coordinate of hit
  //   float v[8];     //!< Barycentric v coordinate of hit
  SPLIT(u);
  SPLIT(v);
  
  //   int   geomID[8];  //!< geometry ID
  //   int   primID[8];  //!< primitive ID
  //   int   instID[8];  //!< instance ID
  SPLIT(geomID);
  SPLIT(primID);
  SPLIT(instID);
  
#undef SPLIT
  
  rtcIntersect8((const void *)&valid8[0],scene,ray8[0]);
  rtcIntersect8((const void *)&valid8[1],scene,ray8[1]);
  
#define REMERGE(field)                                                \
    ((embree::avxf *)ray.field)[0] = *(embree::avxf *)ray8[0].field;  \
    ((embree::avxf *)ray.field)[1] = *(embree::avxf *)ray8[1].field;  \
    
  //   float tnear[8]; //!< Start of ray segment 
  //   float tfar[8];  //!< End of ray segment (set to hit distance)
  REMERGE(tnear);
  REMERGE(tfar);
  //   /* hit data */
  // public:
  //   float Ngx[8];   //!< x coordinate of geometry normal
  //   float Ngy[8];   //!< y coordinate of geometry normal
  //   float Ngz[8];   //!< z coordinate of geometry normal
  REMERGE(Ngx);
  REMERGE(Ngy);
  REMERGE(Ngz);
  
  //   float u[8];     //!< Barycentric u coordinate of hit
  //   float v[8];     //!< Barycentric v coordinate of hit
  REMERGE(u);
  REMERGE(v);
  
  //   int   geomID[8];  //!< geometry ID
  //   int   primID[8];  //!< primitive ID
  //   int   instID[8];  //!< instance ID
  REMERGE(geomID);
  REMERGE(primID);
  REMERGE(instID);
#undef REMERGE
  // ((Scene*)scene)->intersect16(valid,ray);
  
  }
#endif

namespace embree
{
#if EMBREE_AVX512_WORKAROUND && __AVX2__
  RTCORE_API void rtcIntersect16_doublePumpedIntersect8 (const void* valid, RTCScene scene, RTCRay16& ray) 
  {
    avxi valid0 = ((avxi *)valid)[0];
    avxi valid1 = ((avxi *)valid)[1];
    RTCRay8 ray0, ray1;

#define SPLIT(member) \
    (avxf&)ray0.member[0] = (avxf&)ray.member[0]; \
    (avxf&)ray1.member[0] = (avxf&)ray.member[8]; 

    //   /* ray data */
    // public:
    //   float orgx[8];  //!< x coordinate of ray origin
    SPLIT(orgx);
    //   float orgy[8];  //!< y coordinate of ray origin
    SPLIT(orgy);
    //   float orgz[8];  //!< z coordinate of ray origin
    SPLIT(orgz);
  
    //   float dirx[8];  //!< x coordinate of ray direction
    SPLIT(dirx);
    //   float diry[8];  //!< y coordinate of ray direction
    SPLIT(diry);
    //   float dirz[8];  //!< z coordinate of ray direction
    SPLIT(dirz);
  
    //   float tnear[8]; //!< Start of ray segment 
    SPLIT(tnear);
    //   float tfar[8];  //!< End of ray segment (set to hit distance)
    SPLIT(tfar);

    //   float time[8];  //!< Time of this ray for motion blur
    SPLIT(time);
    //   int   mask[8];  //!< Used to mask out objects during traversal
    SPLIT(mask);
  
    //   /* hit data */
    // public:
    //   float Ngx[8];   //!< x coordinate of geometry normal
    SPLIT(Ngx);
    //   float Ngy[8];   //!< y coordinate of geometry normal
    SPLIT(Ngy);
    //   float Ngz[8];   //!< z coordinate of geometry normal
    SPLIT(Ngz);
  
    //   float u[8];     //!< Barycentric u coordinate of hit
    SPLIT(u);
    //   float v[8];     //!< Barycentric v coordinate of hit
    SPLIT(v);
  
    //   int   geomID[8];  //!< geometry ID
    SPLIT(geomID);
    //   int   primID[8];  //!< primitive ID
    SPLIT(primID);
    //   int   instID[8];  //!< instance ID
    SPLIT(instID);

    rtcIntersect8(&valid0,scene,ray0);
    rtcIntersect8(&valid1,scene,ray1);

#define MERGE(member) \
    (avxf&)ray.member[0] = (avxf&)ray0.member[0];      \
    (avxf&)ray.member[8] = (avxf&)ray1.member[0];                             

    MERGE(tfar);
    MERGE(u);
    MERGE(v);
    MERGE(Ngx);
    MERGE(Ngy);
    MERGE(Ngz);
    MERGE(geomID);
    MERGE(primID);
    MERGE(instID);
#undef SPLIT
#undef MERGE
  }

  RTCORE_API void rtcOccluded16_doublePumpedOccluded8 (const void* valid, RTCScene scene, RTCRay16& ray) 
  {
    avxi valid0 = ((avxi *)valid)[0];
    avxi valid1 = ((avxi *)valid)[1];
    RTCRay8 ray0, ray1;

#define SPLIT(member) \
    (avxf&)ray0.member[0] = (avxf&)ray.member[0]; \
    (avxf&)ray1.member[0] = (avxf&)ray.member[8]; 

    //   /* ray data */
    // public:
    //   float orgx[8];  //!< x coordinate of ray origin
    SPLIT(orgx);
    //   float orgy[8];  //!< y coordinate of ray origin
    SPLIT(orgy);
    //   float orgz[8];  //!< z coordinate of ray origin
    SPLIT(orgz);
  
    //   float dirx[8];  //!< x coordinate of ray direction
    SPLIT(dirx);
    //   float diry[8];  //!< y coordinate of ray direction
    SPLIT(diry);
    //   float dirz[8];  //!< z coordinate of ray direction
    SPLIT(dirz);
  
    //   float tnear[8]; //!< Start of ray segment 
    SPLIT(tnear);
    //   float tfar[8];  //!< End of ray segment (set to hit distance)
    SPLIT(tfar);

    //   float time[8];  //!< Time of this ray for motion blur
    SPLIT(time);
    //   int   mask[8];  //!< Used to mask out objects during traversal
    SPLIT(mask);
  
    //   /* hit data */
    // public:
    //   float Ngx[8];   //!< x coordinate of geometry normal
    SPLIT(Ngx);
    //   float Ngy[8];   //!< y coordinate of geometry normal
    SPLIT(Ngy);
    //   float Ngz[8];   //!< z coordinate of geometry normal
    SPLIT(Ngz);
  
    //   float u[8];     //!< Barycentric u coordinate of hit
    SPLIT(u);
    //   float v[8];     //!< Barycentric v coordinate of hit
    SPLIT(v);
  
    //   int   geomID[8];  //!< geometry ID
    SPLIT(geomID);
    //   int   primID[8];  //!< primitive ID
    SPLIT(primID);
    //   int   instID[8];  //!< instance ID
    SPLIT(instID);

    rtcOccluded8(&valid0,scene,ray0);
    rtcOccluded8(&valid1,scene,ray1);

#define MERGE(member) \
    (avxf&)ray.member[0] = (avxf&)ray0.member[0];      \
    (avxf&)ray.member[8] = (avxf&)ray1.member[0];                             

    MERGE(tfar);
    MERGE(u);
    MERGE(v);
    MERGE(Ngx);
    MERGE(Ngy);
    MERGE(Ngz);
    MERGE(geomID);
    MERGE(primID);
    MERGE(instID);
#undef SPLIT
#undef MERGE
  }

#endif

  namespace isa
  {    
    
    template<typename PrimitiveIntersector8>    
    void BVH8Intersector8Chunk<PrimitiveIntersector8>::intersect(avxb* valid_i, BVH8* bvh, Ray8& ray)
    {
#if defined(__AVX__)
      
      /* load ray */
      const avxb valid0 = *valid_i;
      const avx3f rdir = rcp_safe(ray.dir);
      const avx3f org_rdir = ray.org * rdir;
      avxf ray_tnear = select(valid0,ray.tnear,pos_inf);
      avxf ray_tfar  = select(valid0,ray.tfar ,neg_inf);
      const avxf inf = avxf(pos_inf);
      Precalculations pre(valid0,ray);
      
      /* allocate stack and push root node */
      avxf    stack_near[3*BVH8::maxDepth+1];
      NodeRef stack_node[3*BVH8::maxDepth+1];
      stack_node[0] = BVH8::invalidNode;
      stack_near[0] = inf;
      stack_node[1] = bvh->root;
      stack_near[1] = ray_tnear; 
      NodeRef* __restrict__ sptr_node = stack_node + 2;
      avxf*    __restrict__ sptr_near = stack_near + 2;
      
      while (1)
      {
        /* pop next node from stack */
        sptr_node--;
        sptr_near--;
        NodeRef cur = *sptr_node;
        if (unlikely(cur == BVH8::invalidNode)) 
          break;
        
        /* cull node if behind closest hit point */
        avxf curDist = *sptr_near;
        if (unlikely(none(ray_tfar > curDist))) 
          continue;
        
        while (1)
        {
          /* test if this is a leaf node */
          if (unlikely(cur.isLeaf()))
            break;
          
          const avxb valid_node = ray_tfar > curDist;
          STAT3(normal.trav_nodes,1,popcnt(valid_node),8);
          const Node* __restrict__ const node = (BVH8::Node*)cur.node();
          
          /* pop of next node */
          sptr_node--;
          sptr_near--;
          cur = *sptr_node; // FIXME: this trick creates issues with stack depth
          curDist = *sptr_near;
          
          for (unsigned i=0; i<BVH8::N; i++)
          {
            const NodeRef child = node->children[i];
            if (unlikely(child == BVH8::emptyNode)) break;
            
#if defined(__AVX2__)
            const avxf lclipMinX = msub(node->lower_x[i],rdir.x,org_rdir.x);
            const avxf lclipMinY = msub(node->lower_y[i],rdir.y,org_rdir.y);
            const avxf lclipMinZ = msub(node->lower_z[i],rdir.z,org_rdir.z);
            const avxf lclipMaxX = msub(node->upper_x[i],rdir.x,org_rdir.x);
            const avxf lclipMaxY = msub(node->upper_y[i],rdir.y,org_rdir.y);
            const avxf lclipMaxZ = msub(node->upper_z[i],rdir.z,org_rdir.z);
            const avxf lnearP = maxi(maxi(mini(lclipMinX, lclipMaxX), mini(lclipMinY, lclipMaxY)), mini(lclipMinZ, lclipMaxZ));
            const avxf lfarP  = mini(mini(maxi(lclipMinX, lclipMaxX), maxi(lclipMinY, lclipMaxY)), maxi(lclipMinZ, lclipMaxZ));
            const avxb lhit   = maxi(lnearP,ray_tnear) <= mini(lfarP,ray_tfar);      
#else
            const avxf lclipMinX = node->lower_x[i] * rdir.x - org_rdir.x;
            const avxf lclipMinY = node->lower_y[i] * rdir.y - org_rdir.y;
            const avxf lclipMinZ = node->lower_z[i] * rdir.z - org_rdir.z;
            const avxf lclipMaxX = node->upper_x[i] * rdir.x - org_rdir.x;
            const avxf lclipMaxY = node->upper_y[i] * rdir.y - org_rdir.y;
            const avxf lclipMaxZ = node->upper_z[i] * rdir.z - org_rdir.z;
            const avxf lnearP = max(max(min(lclipMinX, lclipMaxX), min(lclipMinY, lclipMaxY)), min(lclipMinZ, lclipMaxZ));
            const avxf lfarP  = min(min(max(lclipMinX, lclipMaxX), max(lclipMinY, lclipMaxY)), max(lclipMinZ, lclipMaxZ));
            const avxb lhit   = max(lnearP,ray_tnear) <= min(lfarP,ray_tfar);      
#endif
            
            /* if we hit the child we choose to continue with that child if it 
               is closer than the current next child, or we push it onto the stack */
            if (likely(any(lhit)))
            {
              const avxf childDist = select(lhit,lnearP,inf);
              const NodeRef child = node->children[i];
              
              /* push cur node onto stack and continue with hit child */
              if (any(childDist < curDist))
              {
                *sptr_node = cur;
                *sptr_near = curDist; 
		sptr_node++;
		sptr_near++;

                curDist = childDist;
                cur = child;
              }
              
              /* push hit child onto stack*/
              else {
                *sptr_node = child;
                *sptr_near = childDist; 
		sptr_node++;
		sptr_near++;

              }
              assert(sptr_node - stack_node < BVH8::maxDepth);
            }	      
          }
        }
        
        /* return if stack is empty */
        if (unlikely(cur == BVH8::invalidNode)) 
          break;
        
        /* intersect leaf */
	assert(cur != BVH8::emptyNode);
        const avxb valid_leaf = ray_tfar > curDist;
        STAT3(normal.trav_leaves,1,popcnt(valid_leaf),8);
        size_t items; const Triangle* tri  = (Triangle*) cur.leaf(items);
        PrimitiveIntersector8::intersect(valid_leaf,pre,ray,tri,items,bvh->geometry);
        ray_tfar = select(valid_leaf,ray.tfar,ray_tfar);
      }
      AVX_ZERO_UPPER();
#endif       
    }
    
     template<typename PrimitiveIntersector8>
    void BVH8Intersector8Chunk<PrimitiveIntersector8>::occluded(avxb* valid_i, BVH8* bvh, Ray8& ray)
    {
#if defined(__AVX__)
      
      /* load ray */
      const avxb valid = *valid_i;
      avxb terminated = !valid;
      const avx3f rdir = rcp_safe(ray.dir);
      const avx3f org_rdir = ray.org * rdir;
      avxf ray_tnear = select(valid,ray.tnear,pos_inf);
      avxf ray_tfar  = select(valid,ray.tfar ,neg_inf);
      const avxf inf = avxf(pos_inf);
      Precalculations pre(valid,ray);

      /* allocate stack and push root node */
      avxf    stack_near[3*BVH8::maxDepth+1];
      NodeRef stack_node[3*BVH8::maxDepth+1];
      stack_node[0] = BVH8::invalidNode;
      stack_near[0] = inf;
      stack_node[1] = bvh->root;
      stack_near[1] = ray_tnear; 
      NodeRef* __restrict__ sptr_node = stack_node + 2;
      avxf*    __restrict__ sptr_near = stack_near + 2;
      
      while (1)
      {
        /* pop next node from stack */
        sptr_node--;
        sptr_near--;
        NodeRef cur = *sptr_node;
        if (unlikely(cur == BVH8::invalidNode)) 
          break;
        
        /* cull node if behind closest hit point */
        avxf curDist = *sptr_near;
        if (unlikely(none(ray_tfar > curDist))) 
          continue;
        
        while (1)
        {
          /* test if this is a leaf node */
          if (unlikely(cur.isLeaf()))
            break;
          
          const avxb valid_node = ray_tfar > curDist;
          STAT3(shadow.trav_nodes,1,popcnt(valid_node),8);
          const Node* __restrict__ const node = (Node*)cur.node();
          
          /* pop of next node */
          sptr_node--;
          sptr_near--;
          cur = *sptr_node; // FIXME: this trick creates issues with stack depth
          curDist = *sptr_near;
          
          for (unsigned i=0; i<BVH8::N; i++)
          {
            const NodeRef child = node->children[i];
            if (unlikely(child == BVH8::emptyNode)) break;
            
#if defined(__AVX2__)
            const avxf lclipMinX = msub(node->lower_x[i],rdir.x,org_rdir.x);
            const avxf lclipMinY = msub(node->lower_y[i],rdir.y,org_rdir.y);
            const avxf lclipMinZ = msub(node->lower_z[i],rdir.z,org_rdir.z);
            const avxf lclipMaxX = msub(node->upper_x[i],rdir.x,org_rdir.x);
            const avxf lclipMaxY = msub(node->upper_y[i],rdir.y,org_rdir.y);
            const avxf lclipMaxZ = msub(node->upper_z[i],rdir.z,org_rdir.z);
            const avxf lnearP = maxi(maxi(mini(lclipMinX, lclipMaxX), mini(lclipMinY, lclipMaxY)), mini(lclipMinZ, lclipMaxZ));
            const avxf lfarP  = mini(mini(maxi(lclipMinX, lclipMaxX), maxi(lclipMinY, lclipMaxY)), maxi(lclipMinZ, lclipMaxZ));
            const avxb lhit   = maxi(lnearP,ray_tnear) <= mini(lfarP,ray_tfar);      
#else
            const avxf lclipMinX = node->lower_x[i] * rdir.x - org_rdir.x;
            const avxf lclipMinY = node->lower_y[i] * rdir.y - org_rdir.y;
            const avxf lclipMinZ = node->lower_z[i] * rdir.z - org_rdir.z;
            const avxf lclipMaxX = node->upper_x[i] * rdir.x - org_rdir.x;
            const avxf lclipMaxY = node->upper_y[i] * rdir.y - org_rdir.y;
            const avxf lclipMaxZ = node->upper_z[i] * rdir.z - org_rdir.z;
            const avxf lnearP = max(max(min(lclipMinX, lclipMaxX), min(lclipMinY, lclipMaxY)), min(lclipMinZ, lclipMaxZ));
            const avxf lfarP  = min(min(max(lclipMinX, lclipMaxX), max(lclipMinY, lclipMaxY)), max(lclipMinZ, lclipMaxZ));
            const avxb lhit   = max(lnearP,ray_tnear) <= min(lfarP,ray_tfar);      
#endif
            
            /* if we hit the child we choose to continue with that child if it 
               is closer than the current next child, or we push it onto the stack */
            if (likely(any(lhit)))
            {
              const avxf childDist = select(lhit,lnearP,inf);
              sptr_node++;
              sptr_near++;
              
              /* push cur node onto stack and continue with hit child */
              if (any(childDist < curDist))
              {
                *(sptr_node-1) = cur;
                *(sptr_near-1) = curDist; 
                curDist = childDist;
                cur = child;
              }
              
              /* push hit child onto stack*/
              else {
                *(sptr_node-1) = child;
                *(sptr_near-1) = childDist; 
              }
              assert(sptr_node - stack_node < BVH8::maxDepth);
            }	      
          }
        }
        
        /* return if stack is empty */
        if (unlikely(cur == BVH8::invalidNode)) 
          break;
        
        /* intersect leaf */
	assert(cur != BVH8::emptyNode);
        const avxb valid_leaf = ray_tfar > curDist;
        STAT3(shadow.trav_leaves,1,popcnt(valid_leaf),8);
        size_t items; const Triangle* tri  = (Triangle*) cur.leaf(items);
        terminated |= PrimitiveIntersector8::occluded(!terminated,pre,ray,tri,items,bvh->geometry);
        if (all(terminated)) break;
        ray_tfar = select(terminated,neg_inf,ray_tfar);
      }
      store8i(valid & terminated,&ray.geomID,0);
      AVX_ZERO_UPPER();
#endif      
    }
    
    DEFINE_INTERSECTOR8(BVH8Triangle4Intersector8ChunkMoeller,BVH8Intersector8Chunk<LeafIterator8<Triangle4Intersector8MoellerTrumbore<LeafMode COMMA true> > >);
    DEFINE_INTERSECTOR8(BVH8Triangle8Intersector8ChunkMoeller,BVH8Intersector8Chunk<LeafIterator8<Triangle8Intersector8MoellerTrumbore<LeafMode COMMA true> > >);
  }
}  

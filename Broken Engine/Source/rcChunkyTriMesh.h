//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// 
#ifndef CHUNKYTRIMESH_H
#define CHUNKYTRIMESH_H
typedef unsigned char uchar;

struct rcChunkyTriMeshNode {
	float bmin[2];
	float bmax[2];
	int i;
	int n;
};

// ---Modified by Sergi Parra for Broken Engine ----
struct rcChunkyTriMesh {
	inline rcChunkyTriMesh() : nodes(0), nnodes(0), tris(0), areas(0), ntris(0), maxTrisPerChunk(0) {};
	inline ~rcChunkyTriMesh() { delete[] nodes; delete[] tris; delete[] areas; }

	rcChunkyTriMeshNode* nodes;
	int nnodes;
	int* tris;
	uchar* areas;
	int ntris;
	int maxTrisPerChunk;

private:
	// Explicitly disabled copy constructor and copy assignment operator.
	rcChunkyTriMesh(const rcChunkyTriMesh&);
	rcChunkyTriMesh& operator=(const rcChunkyTriMesh&);
};

// ---Modified by Sergi Parra for Broken Engine ----
/// Creates partitioned triangle mesh (AABB tree),
/// where each node contains at max trisPerChunk triangles.
bool rcCreateChunkyTriMesh(const float* verts, const int* tris, const uchar* areas, int ntris,
	int trisPerChunk, rcChunkyTriMesh* cm);

/// Returns the chunk indices which overlap the input rectable.
int rcGetChunksOverlappingRect(const rcChunkyTriMesh* cm, float bmin[2], float bmax[2], int* ids, const int maxIds);

/// Returns the chunk indices which overlap the input segment.
int rcGetChunksOverlappingSegment(const rcChunkyTriMesh* cm, float p[2], float q[2], int* ids, const int maxIds);


#endif // CHUNKYTRIMESH_H

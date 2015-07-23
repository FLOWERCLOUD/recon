#ifndef NCD_SKELETON_H_INCLUDED
#define NCD_SKELETON_H_INCLUDED

//#include "dynamicArray.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////
// CONSTANTS
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CLASSES
///////////////////////////////////////////////////////////////////////////////

typedef struct {
  VoxelPositionDouble position;
  float radius;
} SkeletonPoint;



///////////////////////////////////////////////////////////////////////////////
// SkeletonSegment - a sequence of points
///////////////////////////////////////////////////////////////////////////////
class SkeletonSegment {
 public:
  // constructor/destructor
  SkeletonSegment() {};
  ~SkeletonSegment() {};

  // Insert a new point at the end of the segment
  bool InsertPoint(SkeletonPoint &point);
  bool InsertPoint(VoxelPositionDouble &point, float radius);
  bool InsertPoint(double x, double y, double z, float radius);

  // computes distance (square of) from a given point to the closest point on
  // the segment
  double DistanceToPoint(VoxelPositionDouble &point, double maxDist,
			 int *closestPoint);

  // returns the number of points in this segment
  int GetNumberOfPoints() const;

  // removes the last N points from the segment
  bool RemoveLastNPoints(int n);

  // get each point
  SkeletonPoint& operator[](int pos);
  const SkeletonPoint& operator[](int pos) const;

  // operator =
  // this is needed so that Skeleton can have a dynamic array of
  // SkeletonSegments -- need assignment
  SkeletonSegment& operator=(const SkeletonSegment& otherSeg);

  // removes all points
  bool RemoveAllPoints();

 private:
  DynamicArray<SkeletonPoint> points;

};


///////////////////////////////////////////////////////////////////////////////
// Skeleton
///////////////////////////////////////////////////////////////////////////////

class Skeleton {
 public:
  Skeleton();
  Skeleton(const Skeleton& skel);
  ~Skeleton() {};

  // set/get skeleton offsets
  bool SetOffsets(int dx, int dy, int dz);
  void GetOffsets(int *dx, int *dy, int *dz) const;

  // insert a new segment
  bool InsertSegment(const SkeletonSegment &skelSeg);

  // Remove a single segment.
  bool RemoveSegment(int seg_num);

  // removes all segments
  bool RemoveAllSegments();

  // computes shortest distance (square of) from a given point to a skeleton
  // point
  // also returns the number of the closest segment and closest point in the
  // segment
  double DistanceToPoint(VoxelPositionDouble &point, double maxDist,
			 int *closestSeg, int *closestPoint);

  // returns the number of segments
  int GetNumberOfSegments() const;


  // get each segment
  SkeletonSegment& operator[](int pos);
  const SkeletonSegment& operator[](int pos) const;
  // assignment operator.
  Skeleton& operator=(const Skeleton& other);

  // save to a file
  bool SaveToFile(char *file,      // [in] output file name
		  char mode = 0    // [in] output: 0 - points, 1 - lines,
		                   // 2 - full
		  );

  // read from file
  bool ReadFromFile(char *file,       // [in] input file name
		    char mode = 0     // [in] mode: 0 - points, 1 - lines,
		                      // 2 - full
		    );

 private:
  DynamicArray<SkeletonSegment> segments;
  int offX, offY, offZ; // offsets created by forcing volume padding to 1
                        // before processing
};



#endif // NCD_SKELETON_H_INCLUDED

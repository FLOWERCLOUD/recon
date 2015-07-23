#include "skeleton.h"
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Class SkeletonSegment
///////////////////////////////////////////////////////////////////////////////

bool SkeletonSegment::InsertPoint(SkeletonPoint &point) {
  return this->points.Append(point);
}


bool SkeletonSegment::InsertPoint(VoxelPositionDouble &point,
				  float radius) {
  SkeletonPoint p;
  p.position = point;
  p.radius = radius;

  return this->points.Append(p);
}


bool SkeletonSegment::InsertPoint(double x, double y, double z, float radius) {
  SkeletonPoint p;
  p.position.x = x;
  p.position.y = y;
  p.position.z = z;
  p.radius = radius;

  return this->points.Append(p);
}


double SkeletonSegment::DistanceToPoint(VoxelPositionDouble &point,
					double maxDist, int *closestPoint)
{
  double mind = maxDist;
	double d;
  int i;

  (*closestPoint) = -1;

  // maxDist is the maximum distance we are interested in. Any point further
  // away than that can be ignored.
  // To reduce the number of operations, I will compute the distance
  // incrementally and exit as soon as d is larger than maxDist

  for(i=0; i < this->points.GetNrElem(); i++) {
    // incremental computation of distance with early exit to improve speed
    d = (this->points[i].position.x - point.x)*
      (this->points[i].position.x - point.x);
    if(d > mind) continue;
    d = d + ((this->points[i].position.y - point.y)*
	     (this->points[i].position.y - point.y));
    if(d > mind) continue;
    d = d + ((this->points[i].position.z - point.z)*
	     (this->points[i].position.z - point.z));
    if(d > mind) continue;

    if(d < mind) {
      mind = d;
      (*closestPoint) = i;
    }
  }
  if((*closestPoint) != -1) {
    return mind;
  }
  return -1;
}


int SkeletonSegment::GetNumberOfPoints() const {
  return this->points.GetNrElem();
}


bool SkeletonSegment::RemoveLastNPoints(int n) {
  for(int i=0; i < n; i++) {
    if(!this->points.RemoveLastElem()) return false;
  }
  return true;
}


SkeletonPoint& SkeletonSegment::operator[](int pos) {
  return this->points[pos];
}

const SkeletonPoint& SkeletonSegment::operator[](int pos) const {
  return this->points[pos];
}


SkeletonSegment& SkeletonSegment::operator=(const SkeletonSegment& otherSeg) {
  this->points = otherSeg.points;
  return *this;
}


bool SkeletonSegment::RemoveAllPoints() {
  return this->points.Reset();
}

///////////////////////////////////////////////////////////////////////////////
// class Skeleton
///////////////////////////////////////////////////////////////////////////////

Skeleton::Skeleton() {
  this->offX = 0;
  this->offY = 0;
  this->offZ = 0;
}

// Copy constructor.
Skeleton::Skeleton(const Skeleton& skel) {
  // we'll make a copy of the segments.
  int nrSegments = skel.GetNumberOfSegments();
  for (int i = 0; i < nrSegments; i++) {
    this->InsertSegment(skel[i]);
  }
  // copy offsets.
  this->offX = skel.offX;
  this->offY = skel.offY;
  this->offZ = skel.offZ;
}

bool Skeleton::SetOffsets(int dx, int dy, int dz) {
  this->offX = dx;
  this->offY = dy;
  this->offZ = dz;
  return true;
}
void Skeleton::GetOffsets(int *dx, int *dy, int *dz) const {
  if ((dx != NULL) && (dy != NULL) && (dz != NULL)) {
    *dx = this->offX;
    *dy = this->offY;
    *dz = this->offZ;
  }
}


// insert a new segment
bool Skeleton::InsertSegment(const SkeletonSegment &skelSeg) {
  return this->segments.Append(skelSeg);
}


double Skeleton::DistanceToPoint(VoxelPositionDouble &point,
					double maxDist,
					int *closestSeg, int *closestPoint)
{
  double d, mind;
  mind = maxDist;
  int cp;

  (*closestSeg) = -1;
  (*closestPoint) = -1;

  for(int i=0; i < this->segments.GetNrElem(); i++) {
    d = this->segments[i].DistanceToPoint(point, mind, &cp);
    if(d >= 0) {
      // it's close to this segment
      if(d < mind) {
	*closestSeg = i;
	*closestPoint = cp;
      }
    }
  }

  if((*closestSeg) != -1) {
    return mind;
  }

  return -1;
}


  // returns the number of segments
int Skeleton::GetNumberOfSegments() const {
  return this->segments.GetNrElem();
}


  // get each segment
SkeletonSegment& Skeleton::operator[](int pos)
{
  return this->segments[pos];
}

const SkeletonSegment& Skeleton::operator[](int pos) const
{
  return this->segments[pos];
}

// Assignment operator
Skeleton& Skeleton::operator=(const Skeleton& other) {
  if (this != &other) {
    // remove all segments from this, and copy the segments from other.
    this->RemoveAllSegments();
    int nrSegments = other.GetNumberOfSegments();
    for (int i = 0; i < nrSegments; i++) {
      this->InsertSegment(other[i]);
    }

    // copy offsets
    this->offX = other.offX;
    this->offY = other.offY;
    this->offZ = other.offZ;
  }
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
// function SaveSkeleton - saves the skeleton to a file
//   mode - 0 (default) saves skeleton as points, with the segment specified
//            for each point
//            format: X Y Z segment 0.5\n
//          1 saves the skeleton as line segments (2 points per line)
//            format: X1 Y1 Z1 X2 Y2 Z2 segment\n
//
//          2 saves the full Skel structure so that it can be recovered exaclty
//            from the file
//            format:
//              SEGMENTS <nrOfSegments>
//              <left> <first> <last> <right>   // <nrOfSegments> lines
//              POINTS <nrOfPoints>
//              <x> <y> <z> <dt>                // <nrOfPoints> lines
//
///////////////////////////////////////////////////////////////////////////////
bool Skeleton::SaveToFile(
  char *file,       // [in] output file name
  char mode         /*=0*/ // [in] output: 0 - points, 1 - lines,
  //                2 - full structure
)
{
  int i, j;
  FILE *fskelout;
	// float spx, spy, spz;
	// float dt;

#ifdef TRACE
  printf("Starting Save Skeleton to File ...n");
  printf("Skeleton:\n");
  printf("\tnumSegments = %d;\n", this->GetNumberOfSegments());
  printf("\tSegments:\n");
  printf("\tNo\tNrPoints");
  for(i=0; i < this->GetNumberOfSegments(); i++) {
    printf("\t%d\t%d\n", i, this->segments[i].GetNumberOfPoints());
  }
  printf("-----\n");
  fflush(stdout);
#endif

  // open the file
  if ((fskelout = fopen(file,"w")) == NULL) {
    printf("Cannot open output file %s for writing\n", file);
    exit(1);
  }

  switch(mode) {
  case 0:
    {
      //
      // write out the skeleton points
      //

      // output each segment
      for(i=0; i < this->GetNumberOfSegments(); i++) {
	for(j=0; j < this->segments[i].GetNumberOfPoints(); j++) {
	  fprintf(fskelout,"%.3f %.3f %.3f %d %.3f\n",
		  this->segments[i][j].position.x + this->offX,
		  this->segments[i][j].position.y + this->offY,
		  this->segments[i][j].position.z + this->offZ,
		  i,
		  this->segments[i][j].radius);
	}
      }
    }
    break;
  case 1:
    {
      int last;
      // output line segments
      for(i=0; i < this->GetNumberOfSegments(); i++) {

	// output the left and right end points of the segment
	// and the segment number
	// we shouldn't have empty segments here ...

	last = this->segments[i].GetNumberOfPoints() - 1;
	if(last < 0) {
	  printf("WARNING: Skeleton::SaveToFile(...) a segment is empty !!\n");
	}
	else {
	  fprintf(fskelout,"%.3f %.3f %.3f %.3f %.3f %.3f %d\n",
		  this->segments[i][0].position.x  + this->offX,
		  this->segments[i][0].position.y  + this->offY,
		  this->segments[i][0].position.z  + this->offZ,
		  this->segments[i][last].position.x + this->offX,
		  this->segments[i][last].position.y + this->offY,
		  this->segments[i][last].position.z + this->offZ,
		  i);
	}
      }
      break;
    }
  case 2:
    {
      //
      // write out the full skeleton structure
      //
      //
      // segments
      //
      fprintf(fskelout,"SEGMENTS: %d\n", this->GetNumberOfSegments());

      for(i=0; i < this->GetNumberOfSegments(); i++) {
	fprintf(fskelout,"-- segment %d POINTS: %d\n",
		i, this->segments[i].GetNumberOfPoints());
	for(j=0; j < this->segments[i].GetNumberOfPoints(); j++) {
	  fprintf(fskelout, "%.3f %.3f %.3f %.3f\n",
		  this->segments[i][j].position.x + this->offX,
		  this->segments[i][j].position.y + this->offY,
		  this->segments[i][j].position.z + this->offZ,
		  this->segments[i][j].radius);
	}
      }
    }
    break;
  default:
    printf("Wrong parameter to SaveSkeleton: %d ! Skeleton was NOT saved.\n",
	   mode);
    break;
  }

  // close the file
  fclose(fskelout);

  return true;
}




///////////////////////////////////////////////////////////////////////////////
// function ReadFromFile - reads the skeleton from a file
//   mode - 0 (default) reads skeleton as points, with the segment specified
//            for each point
//            format: X Y Z segment radius\n
//          1 reads the skeleton as line segments (2 points per line)
//            format: X1 Y1 Z1 X2 Y2 Z2 segment\n
//          2 saves the full Skel strcture so that it can be recovered exaclty
//            from the file
//            format:
//              SEGMENTS: <nrOfSegments>
//              -- segment <number> POINTS: <nrOfPoints>
//              <x> <y> <z> <dt>                // <nrOfPoints> lines
//              -- segment <number> POINTS: <nrOfPoints>
//              <x> <y> <z> <dt>                // <nrOfPoints> lines
//              ...
//              # these lines are comments - ignored
//              ...
///////////////////////////////////////////////////////////////////////////////
bool Skeleton::ReadFromFile(
       char *file,       // [in] input file name
       char mode /*=0*/  // [in] mode: 0 - points, 1 - lines, 2 - full
       )
{
  if((mode != 2) && (mode != 0)) {
    // mode 1 not implemented
    printf("Skeleton::ReadFromFile: mode 1 not implemented !\n");
    return false;
  }

	// int i;
  FILE *fskel;
  float spx, spy, spz;
  float dt;
  int sps;
	// int left, first, right, last;
  int lineCnt = 0;
  char line[1000]; // , line2[1000];
  int expNrSegments, expNrPoints;
  int nrSegments, nrPoints;
  int segNum;
  int prevSkelSeg;

  SkeletonSegment skelSeg;
  bool firstSkelSeg;

  enum {
    SEC_NONE = 0,
    SEC_COMMENTS,
    SEC_SEGMENTS,
    SEC_POINTS
  } sec;

  //   section sec;

  // open the file
  if ((fskel = fopen(file,"r")) == NULL) {
    printf("Cannot open input file %s for reading\n", file);
    return false;
  }

  switch(mode) {
  case 2:
    {
      lineCnt = 0;
      sec = SEC_NONE;  // current section is none
      firstSkelSeg = true;

      while(!feof(fskel)) {
	lineCnt++;
	line[0] = '\0';
	fgets(line, 1000, fskel);

	// printf("read line: <%s>\n", line);

	if(strlen(line) <= 0) {
	  // printf("-- empty --\n");
	  // empty line
	  continue;
	}

	if(strncmp(line, "#", 1) == 0) {
	  //
	  // skip comment lines (begin with #)
	  //
	  sec = SEC_COMMENTS;
	}
	else {
	  if(strncmp(line, "SEGMENTS:", 9) == 0) {
	    sec = SEC_SEGMENTS;

	    expNrSegments = 0;
	    nrSegments = 0;
	    if(sscanf(line, "SEGMENTS %d\n", &expNrSegments) != 1) {
	      printf("ReadSkeleton: Error reading input file in line %d !\n",
		     lineCnt);
	      return false;
	    }

	    // allocate just enough space
	    this->segments.GrowTo(expNrSegments);

	    // printf("Expected number of segments: %d\n", expNrSegments);
	    continue;
	  }
	  else {
	    if(strncmp(line, "-- segment", 10) == 0) {
	      // add previous skeleton segment to the skeleton
	      if(!firstSkelSeg) {
		this->InsertSegment(skelSeg);
	      }

	      firstSkelSeg = false;

	      if(sscanf(line,"-- segment %d POINTS: %d\n",
			&segNum, &expNrPoints) != 2) {
		printf("Skeleton:ReadFromFile: Error reading input file in line %d!\n",
		       lineCnt);
		return false;
	      }

	      // allocate skeleton segment
	      skelSeg.RemoveAllPoints();
	      nrSegments++;

	      sec = SEC_POINTS;
	      nrPoints = 0;
	      continue;
	    }
	  }
	}

	switch(sec) {
	case SEC_NONE:
	  // printf("--no section--\n");
	  break;
	case SEC_COMMENTS:
	  // do nothing here
	  // printf("--comment--\n");
	  break;
	case SEC_SEGMENTS:
	  // printf("--segment--\n");
	  // nothing here
	  break;
	case SEC_POINTS:
	  //printf("--point--\n");
	  //
	  // points
	  //

	  if(sscanf(line,"%f %f %f %f\n", &spx, &spy, &spz, &dt) != 4) {
	    // ignore this line
	    continue;
	  }
	  if(nrPoints >= expNrPoints) {
	    // error
	    printf("Skeleton::ReadFromFile: Read %d points for segment %d. Expected: %d! (line %d)\n",
		   nrPoints, nrSegments, expNrPoints, lineCnt);
	    return false;
	  }
	  skelSeg.InsertPoint(spx, spy, spz, dt);
	  nrPoints++;

	  break;
	default:
	  // nothing here
	  break;
	}
      } // while

      // add the last segment to the skeleton
      if(!firstSkelSeg) {
	this->InsertSegment(skelSeg);
      }


      //
      // check that we have expNrSegments
      //
      if((nrSegments != expNrSegments)) {
	printf("ReadSkeleton: Error reading skeleton file !\n\
Expected nr. segmnets: %d. Read: %d.\n",
	       expNrSegments, nrSegments);

	return false;
      }

    } // case 2
    break;

  case 0:
    {
      lineCnt = 0;
      sec = SEC_NONE;  // current section is none
      prevSkelSeg = -1;
      nrPoints = 0;
      nrSegments = 0;
      firstSkelSeg = true;

      while(!feof(fskel)) {
	lineCnt++;
	line[0] = '\0';
	fgets(line, 1000, fskel);

	// printf("read line: <%s>\n", line);

	if(strlen(line) <= 0) {
	  // printf("-- empty --\n");
	  // empty line
	  continue;
	}

	if(strncmp(line, "#", 1) == 0) {
	  //
	  // skip comment lines (begin with #)
	  //
	  sec = SEC_COMMENTS;
	}
	else {
	  // add previous skeleton segment to the skeleton
	  sec = SEC_POINTS;
	}

	switch(sec) {
	case SEC_NONE:
	  // printf("--no section--\n");
	  break;
	case SEC_COMMENTS:
	  // do nothing here
	  // printf("--comment--\n");
	  break;
	case SEC_SEGMENTS:
	  // printf("--segment--\n");
	  // nothing here
	  break;
	case SEC_POINTS:
	  //printf("--point--\n");
	  //
	  // points
	  //

	  // parse the new line
	  if(sscanf(line,"%f %f %f %d %f\n", &spx, &spy, &spz, &sps, &dt) != 5)
	  {
	    printf("WARNING: Skeleton::ReadFromFile(...): wrong format in line %d. Line was ignored !\n", lineCnt);
	    // ignore this line
	    continue;
	  }

	  if(sps != prevSkelSeg) {
	    // a new segment begins here
	    if(!firstSkelSeg) {
	      // insert the old one in the skeleton
	      this->InsertSegment(skelSeg);
	      nrSegments++;

	      // reset skelSeg
	      skelSeg.RemoveAllPoints();
	    }
	    firstSkelSeg = false;
	  }
	  prevSkelSeg = sps;

	  skelSeg.InsertPoint(spx, spy, spz, dt);
	  nrPoints++;

	  break;
	default:
	  // nothing here
	  break;
	}

      } // while

      // add the last segment to the skeleton
      if(!firstSkelSeg) {
	this->InsertSegment(skelSeg);
	nrSegments++;
      }
      printf("Skeleton.ReadFromFile(...):Read %d points and %d segments\n",
	     nrPoints, nrSegments);
    } // case 0
    break;
  default:
    printf("Wrong parameter (mode) to ReadSkeleton: %d !\n", mode);
    break;
  }

  // close the file
  fclose(fskel);

  return true;
}


bool Skeleton::RemoveSegment(int seg_num) {
  return this->segments.Remove(seg_num);
}

bool Skeleton::RemoveAllSegments() {
	return this->segments.Reset();
}

BEGIN {
  maxX = 514;
  maxY = 514;
  maxZ = 349;

  offX = 168 - 26;
  offY = 35 - 26;
  offZ = 0 - 25;
}

{ 
  newX = $1 + offX;
  newY = $2 + offY;
  newZ = $3 + offZ;

  if(newX <= 0) print "ERROR"
  if(newY <= 0) print "ERROR"
  if(newZ <= 0) print "ERROR"

  if(newX >= maxX) print "ERROR"
  if(newY >= maxY) print "ERROR"
  if(newZ >= maxZ) print "ERROR"

  print newX " " newY " " newZ " " $4 " " $5
}

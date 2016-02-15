# cs177_projectpart1

Not sure if it is allowed for me to post all of the specifications from the assignment but here is an excerpt: 

"The goal of this part of the project is to build the first stage of a CSIM model of cars traveling along a section of roadway. In this case, your program is a caricature of people driving in circles around the Ontario airport until they can pick up their arriving passengers from the loading zone along the curb directly in front of the airport terminal building. 

.
.
.

For this assignment, the roadway is just a one-way "loop" that carries a single lane of traffic around a circle in front of the airport terminal. (This means you cannot pass the car in front of you, even if its speed is below your own target.)

The roadway will be modelled by a linear sequence of discrete "cells", each representing a short segment of road (11 feet long, say) that is half the length of a car. The entire roadway is a loop with exactly 120 cells, and represents a distance of exactly 1/4 mile. For now, we will assume that "zombie" cars simply drive around in a circle forever, without ever trying to enter or leave it.  "


The project uses a specialized library for running simulations:
CSIM-19 in April 2003

Documentation on CSIM:
http://www.mesquite.com/documentation/documents/CSIM20_User_Guide-C++.pdf
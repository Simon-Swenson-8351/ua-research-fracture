At some point, configuration should probably be broken out into its own .conf 
file, but, right now, just modify the python file to change fixed input nodes.
When you run the program, it will just take N random samples of a particle 
moving through air, perturbed by gravity. Each sample will include M images 
and a .json file with the values for all non-fixed nodes. You'll need numpy, 
scipy, and PIL to run it. The object's location as seen by the camera will be 
noisy (with Gaussian noise), and each pixel will also be noisy (with Gaussian 
noise).

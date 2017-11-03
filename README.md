# parametric
Experiment of creating GPU based parametric surfaces

Prerequisites:

   CMake 2.8 or later for makefile generation
   OpenSceneGraph-3.4 or later


To compile:

   cd parametric
   cmake .
   make


To run:

    Using a single Z_FNCTION for both top and bottom surfaces, using z==0.0 to distinguish between the two

        apps/parametric --rows 100 --columns 100 --shader shaders/parametric.vert --shader shaders/parametric.frag --cone 1 0.5 0 1.0 2.9 --all --Z_FUNCTION "(x, y, z) ((z==0.0?-1.0 : 1.0)*((x-x*x)*(y-y*y)*5.0))" -d -b

    Using Z_BASE and Z_TOP to provide separate functions for bottom and top surfaces

        apps/parametric --rows 100 --columns 100 --shader shaders/parametric.vert --shader shaders/parametric.frag --cone 1 0.5 0 1.0 2.9 --all --Z_BASE "(x, y, z) (-0.1*sin(x*y*6.28))"  --Z_TOP "(x,y,z) ((x-x*x)*(y-y*y)*5.0)" -b -d

    Using the osg_SimulationTime uniform provided by the OSG to animate the bottom and top surfaces functions

        apps/parametric --rows 100 --columns 100 --shader shaders/parametric.vert --shader shaders/parametric.frag --cylinder 0.5 0.5 0.5 0.4 2.2 --all --Z_BASE "(x, y, z) (-0.1*sin(x*y*6.28+osg_SimulationTime))"  --Z_TOP "(x,y,z) ((x-x*x)*(y-y*y)*5.0+0.2*sin(osg_SimulationTime))" -b -d
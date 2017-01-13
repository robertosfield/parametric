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

    To create a a million vertex, two million triangle mesh

        ./parametric --rows 1000 --columns 1000 --shader parametric.vert --shader parametric.frag

        ./parametric --rows 100 --columns 100 --shader parametric.vert --shader parametric.frag --cone 1 0 0 0.2 0.5 --box 0.4 1 0 0.2 0.3 0.5 --capsule 0 0.5 0 0.3 0.2 --cylinder 0.5 0.5 0.5 0.25 1

        ./parametric --rows 100 --columns 100 --shader parametric.vert --shader parametric.frag --cone 1 0.5 0 1.0 2.9 --all --Z_FUNCTION "(x, y, z) ((z==0.0?-1.0 : 1.0)*((x-x*x)*(y-y*y)*5.0))" -d
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

        ./parametric --rows 1000 --columns 1000 --shader parametric.vert --shader parametric.frag --cutOff 0.01

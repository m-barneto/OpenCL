#include <SFML/Graphics.hpp>
using namespace sf;


#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "OpenCLHelper.h"
#include <random>

constexpr float PI = 3.14159265359;

#include "ParticleSystem.h"

int main() {
    OpenCLHelper ocl;
    Kernel k;
    ocl.createKernel("particle_move.cl", "particle_move", k);
    ContextSettings settings{ 0 };
    settings.antialiasingLevel = 16;
    RenderWindow window(VideoMode(800, 600), "OpenCL Particles", Style::Default, settings);
    //window.setFramerateLimit(0);
    window.setVerticalSyncEnabled(true);
    

    ParticleSystem ps(10000, window.getSize());

    /*
    cl_mem buffer = ocl.createBuffer<float>(particles, CL_MEM_READ_WRITE);

    ocl.writeToBuffer(k, particles, buffer);

    ocl.writeArg<cl_mem>(k, 0, buffer);

    ocl.runKernel(k, particles.size(), 100);

    ocl.readArg<float>(0, buffer, &particles[0], particles.size());*/


    //std::vector<cl_int> val;
    //val.push_back(rand());

    //cl_mem buffer = ocl.createBuffer<float>(particles, CL_MEM_READ_WRITE);
    //cl_mem valBuffer = ocl.createBuffer<int>(val , CL_MEM_WRITE_ONLY);
    //cl_mem time_buffer = ocl.createBuffer<cl_int>({ (cl_int)1 }, CL_MEM_WRITE_ONLY);


    
    Event e;
    Clock clock;
    while (window.isOpen()) {
        while (window.pollEvent(e)) {
            switch (e.type) {
            case Event::Closed:
                window.close();
                break;
            }
        }
        window.clear();
        float dt = clock.restart().asSeconds();
        ps.update(dt);
        ps.render(window, dt);
        window.display();

        //ocl.writeToBuffer(k, particles, buffer);
        //val[0] = rand();
        //ocl.writeToBuffer(k, val, valBuffer);
        //std::vector<cl_int> t{ (cl_int)1 };
        //ocl.writeToBuffer(k, t, time_buffer);

        //ocl.writeArg<cl_mem>(k, 0, buffer);
        //ocl.writeArg<cl_mem>(k, 1, valBuffer);

        //ocl.runKernel(k, particles.size(), 1000);

        //ocl.readArg<float>(0, buffer, &particles[0], particles.size());

    }
}

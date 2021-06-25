#pragma once
#include <thread>

struct Particle {
    float x, y, hue;
    cl_bool solid;
};


class ParticleSystem {
private:
    int particleCount;
    std::vector<Particle> particles;
    Vector2u size;
    VertexArray vertexArray;
    std::vector<Vertex> offsets;
    int vertexCount;
    float circleRadius;

    OpenCLHelper ocl;

    Kernel collisionKernel;
    cl_mem collisionBuffer;

    std::default_random_engine eng;
    std::uniform_real_distribution<float> distx;
    std::uniform_real_distribution<float> disty;
    void updateParticles(int start, int end) {
        for (int i = start; i < end; ++i) {
            if (particles[i].solid) {
                continue;
            }
            particles[i].x += ((float)rand() / (float)RAND_MAX * 2.f - 1.f) * 10.f;
            particles[i].y += ((float)rand() / (float)RAND_MAX * 2.f - 1.f) * 10.f;

            if (particles[i].x < 0.f) {
                particles[i].x = size.x;
            }
            else if (particles[i].x > size.x) {
                particles[i].x = 0.f;
            }

            if (particles[i].y < 0.f) {
                particles[i].y = size.y;
            }
            else if (particles[i].y > size.y) {
                particles[i].y = 0.f;
            }
        }
    }
public:
    ParticleSystem(int particleCount, Vector2u size) {
        this->particleCount = particleCount;
        this->size = size;
        this->vertexArray = VertexArray(PrimitiveType::Triangles);

        ocl.createKernel("particle_check_col.cl", "particle_check_col", collisionKernel);
        

        #pragma region Cache Particle Shape

        this->circleRadius = 3.f;
        this->vertexCount = 5;
        
        float x = 0.f;
        float y = 0.f;

        for (int i = 0; i < vertexCount; ++i) {
            // Center point
            Vertex vert({ x, y }, Color::Cyan);
            offsets.push_back(vert);

            // Previous point
            float angle = (float)(i - 1) / vertexCount * 360.f * PI / 180.f;
            vert.position.x += cosf(angle) * circleRadius;
            vert.position.y += sinf(angle) * circleRadius;
            offsets.push_back(vert);

            // Current Point
            vert = Vertex({ x, y }, Color::Cyan);
            angle = (float)i / vertexCount * 360.f * PI / 180.f;
            vert.position.x += cosf(angle) * circleRadius;
            vert.position.y += sinf(angle) * circleRadius;
            offsets.push_back(vert);
        }
        #pragma endregion
        #pragma region Setup Particles
        std::random_device rd;
        eng = std::default_random_engine(rd());
        distx = std::uniform_real_distribution<float>(0.f, size.x);
        disty = std::uniform_real_distribution<float>(0.f, size.y);

        Particle p;
        for (int i = 0; i < particleCount - 1; ++i) {
            p.x = distx(eng);
            p.y = disty(eng);
            p.solid = false;

            particles.push_back(p);
        }

        p.x = size.x / 2.f;
        p.y = size.y / 2.f;
        p.solid = true;

        particles.push_back(p);
        #pragma endregion

        collisionBuffer = ocl.createBuffer<Particle>(particles, CL_MEM_READ_WRITE);
    }

    void update(const float& deltaTime) {

        //std::thread second(updateParticles, 0, 1000);


        int count = 0;
        for (int i = 0; i < particles.size(); ++i) {
            if (particles[i].solid) {
                count++;
                continue;
            }
            particles[i].x += ((float)rand() / (float)RAND_MAX * 2.f - 1.f) * 10.f;
            particles[i].y += ((float)rand() / (float)RAND_MAX * 2.f - 1.f) * 10.f;
            
            if (particles[i].x < 0.f) {
                particles[i].x = size.x;
            }
            else if (particles[i].x > size.x) {
                particles[i].x = 0.f;
            }

            if (particles[i].y < 0.f) {
                particles[i].y = size.y;
            }
            else if (particles[i].y > size.y) {
                particles[i].y = 0.f;
            }

            

            /*
            
            std::cout << ocl.writeToBuffer(collisionKernel, particles, collisionBuffer) << std::endl;
            std::cout << ocl.writeArg<cl_mem>(collisionKernel, 0, collisionBuffer) << std::endl;

            std::cout << ocl.runKernel(collisionKernel, particles.size(), particles.size()) << std::endl;

            std::cout << ocl.readArg<Particle>(0, collisionBuffer, &particles[0], particles.size()) << std::endl;*/
        }

        Particle p;
        p.x = distx(eng);
        p.y = disty(eng);
        p.solid = false;

        //particles.push_back(p);

        /*
        Particle p;
        while (particles.size() - count <= particleCount) {
            p.x = distx(eng);
            p.y = disty(eng);
            p.solid = false;

            particles.push_back(p);
        }
        //*/

        int erro = ocl.writeToBuffer(collisionKernel, particles, collisionBuffer);
        erro = ocl.writeArg<cl_mem>(collisionKernel, 0, collisionBuffer);

        erro = ocl.runKernel(collisionKernel, particles.size(), 1000);

        erro = ocl.readArg<Particle>(0, collisionBuffer, &particles[0], particles.size());

    }

    void render(RenderWindow& window, const float& dT) {
        int count = 0;
        for (int i = 0; i < particles.size(); ++i) {
            if (particles[i].solid) count++;
        }
        window.setTitle(std::to_string(count));

        vertexArray.resize(particles.size() / 2 * vertexCount * 3 * 3);
        for (int i = 0; i < particles.size(); ++i) {
            Color col;
            if (particles[i].solid) {
                col = Color::Cyan;
            }
            else {
                col = Color::Blue;
            }


            Vector2f v{ particles[i].x, particles[i].y };
            for (int j = 0; j < vertexCount * 3; j += 3) {
                vertexArray.append(Vertex{ offsets[j].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 1].position + v, col });
                vertexArray.append(Vertex{ offsets[j + 2].position + v, col });
            }
        }

        window.draw(vertexArray);
    }
};